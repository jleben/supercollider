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

#ifndef SCIDE_WIDGETS_BUS_MONITOR_VIEW_INCLUDED
#define SCIDE_WIDGETS_BUS_MONITOR_VIEW_INCLUDED

#include "util/docklet.hpp"

#include <QWidget>

namespace ScIDE {

class Main;
class ScBusMonitor;

class BusMonitorView : public QWidget
{
public:
    BusMonitorView(Main*, QWidget * parent = 0 );
    QSize sizeHint() const { return QSize(100,100); }
private:
    void paintEvent( QPaintEvent * );
    ScBusMonitor *mMonitor;
    int mDisplayBusOffset;
    int mDisplayBusCount;
};

class BusMonitorDocklet: public Docklet
{
public:
    BusMonitorDocklet(Main * main, QWidget * parent = 0):
        Docklet(tr("Bus Monitor"), parent)
    {
        setWidget(new BusMonitorView(main));
    }
};

}
#endif // SCIDE_WIDGETS_BUS_MONITOR_VIEW_INCLUDED
