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

#ifndef SCIDE_WIDGETS_CODE_EDITOR_AUTOCOMPLETER_HPP_INCLUDED
#define SCIDE_WIDGETS_CODE_EDITOR_AUTOCOMPLETER_HPP_INCLUDED

#include <QObject>
#include <QTextDocument>
#include <QStack>
#include <QKeyEvent>
#include <QStringList>
#include <QSortFilterProxyModel>
#include <QPointer>

namespace ScIDE {

class CodeEditor;
class TokenIterator;
class ScRequest;
class CompletionMenu;

class AutoCompleter : public QObject
{
    Q_OBJECT

public:
    AutoCompleter( CodeEditor * );

    void completeName();
    void aidMethodCall();
    void keyPress( QKeyEvent * );
    void documentChanged( QTextDocument * );

protected:
    virtual bool eventFilter( QObject *, QEvent * );

private slots:
    void onContentsChange(int pos, int removed, int added);
    void onCursorChanged();
    void onResponse( const QString & cmd, const QString & data );
    void onCompletionMenuFinished( int result );

private:
    struct MethodCall {
        int position;
        QString name;
#if 0
        QString className;
        QString methodName;
        QStringList argNames;
        QStringList argDefaults;
#endif
    };

    typedef QStack<MethodCall>::iterator MethodCallIterator;

    QTextDocument *document();

    void checkStack( int cursorPos );

    void startCompletion();
    void quitCompletion( const QString & reason = QString() );

    QString execCompletionMenu( int cursorPos, const QString & data );
    void updateCompletionMenu();

    void pushMethodCall( int pos, const QString & name, int arg = 0 );
    void showMethodCall( const MethodCall & call, int arg = 0 );
    void hideMethodCall();
    QString tokenText( TokenIterator & it );

    CodeEditor *mEditor;
    ScRequest *mScRequest;

    struct {
        bool on;
        int pos;
        int len;
        int contextPos;
        QString text;
        QPointer<CompletionMenu> menu;
    } mCompletion;

    QStack<MethodCall> mMethodCallStack;
};

} // namespace ScIDE

#endif
