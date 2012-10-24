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
#include <QToolButton>
#include <QDebug>

namespace ScIDE {

void Style::polish ( QWidget * widget )
{
    if (qobject_cast<QDockWidget*>(widget)) {
        return;
    }
    if ( (qobject_cast<QTabBar*>(widget))
         || (qobject_cast<QToolButton*>(widget)) )
    {
        static QPalette palette( QColor(85,85,85), QColor(85,85,85) );
        widget->setPalette(palette);
        // TODO:
        QProxyStyle::polish(widget);
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

void Style::drawComplexControl
( ComplexControl control, const QStyleOptionComplex * option,
  QPainter * painter, const QWidget * widget ) const
{
    switch(control) {
    case QStyle::CC_ToolButton:
    {
        const QToolButton *toolBtn = qobject_cast<const QToolButton*>(widget);
        if (!toolBtn)
            break;

        Q_ASSERT(toolBtn);

        const QStyleOptionToolButton *toolOption =
                static_cast<const QStyleOptionToolButton*>(option);

        painter->save();

        QRect r = option->rect;

        if (option->state & QStyle::State_On) {
            painter->setBrush( QColor(60,60,60) );
            painter->setPen( QColor(40,40,40) );
            painter->drawRect( r.adjusted(0,0,-1,-1) );
        }
        else {
            r.adjust(0,0,0,-1);

            QColor fill = option->state & QStyle::State_MouseOver
                    ? QColor(120,120,120)
                    : QColor(85,85,85);

            QColor edge =  option->state & QStyle::State_MouseOver
                    ? QColor(140,140,140)
                    : QColor(110,110,110);

            painter->setBrush(fill);
            painter->setPen(Qt::NoPen);
            painter->drawRect(r);

            painter->setPen(edge);
            painter->drawLine( r.topLeft(), r.topRight() );

            if (toolBtn->arrowType() == Qt::LeftArrow) {
                painter->setPen( QColor(50,50,50) );
                painter->drawLine( option->rect.topLeft(), option->rect.bottomLeft() );
            }
            else if (toolBtn->arrowType() == Qt::RightArrow) {
                painter->setPen( QColor(50,50,50) );
                painter->drawLine( option->rect.topRight(), option->rect.bottomRight() );
            }
        }

        painter->restore();

        QIcon icon = toolOption->icon;
        if (!icon.isNull())
        {
            QIcon::Mode iconMode =
                    option->state & QStyle::State_Enabled
                    ? ( option->state & QStyle::State_MouseOver
                        ? QIcon::Active
                        : QIcon::Normal )
                    : QIcon::Disabled;

            QIcon::State iconState =
                    option ->state & QStyle::State_Selected
                    ? QIcon::On : QIcon::Off;

            QPixmap pixmap = icon.pixmap(toolOption->iconSize, iconMode, iconState);
            QRect pixRect = pixmap.rect();
            pixRect.moveCenter( option->rect.center() );
            painter->drawPixmap(pixRect.topLeft(), pixmap);
        }
        else {
            QStyle::PrimitiveElement elem = Style::PE_CustomBase;
            switch(toolBtn->arrowType()) {
            case Qt::LeftArrow:
                elem = PE_IndicatorArrowLeft; break;
            case Qt::RightArrow:
                elem = PE_IndicatorArrowRight; break;
            case Qt::DownArrow:
                elem = PE_IndicatorArrowDown; break;
            case Qt::UpArrow:
                elem = PE_IndicatorArrowUp; break;
            default:
                break;
            }
            if (elem != Style::PE_CustomBase) {
                drawPrimitive( elem, option, painter, widget );
            }
            else if (!toolOption->text.isEmpty()) {
                painter->drawText( toolOption->rect,
                                   Qt::AlignCenter | Qt::TextShowMnemonic,
                                   toolOption->text );
            }
        }
        return;
    }
    default:
        break;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void Style::drawControl
( ControlElement element, const QStyleOption * option,
  QPainter * painter, const QWidget * widget ) const
{
    switch(element) {
    case QStyle::CE_TabBarTab: {
        const QStyleOptionTabV3 *tabOption = static_cast<const QStyleOptionTabV3*>(option);

        painter->save();;

        bool highlight = tabOption->state & QStyle::State_Selected
                || tabOption->state & QStyle::State_MouseOver;

        QColor fill = highlight ? QColor(120,120,120) : QColor(85,85,85);

        QColor edge =  highlight ? QColor(140,140,140) : QColor(110,110,110);

        QRect r = tabOption->rect.adjusted(0,0,0,-1);

        painter->setBrush( fill );
        painter->setPen( Qt::NoPen );
        painter->drawRect(r);

        painter->setPen( edge );
        painter->drawLine( r.topLeft(), r.topRight() );

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
        painter->drawText( textRect,
                           Qt::AlignVCenter | Qt::AlignLeft | Qt::TextShowMnemonic,
                           tabOption->text );
        return;
    }
    case CE_Splitter:
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawRect( option->rect );
        painter->restore();
        return;
    case CE_MenuBarEmptyArea:
        painter->save();
        painter->setPen(Qt::black);
        painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        painter->restore();
        return;
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
    case QStyle::PE_IndicatorTabTear:
    case QStyle::PE_FrameTabBarBase:
        return;
    case PE_IndicatorDockWidgetResizeHandle:
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawRect( option->rect );
        painter->restore();
        return;
    /*case PE_PanelMenuBar:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        painter->save();
        painter->setPen(Qt::black);
        painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        painter->restore();
        return;*/
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
    case QStyle::PM_TabBarTabOverlap:
        return 0;
    case QStyle::PM_TabBarIconSize:
        return 16;
    case PM_TabCloseIndicatorHeight:
    case PM_TabCloseIndicatorWidth:
        return 16;
    case PM_TabBarScrollButtonWidth:
        return 24;
    case PM_TabBar_ScrollButtonOverlap:
        return 1;
    case QStyle::PM_MenuBarVMargin:
        return 1;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int Style::styleHint
( StyleHint hint, const QStyleOption * option, const QWidget * widget,
  QStyleHintReturn * returnData ) const
{
    switch(hint) {
    default:
        break;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

} // namespace ScIDE
