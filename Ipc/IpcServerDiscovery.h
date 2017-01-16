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

#ifndef IPCSERVERDISCOVERY_H
#define IPCSERVERDISCOVERY_H

#include "IpcCommon.h"
#include <QObject>
#include <QHostAddress>

namespace GCF
{

class IpcServer;

struct IpcServerInfo
{
    IpcServerInfo() : Port(0) { }
    IpcServerInfo(const QString &user, const QHostAddress &addr, quint16 port)
        : User(user), Address(addr), Port(port) { }
    IpcServerInfo(const IpcServerInfo &other)
        : User(other.User), Address(other.Address), Port(other.Port) { }

    QString User;
    QHostAddress Address;
    quint16 Port;

    bool isValid() const { return !this->User.isEmpty() && !this->Address.isNull() && this->Port > 0; }
    bool operator == (const IpcServerInfo &other) const {
        return User == other.User && Address == other.Address && Port == other.Port;
    }
};

struct IpcServerDiscoveryData;
class GCF_IPC_EXPORT IpcServerDiscovery : public QObject
{
    Q_OBJECT

public:
    IpcServerDiscovery(QObject *parent=0);
    ~IpcServerDiscovery();

    static QString user();

    GCF::Result start(quint16 port=0);
    GCF::Result stop();

    QList<GCF::IpcServerInfo> foundServers() const;
    void releaseFoundServer(const GCF::IpcServerInfo &info);
    void releaseAllFoundServers();

signals:
    void foundServer(const GCF::IpcServerInfo &info);

private slots:
    void sendBroadcastDatagram();
    void readBroadcastDatagram();

protected:
    void timerEvent(QTimerEvent *);

private:
    IpcServerDiscoveryData *d;
};

}

inline QDebug operator << (QDebug dbg, const GCF::IpcServerInfo &info)
{
    dbg.nospace() << "GCF::IpcServerInfo(" << info.User << ", "
                  << info.Address << ", " << info.Port << ")";
    return dbg.space();
}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::IpcServerInfo)

#endif // IPCSERVERDISCOVERY_H
