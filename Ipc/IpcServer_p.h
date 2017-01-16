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

#ifndef IPCSERVER_P_H
#define IPCSERVER_P_H

#include <QMetaObject>
#include <QMetaMethod>

#include "../Core/SignalSpy.h"
#include "IpcCommon_p.h"

namespace GCF
{

class IpcRemoteObjectHandler : public QObject
{
    Q_OBJECT

public:
    IpcRemoteObjectHandler(const GCF::IpcMessage &request,
                           IpcSocket *socket,
                           QObject *parent=0);
    ~IpcRemoteObjectHandler();

    GCF::IpcSocket *socket() const { return m_socket; }
    QObject *object() const { return m_object; }

private slots:
    void onIncomingMessage(const GCF::IpcMessage &message);

private:
    GCF::IpcSocket *m_socket;
    QObject *m_object;
};

class IpcRemoteSignalDespatch : public GCF::SignalSpy
{
public:
    IpcRemoteSignalDespatch(const char *signal, IpcRemoteObjectHandler *parent = 0)
        : GCF::SignalSpy(parent->object(), signal, parent), m_remoteObjectHandler(parent) { }
    ~IpcRemoteSignalDespatch() { }

protected:
    void handleSignalEmission(const QVariantList &args) {
        GCF::IpcMessage message(GCF::IpcMessage::SIGNAL_DELIVERY);
        message.data()["signal"] = QString::fromLatin1(this->signal());
        message.data()["arguments"] = args;
        message.setResult(true);
        m_remoteObjectHandler->socket()->sendMessage(message);
    }

private:
    IpcRemoteObjectHandler *m_remoteObjectHandler;
};

}

#endif // IPCSERVER_P_H
