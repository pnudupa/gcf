/****************************************************************************
**
** Copyright (C) VCreate Logic Private Limited, Bangalore
**
** Use of this file is limited according to the terms specified by
** VCreate Logic Private Limited, Bangalore.  Details of those terms
** are listed in licence.txt included as part of the distribution package
** of this file. This file may not be distributed without including the
** licence.txt file.
**
** Contact info@vcreatelogic.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "IpcServerDiscovery.h"

#include "../Core/Log.h"
#include "IpcServer.h"

#include <QSysInfo>
#include <QProcess>
#include <QUdpSocket>
#include <QBasicTimer>
#include <QDataStream>
#include <QTimerEvent>
#include <QNetworkInterface>
#include <QProcessEnvironment>

/**
\struct GCF::IpcServerInfo IpcServerDiscovery.h <GCF3/IpcServerDiscovery>
\brief This structure contains key information about a discovered GCF::IpcServer
\ingroup gcf_ipc


 */

/**
\var GCF::IpcServerInfo::User

This variable contains the name of the user on the computer where the IpcServer was
discovered.
 */

/**
\var GCF::IpcServerInfo::Address

This variable contains the IP address of the computer where the IpcServer was discovered.
 */

/**
\var GCF::IpcServerInfo::Port

This variable contains the TCP port on which the IpcServer is listening.
 */

/**
\fn bool GCF::IpcServerInfo::isValid() const

\return true if the information contained in this structure describes a valid IpcServer
information.
 */

/**
\fn bool GCF::IpcServerInfo::operator == (const GCF::IpcServerInfo &other) const

Comparison operator. Returns true if the objects were equal. False otherwise.
 */

namespace GCF
{

struct IpcServerDiscoveryData
{
    IpcServerDiscoveryData() : broadcastTimerTimeout(2000),
        defaultBroadcastPort(55001), broadcastPort(55001) { }

    static QString user();

    QList<QHostAddress> broadcastAddresses;
    QList<QHostAddress> ipAddresses;
    void updateAddresses();
    bool isMyAddress(const QHostAddress &address) const;

    QUdpSocket broadcastSocket;
    QBasicTimer broadcastTimer;
    const int broadcastTimerTimeout;
    const quint16 defaultBroadcastPort;
    quint16 broadcastPort;

    QList<GCF::IpcServerInfo> foundServers;
    bool addFoundServer(const GCF::IpcServerInfo &info) {
        // Check for duplicates
        if(this->foundServers.contains(info))
            return false;

        // Ensure that info doesnt loop back to a IpcServer
        // within this application process itself.
        QList<GCF::IpcServer*> servers = GCF::IpcServer::servers();
        Q_FOREACH(GCF::IpcServer *server, servers) {
            if(!info.ServerId.isEmpty() && server->serverId() == info.ServerId)
                return false;
            if(info.Port == server->serverPort())
                return false;
        }

        // Genuine record.
        this->foundServers.append(info);
        return true;
    }
};

}

/**
\class GCF::IpcServerDiscovery IpcServerDiscovery <GCF3/IpcServerDiscovery>
\brief This class can be used to discover GCF::IpcServer instances on LAN.
\ingroup gcf_ipc


You can create an instance of this class in your application and invoke the
\ref start() method to begin looking for GCF::IpcServer instances in other
applications on the LAN that are open and listening. Everytime such a server
is found, this class emits a \ref foundServer() signal. You can get a list of
all found servers by calling \ref foundServers(). Once the \ref foundServer()
signal is emitted for a particular server, it wont be emitted again for the
same server. You can force the emission by removing the server from the list
of found servers using the \ref releaseFoundServer() method. Of release information
about all found servers by calling the \ref releaseAllFoundServers() method.

\note You can only discover GCF::IpcServer instances in those applications that
also have an instance of this class \ref start() "started" on the same port.
 */

/**
Constructor.

\param parent a \c QObject parent to which this object will become a child
 */
