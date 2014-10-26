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

#ifndef SC_IDE_SERVER_BUS_MONITOR_INCLUDED
#define SC_IDE_SERVER_BUS_MONITOR_INCLUDED

#include <QObject>
#include <QVector>
#include <osc/OscReceivedElements.h>

namespace ScIDE {

class ScServer;
class ScProcess;

class ScBusMonitor : public QObject
{
    Q_OBJECT
public:
    ScBusMonitor( ScServer *server, ScProcess *lang, QObject *parent = 0 );
    int busCount();
    float peak(int bus);
    float rms(int bus);

signals:
    void update();

private slots:
    void onServerStateChanged( bool running );
    void onLevelsReceived( const osc::ReceivedMessage & );

private:
    ScServer *mServer;
    ScProcess *mLang;
    QVector<float> mBuffer;
};

}

#endif // SC_IDE_SERVER_BUS_MONITOR_INCLUDED
