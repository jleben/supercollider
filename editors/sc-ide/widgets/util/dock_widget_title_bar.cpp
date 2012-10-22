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

#include "dock_widget_title_bar.hpp"

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QFontMetrics>

namespace ScIDE {

DockWidgetTitleBarButton::DockWidgetTitleBarButton( QWidget * parent ):
    QToolButton(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

QSize DockWidgetTitleBarButton::sizeHint() const
{
    QIcon icon = this->icon();
    QSize contentSize;
    if (!icon.isNull())
        return iconSize() + QSize(10,10);
    QString text = this->text();
    if (!text.isEmpty())
        return QFontMetrics(font()).boundingRect(text).size() + QSize(10,10);
    return QSize();
}

void DockWidgetTitleBarButton::enterEvent( QEvent * event )
{
    QToolButton::enterEvent(event);
    update();
}

void DockWidgetTitleBarButton::leaveEvent( QEvent * event )
{
    QToolButton::leaveEvent(event);
    update();
}

void DockWidgetTitleBarButton::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);

    if (isEnabled() && (isChecked() || underMouse())) {
        painter.save();

        QRect r = rect();

        if (isChecked()) {
            QColor fill = QColor(60,60,60);
            QColor border = QColor(40,40,40);
            painter.setPen( border );
            painter.setBrush( fill );
            r.adjust(0,0,-1,-1);
        } else {
            painter.setPen( Qt::NoPen );
            QColor fill = Qt::white;
            fill.setAlpha(50);
            painter.setBrush( fill );
            r.adjust(1,1,-1,-1);
        }

        painter.drawRect(r);

        painter.restore();
    }

    QIcon icon = this->icon();
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(iconSize(),
                                     isEnabled() ? QIcon::Normal
                                                 : QIcon::Disabled,
                                     isChecked() ? QIcon::On
                                                 : QIcon::Off);
        QRect pixRect = pixmap.rect();
        pixRect.moveCenter( rect().center() );
        painter.drawPixmap(pixRect.topLeft(), pixmap);
        return;
    }

    QString text = this->text();
    if (!text.isEmpty()) {
        style()->drawItemText( &painter, rect(), Qt::AlignCenter, palette(),
                               isEnabled(), text, QPalette::Text );
    }
}

DockWidgetTitleBar::DockWidgetTitleBar( QDockWidget *widget ):
    mDockWidget(widget)
{
    QPalette palette( this->palette() );
    palette.setColor( QPalette::WindowText, Qt::white );
    palette.setColor( QPalette::Text, Qt::white );
    setPalette(palette);

    QAction *action;
    QMenu *optionsMenu = new QMenu(this);

    mDockAction = action = optionsMenu->addAction(QString());
    action->setEnabled( widget->features() & QDockWidget::DockWidgetFloatable );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(toggleFloating()) );

    action = optionsMenu->addAction(tr("Close"));
    action->setEnabled( widget->features() & QDockWidget::DockWidgetClosable );
    connect( action, SIGNAL(triggered(bool)), widget, SLOT(hide()) );

    DockWidgetTitleBarButton *optionsBtn = new DockWidgetTitleBarButton;
    optionsBtn->setIcon( style()->standardIcon(QStyle::SP_TitleBarNormalButton) );
    optionsBtn->setMenu( optionsMenu );
    optionsBtn->setPopupMode( QToolButton::InstantPopup );

    QLabel *titleLabel = new QLabel(widget->windowTitle());
    titleLabel->setMargin(5);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    l->addWidget( optionsBtn );
    l->addWidget( titleLabel );
    setLayout(l);

    connect( widget, SIGNAL(topLevelChanged(bool)), this, SLOT(onFloatingChanged(bool)) );

    onFloatingChanged(false);
}

void DockWidgetTitleBar::addAction (QAction *action)
{
    DockWidgetTitleBarButton *btn = new DockWidgetTitleBarButton;
    btn->setIconSize( QSize(16,16) );
    btn->setDefaultAction( action );
    layout()->addWidget( btn );
}

void DockWidgetTitleBar::addWidget (QWidget *widget, int stretch)
{
    static_cast<QHBoxLayout*>(layout())->addWidget(widget, stretch);
}

void DockWidgetTitleBar::toggleFloating()
{
    mDockWidget->setFloating( !mDockWidget->isFloating() );
}

void DockWidgetTitleBar::onFloatingChanged( bool floating )
{
    mDockAction->setText( floating ? tr("Dock") : tr("Undock") );
}

void DockWidgetTitleBar::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);

    painter.setBrush( QColor(85,85,85) );
    painter.setPen( QColor(50,50,50) );
    painter.drawRect( rect().adjusted(0,0,-1,-1) );

    painter.setPen( QColor(110,110,110) );
    painter.drawLine( 1, 1, rect().right() - 1, 1 );
}

} // namespace ScIDE