GCF::IpcServerDiscovery::IpcServerDiscovery(QObject *parent)
    :QObject(parent)
{
    d = new IpcServerDiscoveryData;

#if QT_VERSION < 0x050000
    // Otherwise foundServer() signal wont work on Qt 4.8.x
    static bool ipcServerInfoTypeRegistered = false;
    if(!ipcServerInfoTypeRegistered)
    {
        qRegisterMetaType<GCF::IpcServerInfo>("GCF::IpcServerInfo");
        ipcServerInfoTypeRegistered = true;
    }
#endif

    QString user = d->user(); // Just to get the user name
    if(user.isEmpty())
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                      "Could not figure out user-name");
    d->updateAddresses();

    connect(&d->broadcastSocket, SIGNAL(readyRead()), this, SLOT(readBroadcastDatagram()));
}

/**
Destructor. Stops discovery before destroying itself.
 */
GCF::IpcServerDiscovery::~IpcServerDiscovery()
{
    stop();
    delete d;
}

/**
\return user-name of this computer that will be broadcasted to other applications.
 */
QString GCF::IpcServerDiscovery::user()
{
    return GCF::IpcServerDiscoveryData::user();
}

/**
This function starts the discovery process. It does two things

\li Opens a UDP socket on the given \c port
\li Once in every 2 seconds, it broadcasts information about all GCF::IpcServer
instances in this application.
\li Begins listening for broadcast information packets sent by instances of
this class in other applications opened on the same port.
 */
GCF::Result GCF::IpcServerDiscovery::start(quint16 port)
{
    if(d->broadcastTimer.isActive())
        return GCF::Result(false, QString(), QString("Discovery has already been started"));

    d->broadcastPort = (port == 0) ? d->defaultBroadcastPort : port;
    bool success = d->broadcastSocket.bind(QHostAddress::Any, d->broadcastPort,
                                QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    if(!success)
        return GCF::Result(false, QString(), d->broadcastSocket.errorString());

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("Broadcast socket bound to port %1").arg(d->broadcastPort));
    d->broadcastTimer.start(d->broadcastTimerTimeout, this);

    return true;
}

/**
Stops discovering GCF::IpcServer instances and also stops broadcasting informtion
about GCF::IpcServer instances in this application.
 */
GCF::Result GCF::IpcServerDiscovery::stop()
{
    if(!d->broadcastTimer.isActive())
        return GCF::Result(false, QString(), "Discovery cannot be stopped as it has not yet been started");

    d->broadcastTimer.stop();
    d->broadcastSocket.close();
    return true;
}

/**
\return a list of found servers
 */
QList<GCF::IpcServerInfo> GCF::IpcServerDiscovery::foundServers() const
{
    return d->foundServers;
}

/**
Removes server-info (\c info ) from the list of found servers.
 */
void GCF::IpcServerDiscovery::releaseFoundServer(const GCF::IpcServerInfo &info)
{
    d->foundServers.removeAll( info );
}

/**
Removes all found server information.
 */
void GCF::IpcServerDiscovery::releaseAllFoundServers()
{
    d->foundServers.clear();
}

/**
\fn void GCF::IpcServerDiscovery::foundServer(const GCF::IpcServerInfo &info)

This signal is emitted whenever this class detects the existence of a
\ref GCF::IpcServer on the LAN.
*/

