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
#include <QLabel>
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

static AutoCompleter::Method parseMethod( const YAML::Node & node )
{
    Q_ASSERT(node.Type() == YAML::NodeType::Sequence);
    Q_ASSERT(node.size() >= 2);

    AutoCompleter::Method m;
    m.className = node[0].to<std::string>().c_str();
    m.methodName = node[1].to<std::string>().c_str();
    YAML::Iterator it = node.begin();
    ++it; ++it;
    while (it != node.end())
    {
        // get arg name
        assert(it->Type() == YAML::NodeType::Scalar);
        m.argNames << it->to<std::string>().c_str();
        // get arg default value
        ++it;
        if (it == node.end())
            break;
        if(it->Type() == YAML::NodeType::Scalar)
            m.argDefaults << it->to<std::string>().c_str();
        else
            m.argDefaults << QString();
        // next arg
        ++it;
    }

    return m;
}

static QString methodSignature( const AutoCompleter::Method & method )
{
    QString text = method.methodName;
    text += "(";
    int argc = method.argNames.count();
    for (int i = 0; i < argc; ++i) {
        text += method.argNames[i];
        QString val = method.argDefaults[i];
        if (!val.isEmpty())
            text += " = " + val;
        if (i != argc - 1)
            text += ", ";
    }
    text +=")";
    return text;
}

class CompletionMenu : public PopUpWidget
{
public:
    enum DataRole {
        CompletionRole = Qt::UserRole,
        MethodRole
    };

    CompletionMenu( QWidget * parent = 0 ):
        PopUpWidget(parent),
        mCompletionRole( Qt::DisplayRole )
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

    void addItem( QStandardItem * item )
    {
        mModel->appendRow( item );
    }

    void setCompletionRole( int role )
    {
        mFilterModel->setFilterRole(role);
        mFilterModel->setSortRole(role);
        mCompletionRole = role;
    }

    QString currentText()
    {
        QStandardItem *item =
            mModel->itemFromIndex (
                mFilterModel->mapToSource (
                    mListView->currentIndex()));
        if (item)
            return item->data(mCompletionRole).toString();

        return QString();
    }

    AutoCompleter::Method currentMethod()
    {
        QStandardItem *item =
            mModel->itemFromIndex (
                mFilterModel->mapToSource (
                    mListView->currentIndex()));
        if (item)
            return item->data(MethodRole).value<AutoCompleter::Method>();

        return AutoCompleter::Method();
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
    int mCompletionRole;
};

class MethodCallWidget : public QWidget
{
public:
    MethodCallWidget( QWidget * parent = 0 ):
        QWidget( parent, Qt::ToolTip )
    {
        QPalette p;
        p.setColor( QPalette::Window, p.color(QPalette::ToolTipBase) );
        p.setColor( QPalette::WindowText, p.color(QPalette::ToolTipText) );
        setPalette(p);

        mLabel = new QLabel();

        QHBoxLayout *box = new QHBoxLayout;
        box->setContentsMargins(5,2,5,2);
        box->addWidget(mLabel);
        setLayout(box);
    }

    void showMethod( const AutoCompleter::Method & method, int arg )
    {
        QString text = method.methodName;
        text += " (";
        int argc = method.argNames.count();
        for (int i = 0; i < argc; ++i) {
            if (i == arg)
                text += "<b>" + method.argNames[i] + "</b>";
            else
                text += method.argNames[i];
            QString val = method.argDefaults[i];
            if (!val.isEmpty())
                text += " = " + val;
            if (i != argc - 1)
                text += ", ";
        }
        text +=")";
        mLabel->setText(text);
    }

private:
    QLabel *mLabel;
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
        //startMethodCall();
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

