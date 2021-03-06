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

#include "main_window.hpp"
#include "../core/main.hpp"
#include "../core/doc_manager.hpp"
#include "code_editor/editor.hpp"
#include "multi_editor.hpp"
#include "cmd_line.hpp"
#include "doc_list.hpp"
#include "post_window.hpp"
#include "settings/dialog.hpp"

#include <QAction>
#include <QShortcut>
#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>

namespace ScIDE {

MainWindow::MainWindow(Main * main) :
    mMain(main)
{
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );

    // Construct status bar:

    mLangStatus = new StatusLabel();
    mLangStatus->setText("Inactive");
    mSynthStatus = new StatusLabel();
    mSynthStatus->setText("Inactive");

    QStatusBar *status = statusBar();
    status->addPermanentWidget( new QLabel("Interpreter:") );
    status->addPermanentWidget( mLangStatus );
    status->addPermanentWidget( new QLabel("Synth:") );
    status->addPermanentWidget( mSynthStatus );

    // Code editor
    mEditors = new MultiEditor(main);

    // Docks
    mDocListDock = new DocumentsDock(main->documentManager(), this);
    mPostDock = new PostDock(this);

    // Layout

    // use a layout for tool widgets, to provide for separate margin control
    QVBoxLayout *tool_box = new QVBoxLayout;
    tool_box->addWidget(cmdLine());
    tool_box->setContentsMargins(5,2,5,2);

    QVBoxLayout *center_box = new QVBoxLayout;
    center_box->setContentsMargins(0,0,0,0);
    center_box->setSpacing(0);
    center_box->addWidget(mEditors);
    center_box->addLayout(tool_box);
    QWidget *central = new QWidget;
    central->setLayout(center_box);
    setCentralWidget(central);

    addDockWidget(Qt::LeftDockWidgetArea, mDocListDock);
    addDockWidget(Qt::BottomDockWidgetArea, mPostDock);

    // A system for easy evaluation of pre-defined code:
    connect(&mCodeEvalMapper, SIGNAL(mapped(QString)),
            this, SIGNAL(evaluateCode(QString)));
    connect(this, SIGNAL(evaluateCode(QString,bool)),
            main->scProcess(), SLOT(evaluateCode(QString,bool)));
    // Interpreter: post output
    connect(main->scProcess(), SIGNAL( scPost(QString) ),
            mPostDock->mPostWindow, SLOT( post(QString) ) );
    // Interpreter: monitor running state
    connect(main->scProcess(), SIGNAL( stateChanged(QProcess::ProcessState) ),
            this, SLOT( onInterpreterStateChanged(QProcess::ProcessState) ) );
    // Interpreter: forward status messages
    connect(main->scProcess(), SIGNAL(statusMessage(const QString&)),
            status, SLOT(showMessage(const QString&)));
    // Document list interaction
    connect(mDocListDock->list(), SIGNAL(clicked(Document*)),
            mEditors, SLOT(setCurrent(Document*)));
    connect(mEditors, SIGNAL(currentChanged(Document*)),
            mDocListDock->list(), SLOT(setCurrent(Document*)),
            Qt::QueuedConnection);

    createMenus();

    QIcon icon;
    icon.addFile(":/icons/sc-cube-128");
    icon.addFile(":/icons/sc-cube-48");
    icon.addFile(":/icons/sc-cube-32");
    icon.addFile(":/icons/sc-cube-16");
    QApplication::setWindowIcon(icon);
}