void GCF::IpcServerDiscovery::sendBroadcastDatagram()
{
    QList<GCF::IpcServer*> servers = GCF::IpcServer::servers();
    if(servers.isEmpty())
        return;

    QByteArray datagram("GCFIpcServerDiscovery@");

    QDataStream ds(&datagram, QIODevice::Append);
    ds << d->user();
    ds << (quint32)servers.count();
    Q_FOREACH(GCF::IpcServer *server, servers)
    {
        if(server->isListening())
        {
            ds << server->serverPort();
            ds << server->serverId();
        }
        else
        {
            ds << (quint16)0;
            ds << QString();
        }
    }

    bool validBroadcastAddresses = true;
    foreach(QHostAddress address, d->broadcastAddresses)
    {
        if (d->broadcastSocket.writeDatagram(datagram, address, d->broadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if(!validBroadcastAddresses)
        d->updateAddresses();
}

void GCF::IpcServerDiscovery::readBroadcastDatagram()
{
    while(d->broadcastSocket.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(d->broadcastSocket.pendingDatagramSize());

        QHostAddress address;
        quint16 port = 0;

        if(d->broadcastSocket.readDatagram(datagram.data(), datagram.size(), &address, &port) == -1)
            continue;

        const QByteArray header("GCFIpcServerDiscovery@");
        if(!datagram.startsWith(header) || port != d->broadcastPort)
            continue;

        datagram.remove(0, header.length());

        QDataStream ds(datagram);
        QString user;
        ds >> user;

        qint32 count = 0;
        ds >> count;
        for(int i=0; i<count; i++)
        {
            quint16 serverPort = 0;
            ds >> serverPort;

            QString serverId;
            ds >> serverId;

            GCF::IpcServerInfo info(user, address, serverPort, serverId);
            if(info.isValid() && d->addFoundServer(info))
                emit foundServer(info);
        }
    }
}

void GCF::IpcServerDiscovery::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == d->broadcastTimer.timerId())
        this->sendBroadcastDatagram();
}

QString GCF::IpcServerDiscoveryData::user()
{
    static QString retVal;
    if(!retVal.isEmpty())
        return retVal;

#ifdef Q_OS_WIN
    QString user = QProcessEnvironment::systemEnvironment().value("USERNAME");
#else
    QString user = QProcessEnvironment::systemEnvironment().value("USER");
#endif

    if(user.isEmpty())
        user = "localuser";

    QString host;
#ifdef Q_OS_LINUX
    QProcess proc;
    proc.start("uname", QStringList() << "-n");
    proc.waitForStarted();
    proc.waitForFinished();
    QString os = QString::fromLatin1(proc.readAllStandardOutput()).trimmed();
    if(os.isEmpty())
        host = "GNU/Linux";
    else
        host = QString("GNU/Linux - %1").arg(os);
#endif
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x050000
    int osVersion = int( QSysInfo::macVersion() ) - int ( QSysInfo::MV_10_0 );
#else
    int osVersion = int( QSysInfo::MacintoshVersion ) - int ( QSysInfo::MV_10_0 );
#endif
    host = QString("Mac OS 10.%1").arg(osVersion);
#endif
#ifdef Q_OS_WIN
    switch( QSysInfo::windowsVersion() )
    {
    case QSysInfo::WV_XP: host = "Windows XP"; break;
    case QSysInfo::WV_2003: host = "Windows 2003"; break;
    case QSysInfo::WV_VISTA: host = "Windows Vista"; break;
    case QSysInfo::WV_WINDOWS7: host = "Windows 7"; break;
#if QT_VERSION >= 0x050100
    case QSysInfo::WV_WINDOWS8: host = "Windows 8"; break;
#endif
    default: host = "Windows";
    }
#endif

    retVal = QString("%1 (%2)").arg(user).arg(host);
    return retVal;
}

void GCF::IpcServerDiscoveryData::updateAddresses()
{
    this->broadcastAddresses.clear();
    this->ipAddresses.clear();

    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress broadcastAddress = entry.broadcast();
            if (broadcastAddress != QHostAddress::Null)
            {
                this->broadcastAddresses << broadcastAddress;
                this->ipAddresses << entry.ip();
            }
        }
    }
}

bool GCF::IpcServerDiscoveryData::isMyAddress(const QHostAddress &address) const
{
    if(address == QHostAddress::LocalHost)
        return true;

    Q_FOREACH(QHostAddress localAddress, ipAddresses)
    {
        if(address == localAddress)
            return true;
    }

    return false;
}
