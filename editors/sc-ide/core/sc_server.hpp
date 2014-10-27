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

#ifndef SCIDE_SC_SERVER_HPP_INCLUDED
#define SCIDE_SC_SERVER_HPP_INCLUDED

#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QAction>
#include <QProcess>
#include <QTimer>
#include <QHash>
#include <boost/chrono/system_clocks.hpp>
#include <osc/OscReceivedElements.h>
#include <osc/OscOutboundPacketStream.h>
#include <cstdint>

//Q_DECLARE_METATYPE(osc::ReceivedMessage);
//Q_DECLARE_METATYPE(osc::OutboundPacketStream);

namespace ScIDE {

class ScProcess;
class VolumeWidget;
namespace Settings { class Manager; }

class OscMessageHandler : public QObject
{
    Q_OBJECT
    friend class ScServer;
public:
    OscMessageHandler( QObject * parent = 0): QObject(parent) {}
signals:
    void received( const osc::ReceivedMessage & );
};

class ScServer : public QObject
{
    Q_OBJECT

public:
    enum ActionRole {
        ToggleRunning,
        Boot,
        Quit,
        KillAll,
        Reboot,
        ShowMeters,
        ShowScope,
        ShowFreqScope,
        DumpNodeTree,
        DumpNodeTreeWithControls,
        PlotTree,
        DumpOSC,
        Mute,
        Volume,
        VolumeUp,
        VolumeDown,
        VolumeRestore,
        Record,

        ActionCount
    };

    enum State
    {
        Inactive,
        Active
    };

    ScServer(ScProcess *scLang, Settings::Manager * settings, QObject * parent);

    bool isActive() const { return mPort != 0 && mId != -1; }
    State state() const { return isActive() ? Active : Inactive; }
    QHostAddress address() const { return mServerAddress; }
    int port() const { return mPort; }

    QAction *action(ActionRole role) { return mActions[role]; }

    float volume() const;
    void setVolume( float volume );

    bool isMuted() const;
    void setMuted( bool muted );

    bool isDumpingOSC() const;
    void setDumpingOSC( bool dumping );

    bool isRecording() const;
    boost::chrono::seconds recordingTime() const;

    void subscribe( const QString & message, QObject *, const char *slot );

public slots:
    void boot();
    void reboot();
    void quit();
    void killAll();
    void toggleRunning();
    void showMeters();
    void showScope();
    void showFreqScope();
    void dumpNodeTree();
    void dumpNodeTreeWithControls();
    void plotTree();
    void queryAllNodes(bool dumpControls);
    void increaseVolume();
    void decreaseVolume();
    void changeVolume(float);
    void restoreVolume();
    void mute() { setMuted(true); }
    void unmute() { setMuted(false); }
    void setRecording( bool active );
    void send( const osc::OutboundPacketStream & );

signals:
    void stateChanged( int state );
    void updateServerStatus (int ugenCount, int synthCount,
                             int groupCount, int defCount,
                             float avgCPU, float peakCPU);
    void volumeChanged( float volume );
    void mutedChanged( bool muted );
    void recordingChanged( bool recording );
    void received( const osc::ReceivedMessage & );

private slots:
    void onScLangStateChanged( QProcess::ProcessState );
    void onScLangReponse( const QString & selector, const QString & data );
    void onServerDataArrived();
    void updateToggleRunningAction();
    void updateRecordingAction();
    void updateEnabledActions();
    void sendMuted( bool muted );
    void sendVolume( float volume );
    void sendDumpingOSC( bool dumping );

protected:
    virtual void timerEvent(QTimerEvent * event);

private:
    class NodeIdAllocator
    {
    public:
        NodeIdAllocator():
            mMask(0),
            mNextID(0)
        {}

        NodeIdAllocator(int32_t globalId)
        {
            mMask = globalId << 26;
            mNextID = 0;
        }

        int32_t next()
        {
            int32_t id = mNextID | mMask;
            mNextID = ++mNextID % 0x03FFFFFF;
            return id;
        }

    private:
        int32_t mMask;
        int32_t mNextID;
    };

private:
    void createActions( Settings::Manager * );
    void handleRuningStateChangedMsg( const QString & data );
    void onStart( QString const & hostName, int port );
    void onDoneRegistration( const osc::ReceivedMessage & );
    void onStop();

    void processOscPacket( const osc::ReceivedPacket & packet )
    {
        if (packet.IsMessage())
            processOscMessage( osc::ReceivedMessage(packet) );
        else
            processOscBundle( osc::ReceivedBundle(packet) );
    }

    void processOscBundle( const osc::ReceivedBundle & bundle )
    {
        for (auto iter = bundle.ElementsBegin(); iter != bundle.ElementsEnd(); ++iter)
        {
            const osc::ReceivedBundleElement & element = *iter;
            if (element.IsMessage())
                processOscMessage( osc::ReceivedMessage(element) );
            else
                processOscBundle( osc::ReceivedBundle(element) );
        }
    }

    void processOscMessage( const osc::ReceivedMessage & );
    void processServerStatusMessage( const osc::ReceivedMessage & );
    void processCommandFailureMessage( const osc::ReceivedMessage & );


#if 0
    template<typename T>
    bool sendOscMessage( osc::OutboundPacketStream & stream, const T & arg)
    {
        stream << arg;
        sendOscMessage(stream, arg);

        stream << osc::MessageTerminator();
    }

    template<typename H, typename ...T>
    bool sendOscMessage( osc::OutboundPacketStream & stream,
                         const H & arg, const T & args ...)
    {
        stream << arg;
        sendOscMessage(stream, args...);
    }

    template<typename H, typename ...T>
    bool sendOscMessage( size_t max_bytes, const char *address,
                         const H & arg, const T & ... args)
    {

    }
#endif
    ScProcess *mLang;

    QUdpSocket * mUdpSocket;
    QHostAddress mServerAddress;
    int mPort;

    int32_t mId;
    NodeIdAllocator mNodeIds;

    QAction * mActions[ActionCount];

    VolumeWidget *mVolumeWidget;
    QTimer mRecordTimer;
    boost::chrono::system_clock::time_point mRecordTime;
    bool mIsRecording;

    QHash<QString,OscMessageHandler*> mOscMessageHandlers;
};

}

#endif // SCIDE_SC_SERVER_HPP_INCLUDED