void MainWindow::createMenus()
{
    Settings::Manager *s = mMain->settings();
    s->beginGroup("IDE/shortcuts");

    new QShortcut( s->shortcut("cmdLineFocus"), this, SLOT(toggleComandLineFocus()) );

    QAction *act;

    // File

    mActions[Quit] = act = new QAction(
        QIcon::fromTheme("application-exit"), tr("&Quit..."), this);
    act->setShortcut(s->shortcut("quit"));
    act->setStatusTip(tr("Quit SuperCollider IDE"));
    QObject::connect( act, SIGNAL(triggered()), this, SLOT(onQuit()) );

    // View
    mActions[ShowDocList] = act = new QAction(tr("&Documents"), this);
    act->setStatusTip(tr("Show/Hide the Documents dock"));
    act->setCheckable(true);
    connect(act, SIGNAL(triggered(bool)), mDocListDock, SLOT(setVisible(bool)));
    connect(mDocListDock, SIGNAL(visibilityChanged(bool)), act, SLOT(setChecked(bool)));

    // Language

    mActions[EvaluateCurrentFile] = act = new QAction(
        QIcon::fromTheme("media-playback-start"), tr("Evaluate &File"), this);
    act->setStatusTip(tr("Evaluate current File"));
    connect(act, SIGNAL(triggered()), this, SLOT(evaluateCurrentFile()));

    mActions[EvaluateSelection] = act = new QAction(
        QIcon::fromTheme("media-playback-start"), tr("&Evaluate Selection"), this);
    act->setShortcut(s->shortcut("evaluateSelection"));
    act->setStatusTip(tr("Evaluate selection or current line"));
    connect(act, SIGNAL(triggered()), this, SLOT(evaluateSelection()));

    mActions[EvaluateRegion] = act = new QAction(
    QIcon::fromTheme("media-playback-start"), tr("&Evaluate Region"), this);
    act->setShortcut(s->shortcut("evaluateRegion"));
    act->setStatusTip(tr("Evaluate current region"));
    connect(act, SIGNAL(triggered()), this, SLOT(evaluateRegion()));

    mMain->scProcess()->action(ScIDE::SCProcess::StopMain)
        ->setShortcut(s->shortcut("stopMain"));

    // Settings

    mActions[ShowSettings] = act = new QAction(tr("&Configure IDE..."), this);
    act->setStatusTip(tr("Show configuration dialog"));
    connect(act, SIGNAL(triggered()), this, SLOT(showSettings()));

    // Help

    mActions[BrowseHelp] = act = new QAction(
    QIcon::fromTheme("system-help"), tr("&Browse Help"), this);
    act->setStatusTip(tr("Open help browser on the Browse page."));
    //connect(act, SIGNAL(triggered()), this, SLOT(browseHelp()));
    mCodeEvalMapper.setMapping(act, "HelpBrowser.openBrowsePage");
    connect(act, SIGNAL(triggered()), &mCodeEvalMapper, SLOT(map()));

    mActions[SearchHelp] = act = new QAction(
    QIcon::fromTheme("system-help"), tr("&Search Help"), this);
    act->setStatusTip(tr("Open help browser on the Search page."));
    //connect(act, SIGNAL(triggered()), this, SLOT(searchHelp()));
    mCodeEvalMapper.setMapping(act, "HelpBrowser.openSearchPage");
    connect(act, SIGNAL(triggered()), &mCodeEvalMapper, SLOT(map()));

    mActions[HelpForSelection] = act = new QAction(
    QIcon::fromTheme("system-help"), tr("&Help for Selection"), this);
    act->setShortcut(s->shortcut("helpForSelection"));
    act->setStatusTip(tr("Find help for selected text"));
    connect(act, SIGNAL(triggered()), this, SLOT(helpForSelection()));

    s->endGroup(); // IDE/shortcuts;

    QMenu *menu;
    QMenu *submenu;

    menu = new QMenu(tr("&File"), this);
    menu->addAction( mEditors->action(MultiEditor::DocNew) );
    menu->addAction( mEditors->action(MultiEditor::DocOpen) );
    menu->addAction( mEditors->action(MultiEditor::DocSave) );
    menu->addAction( mEditors->action(MultiEditor::DocSaveAs) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::DocClose) );
    menu->addSeparator();
    menu->addAction( mActions[Quit] );

    menuBar()->addMenu(menu);

    menu = new QMenu(tr("&Edit"), this);
    menu->addAction( mEditors->action(MultiEditor::Undo) );
    menu->addAction( mEditors->action(MultiEditor::Redo) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::Cut) );
    menu->addAction( mEditors->action(MultiEditor::Copy) );
    menu->addAction( mEditors->action(MultiEditor::Paste) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::Find) );
    menu->addAction( mEditors->action(MultiEditor::Replace) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::IndentMore) );
    menu->addAction( mEditors->action(MultiEditor::IndentLess) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::OpenClassDefinition) );

    menuBar()->addMenu(menu);

    menu = new QMenu(tr("&View"), this);
    submenu = new QMenu(tr("&Docks"), this);
    submenu->addAction( mActions[ShowDocList] );
    menu->addMenu(submenu);
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::EnlargeFont) );
    menu->addAction( mEditors->action(MultiEditor::ShrinkFont) );
    menu->addSeparator();
    menu->addAction( mEditors->action(MultiEditor::ShowWhitespace) );

    menuBar()->addMenu(menu);

    menu = new QMenu(tr("&Language"), this);
    menu->addAction( mMain->scProcess()->action(SCProcess::StartSCLang) );
    menu->addAction( mMain->scProcess()->action(SCProcess::RecompileClassLibrary) );
    menu->addAction( mMain->scProcess()->action(SCProcess::StopSCLang) );
    menu->addSeparator();
    menu->addAction( mActions[EvaluateCurrentFile] );
    menu->addAction( mActions[EvaluateRegion] );
    menu->addAction( mActions[EvaluateSelection] );
    menu->addSeparator();
    menu->addAction( mMain->scProcess()->action(ScIDE::SCProcess::RunMain) );
    menu->addAction( mMain->scProcess()->action(ScIDE::SCProcess::StopMain) );

    menuBar()->addMenu(menu);

    menu = new QMenu(tr("&Settings"), this);
    menu->addAction( mActions[ShowSettings] );

    menuBar()->addMenu(menu);

    menu = new QMenu(tr("&Help"), this);
    menu->addAction( mActions[BrowseHelp] );
    menu->addAction( mActions[SearchHelp] );
    menu->addAction( mActions[HelpForSelection] );

    menuBar()->addMenu(menu);
}

