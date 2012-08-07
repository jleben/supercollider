/*
    SuperCollider Qt IDE
    Copyright (c) 2012 Jakob Leben & Tim Blechmann
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "autocompleter.hpp"
#include "tokens.hpp"
#include "editor.hpp"
#include "../util/popup_widget.hpp"
#include "../../core/sc_process.hpp"
#include "../../core/main.hpp"

#include "yaml-cpp/node.h"
#include "yaml-cpp/parser.h"

#include <QDebug>
#include <QToolTip>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHBoxLayout>
#include <QApplication>

namespace ScIDE {

static bool tokenMaybeName( Token::Type type )
{
    return (type == Token::Name || type == Token::Keyword || type == Token::Builtin);
}

class CompletionMenu : public PopUpWidget
{
public:
    CompletionMenu( QWidget * parent = 0 ):
        PopUpWidget(parent)
    {
        mModel = new QStandardItemModel(this);
        mFilterModel = new QSortFilterProxyModel(this);
        mFilterModel->setSourceModel(mModel);

        mListView = new QListView();
        mListView->setModel(mFilterModel);
        mListView->setFrameShape(QFrame::NoFrame);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->addWidget(mListView);
        layout->setContentsMargins(1,1,1,1);

        connect(mListView, SIGNAL(clicked(QModelIndex)), this, SLOT(accept()));
        connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(accept()));

        mListView->setFocus(Qt::OtherFocusReason);

        resize(200, 200);
    }

    void addItem( const QString & text )
    {
        mModel->appendRow( new QStandardItem(text) );
    }

    QString currentText()
    {
        QStandardItem *item =
            mModel->itemFromIndex (
                mFilterModel->mapToSource (
                    mListView->currentIndex()));
        if (item)
            return item->text();

        return QString();
    }

    QString exec( const QPoint & pos )
    {
        QString result;
        QPointer<CompletionMenu> self = this;
        if (PopUpWidget::exec(pos)) {
            if (!self.isNull())
                result = currentText();
        }
        return result;
    }

    QSortFilterProxyModel *model() { return mFilterModel; }

    QListView *view() { return mListView; }

private:
    QListView *mListView;
    QStandardItemModel *mModel;
    QSortFilterProxyModel *mFilterModel;
};

AutoCompleter::AutoCompleter( CodeEditor *editor ):
    QObject(editor),
    mEditor(editor),
    mScRequest( new ScRequest(Main::instance()->scProcess(), this) )
{
    mCompletion.on = false;

    connect(editor, SIGNAL(cursorPositionChanged()),
            this, SLOT(onCursorChanged()));
    connect(mScRequest, SIGNAL(response(QString,QString)),
            this, SLOT(onResponse(QString,QString)));
}

bool AutoCompleter::eventFilter( QObject *, QEvent *ev )
{
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
        switch(kev->key())
        {
        case Qt::Key_Backspace:
            mEditor->textCursor().deletePreviousChar();
            return true;
        case Qt::Key_Delete:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
            return false;
        }

        QString text = kev->text();
        if (!text.isEmpty()) {
            mEditor->textCursor().insertText(text);
            keyPress(kev);
            return true;
        }
    }
    return false;
}

void AutoCompleter::documentChanged( QTextDocument * doc )
{
    connect(doc, SIGNAL(contentsChange(int,int,int)),
            this, SLOT(onContentsChange(int,int,int)));
}

inline QTextDocument *AutoCompleter::document()
{
    return static_cast<QPlainTextEdit*>(mEditor)->document();
}

void AutoCompleter::keyPress( QKeyEvent *e )
{
    int key = e->key();
    switch (e->key())
    {
    case Qt::Key_ParenLeft:
    case Qt::Key_Comma:
        aidMethodCall();
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return;
    default:
        qDebug("key");
        if (!e->text().isEmpty() && !mCompletion.on)
            startCompletion();
    }
#if 0
    if ((key >= Qt::Key_0 && key <= Qt::Key_9) ||
        (key >= Qt::Key_A && key <= Qt::Key_Z) ||
        key == Qt::Key_Period)
#endif
}

void AutoCompleter::onContentsChange( int pos, int removed, int added )
{
    qDebug("contentsChange");
    while (!mMethodCallStack.isEmpty())
    {
        MethodCall & call = mMethodCallStack.top();
        if (pos > call.position) {
            qDebug("change after call. aborting.");
            return;
        }
        else {
            qDebug("change before call. popping.");
            mMethodCallStack.pop();
        }
    }

    if (mCompletion.on)
    {
        if(pos < mCompletion.contextPos)
        {
            quitCompletion("context changed");
        }
        else if(pos <= mCompletion.pos + mCompletion.len)
        {
            QTextBlock block( document()->findBlock(mCompletion.pos) );
            TokenIterator it( block, mCompletion.pos - block.position() );
            Token::Type type = it.type();
            if (type == Token::Class || tokenMaybeName(type)) {
                mCompletion.len = it->length;
                mCompletion.text = tokenText(it);
            }
            else {
                mCompletion.len = 0;
                mCompletion.text.clear();
            }
            if (!mCompletion.menu.isNull())
                updateCompletionMenu();
        }
    }
}

void AutoCompleter::onCursorChanged()
{
    qDebug("cursorChanged");
    int cursorPos = mEditor->textCursor().position();
    //checkStack(cursorPos);
    if (mCompletion.on) {
        if (cursorPos < mCompletion.pos ||
            cursorPos > mCompletion.pos + mCompletion.len)
        {
            quitCompletion("out of bounds");
        }
    }
}

void AutoCompleter::startCompletion()
{
    qDebug("complete...");
    if (mCompletion.on) {
        qWarning("AutoCompleter::startCompletion(): completion already started!");
        return;
    }

    QTextCursor cursor( mEditor->textCursor() );
    int cursorPos = cursor.positionInBlock();
    QTextBlock block( cursor.block() );
    TokenIterator it( block, cursorPos - 1 );

#if 0
    TokenIterator t(block);
    QStringList out;
    while( t.isValid() ) {
        out << QString("(%1 %2)").arg(t->position).arg(t->character != 0 ? t->character : '/');
        ++t;
    }
    qDebug() << "tokens:" << out;
#endif

    if (!it.isValid())
        return;

    const Token & token = *it;
    QString contextText, command;

    if (token.type == Token::Class)
    {
        if (token.length < 3)
            return;
        mCompletion.pos = it.position();
        mCompletion.len = it->length;
        mCompletion.text = tokenText(it);
        mCompletion.contextPos = mCompletion.pos + 3;
        contextText = mCompletion.text;
        contextText.truncate(3);
        command = "completeClass";
    }
    else {
        // Parse method call

        TokenIterator cit, dit, mit;

        if (token.character == '.')
        {
            dit = it;
            --it;
            if (it.type() == Token::Class)
                cit = it;
            else
                return;
            it = dit.next();
            if (tokenMaybeName(it.type()))
                mit = it;
        }
        else if (tokenMaybeName(token.type))
        {
            mit = it;
            --it;
            if (it.isValid() && it->character == '.')
                dit = it;
            else
                return;
            --it;
            if (it.type() == Token::Class)
                cit = it;
        }
        else
            return;

        if (!cit.isValid() && mit->length < 3)
            return;

        if (mit.isValid()) {
            mCompletion.pos = mit.position();
            mCompletion.len = mit->length;
        }
        else {
            mCompletion.pos = dit.position() + 1;
            mCompletion.len = 0;
        }

        mCompletion.text = tokenText(mit);

        if (cit.isValid()) {
            mCompletion.contextPos = mCompletion.pos;
            contextText = tokenText(cit);
            command = "completeClassMethod";
        }
        else {
            mCompletion.contextPos = mCompletion.pos + 3;
            contextText = tokenText(mit);
            contextText.truncate(3);
            command = "completeMethod";
        }
    }

    mCompletion.on = true;
    qDebug() << "completion ON";
    qDebug() << "sending request:" << command << contextText;
    mScRequest->send( command, contextText );
}

void AutoCompleter::quitCompletion( const QString & reason )
{
    Q_ASSERT(mCompletion.on);

    qDebug() << QString("Completion OFF (%1)").arg(reason);

    mScRequest->cancel();

    if (mCompletion.menu) {
        mCompletion.menu->hide();
        mCompletion.menu->deleteLater();
        mCompletion.menu = 0;
    }

    mCompletion.on = false;
}

void AutoCompleter::aidMethodCall()
{
    // go find the bracket that I'm currently in,
    // and count relevant commas along the way

    QTextDocument *doc = document();
    QTextCursor cursor( mEditor->textCursor() );

    int pos = cursor.position();

    QTextBlock block( doc->findBlock(pos) );
    if (!block.isValid())
        return;
    pos -= block.position();

    TokenIterator it( TokenIterator::leftOf( block, pos ) );

    int level = 1;
    int argPos = 0;

    while (it.isValid())
    {
        char chr = it->character;
        Token::Type type = it->type;
        if (chr == ',') {
            if (level == 1)
                ++argPos;
        }
        else if (type == Token::ClosingBracket)
            ++level;
        else if (type == Token::OpeningBracket)
        {
            --level;
            if (level == 0) {
                if (chr == '(')
                    break;
                else {
                    qDebug() << "wrong bracket type:" << chr;
                    return;
                }
            }
        }
        --it;
    }

    if (!it.isValid()) {
        qDebug() << "no current bracket.";
        return;
    }

    int bracketPos;
    pos = bracketPos = it.position();

    QString className, methodName;

    --it;
    if (it.isValid())
    {
        int type = it->type;
        if (type == Token::Class) {
            className = tokenText(it);
            methodName = "new";
        }
        else if (type == Token::Name) {
            methodName = tokenText(it);
            --it;
            if (it.isValid() && it->character == '.') {
                --it;
                if (it.isValid() && it->type == Token::Class)
                    className = tokenText(it);
            }
        }
        else
            qDebug("no Class or Name token before bracket.");
    }
    else
        qDebug("no token before bracket.");


    if (methodName.isEmpty())
        return;

    qDebug("found call: %s.%s(%i)",
           className.toStdString().c_str(),
           methodName.toStdString().c_str(),
           argPos);

    pushMethodCall( bracketPos, className + "." + methodName );
}

void AutoCompleter::checkStack( int cursorPos )
{
    QTextDocument *doc = document();

    while (!mMethodCallStack.isEmpty())
    {
        MethodCall & call = mMethodCallStack.top();
        if (call.position >= cursorPos) {
            qDebug("call right of cursor. popping.");
            mMethodCallStack.pop();
            continue;
        }

        QTextBlock block( document()->findBlock( call.position ) );
        TokenIterator token = TokenIterator::rightOf(block, call.position - block.position());
        if (!token.isValid()) {
            qDebug("call stack out of sync!");
            mMethodCallStack.clear();
            break;
        }

        ++token;
        int arg = 0;
        int level = 1;
        while( level > 0 && token.isValid() && token.position() < cursorPos )
        {
            char chr = token.character();
            Token::Type type = token->type;
            if (chr == ',' && level == 1)
                ++arg;
            else if (type == Token::OpeningBracket)
                ++level;
            else if (type == Token::ClosingBracket)
                --level;

            ++token;
        }

        if (level > 0) {
            qDebug("current call: %s(%i)", call.name.toStdString().c_str(), arg);
            showMethodCall(call, arg);
            return;
        }
        else {
            qDebug("call left of cursor. popping.");
            mMethodCallStack.pop();
        }
    }

    hideMethodCall();
    qDebug("call stack empty");
}
#if 0
void AutoCompleter::checkStack( int cursorPos )
{
    const QString rightBrackets(")]}");
    const QString leftBrackets("([{");

    if (mMethodCallStack.isEmpty()) {
        qDebug("stack empty");
        return;
    }

    MethodCall & call = mMethodCallStack.top()

    QTextDocument *doc = document();
    QTextBlock block( doc->findBlock(call.position) );
    BracketIterator lbracket = BracketIterator::rightOf(block, call.position - block.position());

    if (!lbracket.isValid()) {
        qDebug("stack out of sync!");
        mMethodCallStack.clear();
        return;
    }

    BracketIterator rbracket = lbracket;
    ++rbracket;

    while (!mMethodCallStack.isEmpty())
    {
        call = mMethodCallStack.top();

        if (call.position > lbracket.position()) {
            mMethodCallStack.pop();
            continue;
        }

        int arg = 0;
        int level = 0;

        // search left

        while (lbracket.isValid() && lbracket.position() > call.position)
        {
            char ch = lbracket.character();
            if (chr == ',' && level >= 0)
                ++arg;
            else if (leftBrackets.contains(chr)) {
                ++level;
                if (level > 0)
                    arg = 0;
            }
            else if (rightBrackets.contains(chr))
                --level;

            --lbracket;
        }

        if (!lbracket.isValid() || lbracket.position() != call.position || level < 0) {
            qDebug("stack out of sync!");
            mMethodCallStack.clear();
            return;
        }

        // search right

        while( level >= 0 && rbracket.isValid() && rbracket.position() < cursorPos )
        {
            char ch = bracket.character();
            if (chr == ',' && level = 0)
                ++arg;
            else if (leftBrackets.contains(chr))
                ++level;
            else if (rightBrackets.contains(chr))
                --level;

            ++rbracket;
        }

        if ( level >= 0 ) {
            qDebug() << "current method:" << call.name << "|" << arg;
        }
        else {
            mMethodCallStack.pop();
        }
    }
}
#endif

void AutoCompleter::onResponse( const QString & cmd, const QString & data )
{
    qDebug("response.");
    if (mCompletion.on && (
        cmd == "completeClass" ||
        cmd == "completeMethod" ||
        cmd == "completeClassMethod"))
    {
        showCompletionMenu( mCompletion.pos, data );
    }
}

void AutoCompleter::onCompletionMenuFinished( int result )
{
    qDebug("completion menu finished");

    if (!mCompletion.on)
        return;

    if (result) {
        QString text = mCompletion.menu->currentText();

        if (!text.isEmpty()) {
            quitCompletion("done");

            QTextCursor cursor( mEditor->textCursor() );
            cursor.setPosition( mCompletion.pos );
            cursor.setPosition( mCompletion.pos + mCompletion.len, QTextCursor::KeepAnchor );
            cursor.insertText(text);

            return;
        }
    }

    // Do not cancel completion whenever menu hidden.
    // It could be hidden because of current filter yielding 0 results.

    //quitCompletion("cancelled");
}

void AutoCompleter::showCompletionMenu( int cursorPos, const QString & data )
{
    if (!mCompletion.menu.isNull()) {
        qWarning("Recursive request to show completion menu!");
        return;
    }

    std::stringstream stream;
    stream << data.toStdString();
    YAML::Parser parser(stream);

    YAML::Node doc;
    if(!parser.GetNextDocument(doc) || doc.Type() != YAML::NodeType::Sequence) {
        qWarning("Bad YAML data!");
        return;
    }

    QPointer<CompletionMenu> popup = new CompletionMenu(mEditor);
    mCompletion.menu = popup;

    for (YAML::Iterator it = doc.begin(); it != doc.end(); ++it) {
        YAML::Node const & entry = *it;
        //qDebug() << (int) entry.Type();
        if(entry.Type() == YAML::NodeType::Scalar)
            popup->addItem( entry.to<std::string>().c_str() );
    }

    qDebug() << "item count = " << doc.size();

    popup->view()->installEventFilter(this);
    connect(popup, SIGNAL(finished(int)), this, SLOT(onCompletionMenuFinished(int)));

    QTextCursor cursor(document());
    cursor.setPosition(cursorPos);
    QPoint pos =
        mEditor->viewport()->mapToGlobal( mEditor->cursorRect(cursor).bottomLeft() )
        + QPoint(0,5);

    popup->popup(pos);

    updateCompletionMenu();
}

void AutoCompleter::updateCompletionMenu()
{
    Q_ASSERT(mCompletion.on && !mCompletion.menu.isNull());

    CompletionMenu *menu = mCompletion.menu;

    if (!mCompletion.text.isEmpty()) {
        QString pattern = mCompletion.text;
        pattern.prepend("^");
        menu->model()->setFilterRegExp(pattern);
    }
    else {
        menu->model()->setFilterRegExp(QString());
    }

    if (menu->model()->hasChildren()) {
        menu->model()->sort(0);
        if (!menu->view()->selectionModel()->hasSelection())
            menu->view()->setCurrentIndex( menu->model()->index(0,0) );
        menu->show();
    }
    else {
        menu->hide();
    }
}

void AutoCompleter::pushMethodCall( int pos, const QString & name, int arg )
{
    Q_ASSERT( mMethodCallStack.isEmpty() || mMethodCallStack.last().position <= pos );

    if ( !mMethodCallStack.isEmpty() && mMethodCallStack.last().position == pos )
        return;

    MethodCall call;
    call.position = pos;
    call.name = name;

    mMethodCallStack.push(call);

    showMethodCall(call, arg);
}

void AutoCompleter::showMethodCall( const MethodCall & call, int arg )
{
    QTextCursor cursor(document());
    cursor.setPosition(call.position);
    QPoint pos = mEditor->viewport()->mapToGlobal( mEditor->cursorRect(cursor).topLeft() );
    pos += QPoint(0, -40);

    QString text = "<p style='white-space:pre'>";
    text += call.name;
    text += "(";
    for (int i = 0; i < arg; ++i)
        text += "arg, ";
    text += "<b>arg</b>)</p>";

    qDebug() << "showing tooltip:" << text << pos;

    QToolTip::showText( pos, text, mEditor );
}

void AutoCompleter::hideMethodCall()
{
    QToolTip::hideText();
}

QString AutoCompleter::tokenText( TokenIterator & it )
{
    if (!it.isValid()) {
        qDebug("token iterator invalid!");
        return QString();
    }

    int pos = it.position();
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(pos + it->length, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

} // namespace ScIDE
