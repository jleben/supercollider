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

#include "dock_widget.hpp"

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QFontMetrics>
#include <QEvent>
#include <QDebug>

namespace ScIDE {

DockWidgetToolBar::DockWidgetToolBar(const QString &title)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mOptionsMenu = new QMenu(this);

    QToolButton *optionsBtn = new QToolButton;
    optionsBtn->setIcon( optionsBtn->style()->standardIcon(QStyle::SP_TitleBarNormalButton) );
    optionsBtn->setIconSize( QSize(16,16) );
    optionsBtn->setMenu( mOptionsMenu );
    optionsBtn->setPopupMode( QToolButton::InstantPopup );
    optionsBtn->setToolButtonStyle( Qt::ToolButtonIconOnly );
    optionsBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setMargin(5);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    l->addWidget( optionsBtn );
    l->addWidget( titleLabel );
    setLayout(l);
}

void DockWidgetToolBar::addAction (QAction *action)
{
    QToolButton *btn = new QToolButton;
    btn->setIconSize( QSize(16,16) );
    btn->setDefaultAction( action );
    if (!btn->icon().isNull())
        btn->setToolButtonStyle( Qt::ToolButtonIconOnly );
    btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    if (layout())
        layout()->addWidget( btn );
}

void DockWidgetToolBar::addWidget (QWidget *widget, int stretch)
{
    static_cast<QHBoxLayout*>(layout())->addWidget(widget, stretch);
}

void DockWidgetToolBar::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);

    QRect r = rect();

    painter.setBrush( palette().color(QPalette::Mid) );
    painter.setPen(Qt::NoPen);
    painter.drawRect(r);

    painter.setPen( palette().color(QPalette::Shadow) );
    painter.drawLine( r.bottomLeft(), r.bottomRight() );

    painter.setPen( palette().color(QPalette::Mid).lighter(120) );
    painter.drawLine( r.topLeft(), r.topRight() );
}


static void updateWindowState( QWidget * window, QDockWidget::DockWidgetFeatures features )
{
    Qt::WindowFlags flags = window->windowFlags();
    if (features & QDockWidget::DockWidgetClosable)
        flags |= Qt::WindowCloseButtonHint;
    else
        flags &= ~Qt::WindowCloseButtonHint;
    window->setWindowFlags( flags );
}

DockWidget::DockWidget( const QString & title, QWidget * parent ):
    QDockWidget(title, parent),
    mWidget(0),
    mWindow(0)
{
    mToolBar = new DockWidgetToolBar(title);
    setTitleBarWidget(mToolBar);

    QMenu *optionsMenu = mToolBar->optionsMenu();
    QAction *action;

    mFloatAction = action = optionsMenu->addAction(tr("Undock"));
    action->setEnabled( features() & QDockWidget::DockWidgetFloatable );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(toggleFloating()) );

    mDetachAction = action = optionsMenu->addAction(tr("Detach"));
    action->setEnabled( features() & QDockWidget::DockWidgetFloatable );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(toggleDetached()) );

    action = optionsMenu->addAction(tr("Close"));
    action->setEnabled( features() & QDockWidget::DockWidgetClosable );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(close()) );

    mVisibilityAction = action = new QAction(title, this);
    action->setCheckable(true);
    connect( action, SIGNAL(triggered(bool)), this, SLOT(setPresent(bool)) );

    connect( this, SIGNAL(topLevelChanged(bool)), this, SLOT(onFloatingChanged(bool)) );
    connect( this, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)),
             this, SLOT(onFeaturesChanged(QDockWidget::DockWidgetFeatures)) );
}

void DockWidget::toggleFloating()
{
    bool undock = !isFloating();
    if (!undock && isDetached()) {
        setDetached(false);
    }
    setFloating( undock );
}

void DockWidget::toggleDetached()
{
    setDetached( !isDetached() );
}

void DockWidget::setDetached( bool detach )
{
    if (isDetached() == detach)
        return;

    Q_ASSERT(!isDetached() || mWindow != NULL);

    if(detach) {
        if (!isFloating())
            setFloating( true ); // at least for the sake of dock/undock action

        hide();

        QDockWidget::setWidget(0);
        QDockWidget::setTitleBarWidget(0);

        QVBoxLayout *layout;

        if (!mWindow) {
            mWindow = new QWidget();
            layout = new QVBoxLayout();
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(0);
            mWindow->setLayout( layout );
            mWindow->setWindowTitle( windowTitle() );
            mWindow->installEventFilter(this);
            updateWindowState( mWindow, features() );
        }
        else
            layout = qobject_cast<QVBoxLayout*>(mWindow->layout());

        layout->addWidget(mToolBar);
        layout->addWidget(mWidget);

        if (!mUndockedGeom.isNull())
            mWindow->setGeometry( mUndockedGeom );

        mWidget->show();
        mToolBar->show();
        mWindow->show();
    }
    else {
        mWindow->hide();

        QDockWidget::setTitleBarWidget(mToolBar);
        QDockWidget::setWidget(mWidget);

        if (isFloating() && !mUndockedGeom.isNull())
            setGeometry( mUndockedGeom );

        show();
    }

    mDetachAction->setText( detach ? tr("Attach") : tr("Detach") );
}

void DockWidget::setPresent( bool present )
{
    if (isDetached())
        mWindow->setVisible(present);
    else
        QDockWidget::setVisible(present);
}

void DockWidget::close()
{
    setPresent(false);
}

void DockWidget::onFloatingChanged( bool floating )
{
    mFloatAction->setText( floating ? tr("Dock") : tr("Undock") );
}

void DockWidget::onFeaturesChanged ( QDockWidget::DockWidgetFeatures features )
{
    if( mWindow )
        updateWindowState( mWindow, features );
}

bool DockWidget::event( QEvent *event )
{
    switch(event->type()) {
    case QEvent::Show:
        mVisibilityAction->setChecked(true);
        break;
    case QEvent::Hide:
        mVisibilityAction->setChecked(false);
        break;
    case QEvent::Resize:
    case QEvent::Move:
        if(isFloating())
            mUndockedGeom = geometry();
        break;
    default:
        break;
    }

    return QDockWidget::event(event);
}

bool DockWidget::eventFilter( QObject *object, QEvent * event )
{
    if (object == mWindow) {
        switch(event->type()) {
        case QEvent::Show:
            mVisibilityAction->setChecked(true);
            break;
        case QEvent::Hide:
            mVisibilityAction->setChecked(false);
            break;
        case QEvent::Resize:
        case QEvent::Move:
            mUndockedGeom = mWindow->geometry();
            break;
        default:
            break;
        }
    }

    return QDockWidget::eventFilter(object, event);
}

} // namespace ScIDE
