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

#ifndef IPCREMOTEOBJECT_H
#define IPCREMOTEOBJECT_H

#include "IpcCommon.h"

#include <QObject>
#include <QHostAddress>
#include <QByteArray>

namespace GCF
{

class IpcMessage;
class IpcRemoteObjectCall;

struct IpcRemoteObjectData;
class GCF_IPC_EXPORT IpcRemoteObject : public QObject
{
    Q_OBJECT

public:
    IpcRemoteObject(const QHostAddress &addr, quint16 port,
                    const QString &object, QObject *parent=0);
    ~IpcRemoteObject();

    bool isActivated() const;
    void reactivate();

    QHostAddress remoteAddress() const;
    quint16 remotePort() const;
    QString remoteObjectPath() const;

    QVariantMap properties() const;
    QStringList signalMethods() const;
    QStringList invokableMethods() const;

    GCF::Result updateProperty(const QString &propertyName,
                        QObject *receiver=0, const char *member=0);
    GCF::Result changeProperty(const QString &propertyName, const QVariant &propertyValue,
                        QObject *receiver=0, const char *member=0);
    GCF::Result ipcConnect(const char *signal, QObject *receiver=0, const char *member=0);
    GCF::Result ipcConnect(QObject *sender, const char *signal=0, const char *member=0);
    GCF::Result call(const QString &method, const QVariantList &args,
              QObject *receiver=0, const char *member=0);

signals:
    void activated();
    void deactivated();
    void requestFinished(int requestId);
    void signalOccurance(const QString &signal, const QVariantList &args);
    void propertyUpdated(const QString &propertyName, const QVariant &value);
    void couldNotActivate(const QString &error);
    void error(const QString &error);

private slots:
    void onSetup();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketConnectError();
    void onSocketIncomingMessage(const GCF::IpcMessage &message);

private:
    void queueMessage(const GCF::IpcMessage &message);
    void sendNextMessage();

    void emitActivated();
    void emitDeactivated();
    void emitRequestFinished(int requestId);
    void emitSignalOccurance(const QString &signal, const QVariantList &args);
    void emitPropertyUpdated(const QString &name, const QVariant &value);
    void emitCouldNotActivate(const QString &error);
    void emitError(const QString &error);

private:
    IpcRemoteObjectData *d;
};

inline GCF::Result ipcConnect(QObject *sender, const char *signal, GCF::IpcRemoteObject *receiver, const char *member)
{
    if(!receiver)
        return GCF::Result(false, QString(), QString("Receiver not specified"));
    return receiver->ipcConnect(sender, signal, member);
}

inline GCF::Result ipcConnect(GCF::IpcRemoteObject *sender, const char *signal, QObject *receiver, const char *member)
{
    if(!sender)
        return GCF::Result(false, QString(), QString("Sender not specified"));
    return sender->ipcConnect(signal, receiver, member);
}

GCF::Result GCF_IPC_EXPORT ipcConnect(QObject *sender, const char *signal, const QHostAddress &addr,
                quint16 port, const QString &object, const QString &method);

}

#endif // IPCREMOTEOBJECT_H
