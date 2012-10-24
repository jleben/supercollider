/*
    SuperCollider Qt IDE
    Copyright (c) 2010-2012 Jakob Leben
    Copyright (c) 2012 Tim Blechmann
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

#include "style.hpp"

#include <QPainter>
#include <QStyleOption>
#include <QDockWidget>
#include <QStyleOptionTabV3>
#include <QTabBar>
#include <QDebug>

namespace ScIDE {

void Style::polish ( QWidget * widget )
{
    if (qobject_cast<QDockWidget*>(widget)) {
        QStyle::polish(widget);
    }
    else if (QTabBar *tabBar = qobject_cast<QTabBar*>(widget)) {
        QProxyStyle::polish(widget);
        QPalette palette( tabBar->palette() );
        palette.setColor( QPalette::WindowText, Qt::white );
        palette.setColor( QPalette::Text, Qt::white );
        tabBar->setPalette(palette);
    }
    else
        QProxyStyle::polish(widget);

}

void Style::unpolish ( QWidget * widget )
{
    if (qobject_cast<QDockWidget*>(widget)) {
        QStyle::unpolish(widget);
    }
    else
        QProxyStyle::unpolish(widget);
}

void Style::drawControl
( ControlElement element, const QStyleOption * option,
  QPainter * painter, const QWidget * widget ) const
{
    switch(element) {
    case QStyle::CE_TabBarTab: {
        const QStyleOptionTabV3 *tabOption = static_cast<const QStyleOptionTabV3*>(option);

        painter->save();
        painter->setPen( Qt::NoPen );
        QColor fill;
        if (tabOption->state & QStyle::State_Selected || tabOption->state & QStyle::State_MouseOver)
            fill = QColor(120,120,120);
        else
            fill = QColor(85,85,85);
        painter->setBrush( fill );
        int lmargin = tabOption->position == QStyleOptionTab::Beginning ? 1 : 0;
        int rmargin = tabOption->position == QStyleOptionTab::End ? -1 : 0;
        painter->drawRect( tabOption->rect.adjusted(lmargin,2,rmargin,-1) );
        painter->restore();

        if (!tabOption->icon.isNull()) {
            QPixmap pixmap = tabOption->icon.pixmap( tabOption->iconSize );
            QRect iconRect = pixmap.rect();
            iconRect.moveCenter( tabOption->rect.center() );
            int lmargin = 5;
            if (tabOption->leftButtonSize.width() > 0)
                lmargin += tabOption->leftButtonSize.width() + 4;
            iconRect.moveLeft( tabOption->rect.left() + lmargin );
            painter->drawPixmap( iconRect, pixmap );
        }

        QRect textRect = subElementRect( QStyle::SE_TabBarTabText, option, widget );
        painter->drawText( textRect, Qt::AlignVCenter | Qt::AlignLeft, tabOption->text );
        return;
    }
    default:
        break;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void Style::drawPrimitive
( PrimitiveElement element, const QStyleOption * option,
  QPainter * painter, const QWidget * widget ) const
{
    switch (element) {
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

QRect Style::subElementRect
( SubElement element, const QStyleOption * option, const QWidget * widget ) const
{
    switch(element) {
    // NOTE: Assuming horizontal tab bar direction
    case QStyle::SE_TabBarTabRightButton: {
        QRect r;
        r.setSize(QSize(16, 16));
        r.moveCenter(option->rect.center());
        r.moveRight(option->rect.right() + 1 - 5);
        return r;
    }
    case QStyle::SE_TabBarTabLeftButton: {
        QRect r;
        r.setSize(QSize(16, 16));
        r.moveCenter(option->rect.center());
        r.moveLeft(option->rect.left() + 5);
        return r;
    }
    case QStyle::SE_TabBarTabText: {
        const QStyleOptionTabV3 *tabOption = static_cast<const QStyleOptionTabV3*>(option);

        int lMargin = 5;
        if (tabOption->leftButtonSize.width() > 0)
            lMargin += tabOption->leftButtonSize.width() + 4;
        if (!tabOption->icon.isNull())
            lMargin += tabOption->iconSize.width() + 4;

        int rMargin = 5;
        if (tabOption->rightButtonSize.width() > 0)
            rMargin += tabOption->rightButtonSize.width() + 4;

        QRect r = option->rect;
        r.adjust( lMargin, 0, rMargin, 0 );
        return r;
    }
    default:
        return QProxyStyle::subElementRect(element, option, widget);
    }
}

QSize Style::sizeFromContents
( ContentsType type, const QStyleOption * option, const QSize & contentsSize,
  const QWidget * widget ) const
{
    switch(type) {
    case QStyle::CT_TabBarTab:
        return contentsSize + QSize(10, 10);
    default:
        return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    }
}

int	Style::pixelMetric
( PixelMetric metric, const QStyleOption * option, const QWidget * widget ) const
{
    switch(metric) {
    case QStyle::PM_DockWidgetFrameWidth:
        return 2;
    case QStyle::PM_DockWidgetSeparatorExtent:
    case QStyle::PM_SplitterWidth:
        return 1;
    case QStyle::PM_TabBarBaseHeight:
    case QStyle::PM_TabBarTabHSpace:
    case QStyle::PM_TabBarTabVSpace:
    case QStyle::PM_TabBarTabShiftHorizontal:
    case QStyle::PM_TabBarTabShiftVertical:
        return 0;
    case QStyle::PM_TabBarIconSize:
        return 16;
    case PM_TabCloseIndicatorHeight:
        return 16;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

} // namespace ScIDE
