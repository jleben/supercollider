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

#include "bus_monitor.hpp"
#include "../sc_server.hpp"
#include "../sc_process.hpp"

#include <QDebug>
#include <iostream>

using namespace std;

namespace ScIDE {

ScBusMonitor::ScBusMonitor( ScServer *server, ScProcess *lang, QObject *parent ):
    QObject(parent),
    mServer(server),
    mLang(lang)
{
    connect(mServer, SIGNAL(runningStateChange(bool,QString,int)),
            this, SLOT(onServerStateChanged(bool)));
    /*connect(mServer, SIGNAL(received(osc::ReceivedMessage)),
            this, SLOT(onServerMessage(osc::ReceivedMessage)));*/

    mServer->subscribe("/server-audio-bus-levels",
                       this, SLOT(onServerMessage(osc::ReceivedMessage)));
}

void ScBusMonitor::onServerStateChanged( bool running )
{
    qDebug() << "ScBusMonitor: server running:" << running;

    if (running)
    {
        QString play_bus_monitor =
                "SynthDef(\"audio-bus-levels\", {"
                    "var in = In.ar(0, 5);"
                    "SendPeakRMS.kr(in, 3, 3, \"/server-audio-bus-levels\")"
                "}).play(RootNode(Server.default), nil, 'addToTail')";

        mLang->evaluateCode(play_bus_monitor, true);
    }
}

void ScBusMonitor::onServerMessage( const osc::ReceivedMessage & message )
{
    qDebug("ScBusMonitor: server levels received!");

    unsigned int arg_count = message.ArgumentCount();

    if (mBuffer.count() != arg_count)
        mBuffer.resize(arg_count);

    auto args = message.ArgumentStream();

    try
    {
        int dummy;
        args >> dummy;
        args >> dummy;

        for (int i = 2; i < arg_count; ++i)
        {
            args >> mBuffer[i];
            cout << mBuffer[i] << " ";
        }
        cout << endl;
    }
    catch (osc::WrongArgumentTypeException)
    {
        qCritical("ScBusMonitor: Misformatted OSC message.");
        return;
    }

    emit update();
}

}
