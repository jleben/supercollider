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

#ifndef SCIDE_WIDGETS_UTIL_DOCK_WIDGET_TITLE_BAR_HPP_INCLUDED
#define SCIDE_WIDGETS_UTIL_DOCK_WIDGET_TITLE_BAR_HPP_INCLUDED

#include <QWidget>
#include <QToolButton>
#include <QAction>
#include <QDockWidget>

namespace ScIDE {

class DockWidgetTitleBarButton : public QToolButton
{
public:
    DockWidgetTitleBarButton( QWidget * parent = 0 );

    virtual QSize sizeHint() const;

protected:
    virtual void enterEvent( QEvent * event );
    virtual void leaveEvent( QEvent * event );
    virtual void paintEvent( QPaintEvent *event );
};

class DockWidgetTitleBar : public QWidget
{
    Q_OBJECT

public:
    DockWidgetTitleBar( QDockWidget *widget );

    void addAction (QAction *action);
    void addWidget (QWidget *widget, int stretch = 0 );

private slots:
    void toggleFloating();
    void onFloatingChanged( bool floating );

protected:
    virtual void paintEvent( QPaintEvent *event );

private:
    QDockWidget *mDockWidget;
    QAction *mDockAction;
};

} // namespace ScIDE

#endif // SCIDE_WIDGETS_UTIL_DOCK_WIDGET_TITLE_BAR_HPP_INCLUDED
