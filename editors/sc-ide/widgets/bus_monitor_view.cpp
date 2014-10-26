/*
    SuperCollider Qt IDE
    Copyright (c) 2014 Jakob Leben
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

#include "bus_monitor_view.hpp"
#include "../core/sc_server/bus_monitor.hpp"
#include "../core/main.hpp"

#include <QPainter>

#include <cmath>

namespace ScIDE {

BusMonitorView::BusMonitorView(Main* main, QWidget * parent):
    mDisplayBusOffset(0),
    mDisplayBusCount(8)
{
    mMonitor = new ScBusMonitor(main->scServer(), main->scProcess(), this);
    connect(mMonitor, SIGNAL(update()), this, SLOT(update()));
}

void BusMonitorView::paintEvent( QPaintEvent * )
{
    int busCount = mMonitor->busCount();

    if (!busCount || !mDisplayBusCount)
        return;

    QPainter painter(this);
     //painter.fillRect(rect(), Qt::white);

    int busWidth = width() / mDisplayBusCount;

    for (int i = mDisplayBusOffset;
         i < busCount &&
         i < mDisplayBusOffset + mDisplayBusCount;
         ++i)
    {
        float amp = mMonitor->peak(i);
        float db = std::log10(amp) * 20;
        db = std::max(-80.f, std::min(db, 0.f));
        db = (db / 80.f) + 1.f;

        int busHeight = db * height();
        if (!busHeight)
            continue;

        QRect r( i * busWidth + 1,
                 height() - busHeight,
                 busWidth - 2,
                 busHeight);

        //qDebug() << r;
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::green);
        painter.drawRect(r);
    }
}

}
