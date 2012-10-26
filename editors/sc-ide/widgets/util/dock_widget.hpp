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

class DockWidgetToolBar : public QWidget
{
    Q_OBJECT
public:
    DockWidgetToolBar(const QString & title);

    void addAction (QAction *action);
    void addWidget (QWidget *widget, int stretch = 0 );
    QMenu *optionsMenu () { return mOptionsMenu; }

protected:
    virtual void paintEvent( QPaintEvent *event );
    QMenu *mOptionsMenu;
};

class DockWidget : public QDockWidget
{
    Q_OBJECT
public:
    DockWidget( const QString & title, QWidget * parent = 0 );

    DockWidgetToolBar *toolBar() { return mToolBar; }

    void setWidget( QWidget *widget ) {
        mWidget = widget;
        if (!isDetached())
            QDockWidget::setWidget(widget);
    }

    QWidget *widget () { return mWidget; }

    QAction *toggleViewAction() { return mVisibilityAction; }

    bool isDetached() const { return mToolBar->parent() != this; }
    void setDetached( bool detached );
    QWidget *window() { return mWindow; }

public slots:
    void toggleFloating();
    void toggleDetached();
    void setPresent( bool present );
    void close();

private slots:
    void onFloatingChanged( bool floating );
    void onFeaturesChanged ( QDockWidget::DockWidgetFeatures features );

protected:
    virtual bool event( QEvent * );
    virtual bool eventFilter(QObject *object, QEvent * event);

private:
    QAction *mFloatAction;
    QAction *mDetachAction;
    QAction *mVisibilityAction;

    DockWidgetToolBar *mToolBar;
    QWidget *mWidget;
    QWidget *mWindow;

    QRect mUndockedGeom;
};

} // namespace ScIDE

#endif // SCIDE_WIDGETS_UTIL_DOCK_WIDGET_TITLE_BAR_HPP_INCLUDED