    while (!mMethodCall.stack.isEmpty())
    {
        MethodCall & call = mMethodCall.stack.top();
        if (pos > call.position) {
            qDebug("MethodCall: change after call. aborting.");
            break;
        }
        else {
            qDebug("MethodCall: change before call. popping.");
            mMethodCall.stack.pop();
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

    if (mCompletion.on) {
        if (cursorPos < mCompletion.pos ||
            cursorPos > mCompletion.pos + mCompletion.len)
        {
            quitCompletion("out of bounds");
        }
    }

    updateMethodCall(cursorPos);
}

void AutoCompleter::onResponse( const QString & cmd, const QString & data )
{
    qDebug("response.");
    if (mCompletion.on && (
        cmd == "completeClass" ||
        cmd == "completeMethod" ||
        cmd == "completeClassMethod"))
    {
        showCompletionMenu( data );
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
        mCompletion.type = ClassCompletion;
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
            mCompletion.text = tokenText(mit);
        }
        else {
            mCompletion.pos = dit.position() + 1;
            mCompletion.len = 0;
            mCompletion.text.clear();
        }

        if (cit.isValid()) {
            mCompletion.contextPos = mCompletion.pos;
            contextText = tokenText(cit);
            command = "completeClassMethod";
            mCompletion.type = ClassMethodCompletion;
        }
        else {
            mCompletion.contextPos = mCompletion.pos + 3;
            contextText = tokenText(mit);
            contextText.truncate(3);
            command = "completeMethod";
            mCompletion.type = MethodCompletion;
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

void AutoCompleter::showCompletionMenu( const QString & data )
{
    if (!mCompletion.menu.isNull()) {
        qWarning("Recursive request to show completion menu!");
        return;
    }

    QPointer<CompletionMenu> popup = new CompletionMenu(mEditor);

    std::stringstream stream;
    stream << data.toStdString();
    YAML::Parser parser(stream);

    YAML::Node doc;
    if(!parser.GetNextDocument(doc) || doc.Type() != YAML::NodeType::Sequence) {
        qWarning("Bad YAML data!");
        delete popup;
        return;
    }

    switch (mCompletion.type)
    {
    case ClassCompletion:
    {
        for (YAML::Iterator it = doc.begin(); it != doc.end(); ++it)
        {
            YAML::Node const & entry = *it;
            //qDebug() << (int) entry.Type();
            if(entry.Type() == YAML::NodeType::Scalar) {
                QString text( entry.to<std::string>().c_str() );
                popup->addItem( new QStandardItem(text) );
            }
            else
                qWarning("Class Name Completion: a YAML data entry not a scalar");
        }
        break;
    }
    case ClassMethodCompletion:
    case MethodCompletion:
    {
        for (YAML::Iterator it = doc.begin(); it != doc.end(); ++it)
        {
            YAML::Node const & entry = *it;
            if (entry.Type() != YAML::NodeType::Sequence) {
                qWarning("Method Name Completion: a YAML data entry not a Sequence");
                continue;
            }
            if (entry.size() < 2) {
                qWarning("YAML parsing: two few sequence elements");
                continue;
            }
            Method m = parseMethod(entry);
            QStandardItem *item = new QStandardItem();
            if (mCompletion.type == ClassMethodCompletion)
                item->setText(m.methodName);
            else {
                item->setText(m.methodName + " (" + m.className + ')');
                item->setData(m.methodName, CompletionMenu::CompletionRole);
            }
            item->setData( QVariant::fromValue<Method>(m), CompletionMenu::MethodRole );
            popup->addItem(item);
        }
        break;
    }
    default:
        Q_ASSERT(false);
    }

    if (mCompletion.type == MethodCompletion)
        popup->setCompletionRole(CompletionMenu::CompletionRole);

    mCompletion.menu = popup;

    popup->view()->installEventFilter(this);
    connect(popup, SIGNAL(finished(int)), this, SLOT(onCompletionMenuFinished(int)));

    QTextCursor cursor(document());
    cursor.setPosition(mCompletion.pos);
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

void AutoCompleter::onCompletionMenuFinished( int result )
{
    qDebug("completion menu finished");

    if (!mCompletion.on)
        return;

    if (result) {
        QString text = mCompletion.menu->currentText();

        if (!text.isEmpty()) {
            CompletionType type = mCompletion.type;
            Method method;
            if (type == MethodCompletion || type == ClassMethodCompletion)
                method = mCompletion.menu->currentMethod();

            quitCompletion("done");

            QTextCursor cursor( mEditor->textCursor() );
            cursor.setPosition( mCompletion.pos );
            cursor.setPosition( mCompletion.pos + mCompletion.len, QTextCursor::KeepAnchor );
            cursor.insertText(text);

            if (!method.methodName.isEmpty()) {
                cursor.insertText("(");
                pushMethodCall(cursor.position() - 1, method);
            }

            return;
        }
    }

    // Do not cancel completion whenever menu hidden.
    // It could be hidden because of current filter yielding 0 results.

    //quitCompletion("cancelled");
}

void AutoCompleter::startMethodCall()
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

    //pushMethodCall( bracketPos, className + "." + methodName );
}

void AutoCompleter::updateMethodCall( int cursorPos )
{
    QTextDocument *doc = document();

    while (!mMethodCall.stack.isEmpty())
    {
        MethodCall & call = mMethodCall.stack.top();
        if (call.position >= cursorPos) {
            qDebug("call right of cursor. popping.");
            mMethodCall.stack.pop();
            continue;
        }

        QTextBlock block( document()->findBlock( call.position ) );
        TokenIterator token = TokenIterator::rightOf(block, call.position - block.position());
        if (!token.isValid()) {
            qDebug("call stack out of sync!");
            mMethodCall.stack.clear();
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
            qDebug("current call: %s(%i)", call.method.methodName.toStdString().c_str(), arg);
            showMethodCall(call, arg);
            return;
        }
        else {
            qDebug("call left of cursor. popping.");
            mMethodCall.stack.pop();
        }
    }

    hideMethodCall();
    qDebug("call stack empty");
}
#if 0
void AutoCompleter::updateMethodCall( int cursorPos )
{
    const QString rightBrackets(")]}");
    const QString leftBrackets("([{");

    if (mMethodCall.stack.isEmpty()) {
        qDebug("stack empty");
        return;
    }

    MethodCall & call = mMethodCall.stack.top()

    QTextDocument *doc = document();
    QTextBlock block( doc->findBlock(call.position) );
    BracketIterator lbracket = BracketIterator::rightOf(block, call.position - block.position());

    if (!lbracket.isValid()) {
        qDebug("stack out of sync!");
        mMethodCall.stack.clear();
        return;
    }

    BracketIterator rbracket = lbracket;
    ++rbracket;

    while (!mMethodCall.stack.isEmpty())
    {
        call = mMethodCall.stack.top();

        if (call.position > lbracket.position()) {
            mMethodCall.stack.pop();
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
            mMethodCall.stack.clear();
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
            mMethodCall.stack.pop();
        }
    }
}
#endif

void AutoCompleter::pushMethodCall( int pos, const Method & method, int arg )
{
    Q_ASSERT( mMethodCall.stack.isEmpty() || mMethodCall.stack.last().position <= pos );

    if ( !mMethodCall.stack.isEmpty() && mMethodCall.stack.last().position == pos )
        return;

    MethodCall call;
    call.position = pos;
    call.method = method;

    mMethodCall.stack.push(call);

    showMethodCall(call, arg);
}

void AutoCompleter::showMethodCall( const MethodCall & call, int arg )
{
    QTextCursor cursor(document());
    cursor.setPosition(call.position);
    QPoint pos =
        mEditor->viewport()->mapToGlobal( mEditor->cursorRect(cursor).topLeft() );
    pos += QPoint(0, -20);

    if (mMethodCall.widget.isNull())
        mMethodCall.widget = new MethodCallWidget(mEditor->viewport());

    MethodCallWidget *w = mMethodCall.widget;

    w->showMethod( call.method, arg );
    w->resize(w->sizeHint());
    qDebug() << "pop up method call:" << w->sizeHint();
    w->move(pos);
    w->show();
}

void AutoCompleter::hideMethodCall()
{
    delete mMethodCall.widget;
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
