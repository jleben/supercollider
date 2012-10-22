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

namespace ScIDE {

void Style::polish ( QWidget * widget )
{
    if (qobject_cast<QDockWidget*>(widget)) {
        QStyle::polish(widget);
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
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
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

int	Style::pixelMetric
( PixelMetric metric, const QStyleOption * option, const QWidget * widget ) const
{
    switch(metric) {
    case QStyle::PM_DockWidgetFrameWidth:
        return 2;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

} // namespace ScIDE