QAction *MainWindow::action( ActionRole role )
{
    Q_ASSERT( role < ActionCount );
    return mActions[role];
}

bool MainWindow::quit()
{
    bool ok = true;

    QList<Document*> docs = mMain->documentManager()->documents();
    Q_FOREACH(Document *doc, docs) {
        if(doc->textDocument()->isModified()) {
            ok = false;
            break;
        }
    }

    if(!ok) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(
            NULL,
            tr("SuperCollider IDE"),
            tr("There are unsaved changes to documents.\n"
                "Do you want to quit without saving?"),
            QMessageBox::Ok | QMessageBox::Cancel
        );
        if( ret == QMessageBox::Cancel )
            return false;
    }

    mMain->storeSettings();

    QApplication::quit();

    return true;
}

void MainWindow::onQuit()
{
    quit();
}

void MainWindow::onInterpreterStateChanged( QProcess::ProcessState state )
{
    QString text;
    QColor color;

    switch(state) {
    case QProcess::NotRunning:
        text = "Inactive";
        color = Qt::white;
        break;
    case QProcess::Starting:
        text = "Booting";
        color = QColor(255,255,0);
        break;
    case QProcess::Running:
        text = "Active";
        color = Qt::green;
        break;
    }

    mLangStatus->setText(text);
    mLangStatus->setTextColor(color);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!quit()) event->ignore();
}

void MainWindow::toggleComandLineFocus()
{
    QWidget *cmd = cmdLine();
    if(cmd->hasFocus()) {
        QWidget *editor = mEditors->currentEditor();
        if(editor) editor->setFocus(Qt::OtherFocusReason);
    }
    else
        cmd->setFocus(Qt::OtherFocusReason);
}

void MainWindow::showSettings()
{
    Settings::Dialog dialog(mMain->settings());
    dialog.resize(700,400);
    int result = dialog.exec();
    if( result == QDialog::Accepted )
        mMain->applySettings();
}

QWidget *MainWindow::cmdLine()
{
    static QWidget *widget = 0;
    if(!widget) {
        CmdLine *w = new CmdLine("Command line:");
        connect(w, SIGNAL(invoked(QString, bool)),
                mMain->scProcess(), SLOT(evaluateCode(QString, bool)));
        widget = w;
    }
    return widget;
}

void MainWindow::evaluateSelection()
{
    // Evaluate selection, if any, otherwise current line

    CodeEditor *editor = mEditors->currentEditor();
    if (!editor)
        return;

    QString text;
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection())
        text = cursor.selectedText();
    else {
        text = cursor.block().text();
        if( mEditors->stepForwardEvaluation() ) {
            QTextCursor newCursor = cursor;
            newCursor.movePosition(QTextCursor::NextBlock);
            newCursor.movePosition(QTextCursor::EndOfBlock);
            editor->setTextCursor(newCursor);
        }
        // Adjust cursor for code blinking:
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }

    if (text.isEmpty()) return;

    // NOTE: QTextDocument contains unicode paragraph separators U+2029
    text.replace( QChar( 0x2029 ), QChar( '\n' ) );

    Main::instance()->scProcess()->evaluateCode(text);

    editor->blinkCode(cursor);
}

void MainWindow::evaluateRegion()
{
    CodeEditor *editor = mEditors->currentEditor();
    if (!editor)
        return;

    QTextCursor region = editor->currentRegion();
    if(region.isNull()) {
        // TODO: post a warning in status bar
        return;
    }

    QString text = region.selectedText();
    text.replace( QChar( 0x2029 ), QChar( '\n' ) );

    Main::instance()->scProcess()->evaluateCode(text);

    editor->blinkCode(region);
}

void MainWindow::evaluateCurrentFile()
{
    CodeEditor *editor = mEditors->currentEditor();
    if (!editor)
        return;

    QString documentText = editor->document()->textDocument()->toPlainText();
    Main::instance()->scProcess()->evaluateCode(documentText);
}

void MainWindow::helpForSelection()
{
    CodeEditor *editor = mEditors->currentEditor();
    if (editor)
    {
        QString code = QString("\"%1\".help").arg(editor->textCursor().selectedText());
        Main::instance()->scProcess()->evaluateCode(code, true);
    }
}

//////////////////////////// StatusLabel /////////////////////////////////

StatusLabel::StatusLabel(QWidget *parent) : QLabel(parent)
{
    setAutoFillBackground(true);
    setMargin(3);
    setAlignment(Qt::AlignCenter);
    setBackground(Qt::black);
    setTextColor(Qt::white);
}

void StatusLabel::setBackground(const QBrush & brush)
{
    QPalette plt(palette());
    plt.setBrush(QPalette::Window, brush);
    setPalette(plt);
}

void StatusLabel::setTextColor(const QColor & color)
{
    QPalette plt(palette());
    plt.setColor(QPalette::WindowText, color);
    setPalette(plt);
}

} // namespace ScIDE
