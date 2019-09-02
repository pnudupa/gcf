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

#include "IpcServer.h"
#include "IpcServer_p.h"
#include "IpcCommon_p.h"
#include "IpcCommon_p.h"

#include "../Core/Application.h"
#include "../Core/Log.h"

#include <QUuid>
#include <QVector>

/**
\class GCF::IpcServer IpcServer.h <GCF3/IpcServer>
\brief Provides a IPC server based on \c QTcpServer
\ingroup gcf_ipc

Create an instance of this class and call \c listen() to begin listening for
incoming requests and responding to them.
*/

Q_GLOBAL_STATIC( QList<GCF::IpcServer*>, IpcServerList )

/**
\return a list of all server instances created in this application
*/
QList<GCF::IpcServer*> GCF::IpcServer::servers()
{
    return *(::IpcServerList());
}

/**
Creates an instance of this class with parent object \c parent
 */
GCF::IpcServer::IpcServer(QObject *parent)
    :QTcpServer(parent)
{
    ::IpcServerList()->append(this);
}

/**
Destructor
 */
GCF::IpcServer::~IpcServer()
{
    ::IpcServerList()->removeAll(this);
}

QString GCF::IpcServer::serverId() const
{
    if(m_serverId.isEmpty())
        m_serverId = QUuid::createUuid().toString();

    return m_serverId;
}

#if QT_VERSION >= 0x050000
void GCF::IpcServer::incomingConnection(qintptr handle)
#else
void GCF::IpcServer::incomingConnection(int handle)
#endif
{
    GCF::IpcSocket *socket = new GCF::IpcSocket(this);
    socket->setSocketDescriptor(handle);

    // We dont want to add this socket to a list of pending-connections
    // We simpy want to deal with the socket connection ourselves and
    // close the socket connection.
    //
    // So we are not going to call this->addPendingConnection(socket);

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("Incoming IPC connection from %1:%2")
                               .arg(socket->peerAddress().toString())
                               .arg(socket->peerPort()));

    connect(socket, SIGNAL(incomingMessage(GCF::IpcMessage)),
            this, SLOT(onIncomingMessage(GCF::IpcMessage)));
}

void GCF::IpcServer::onIncomingMessage(const GCF::IpcMessage &message)
{
    GCF::LogMessageBranch branch(QString("Processing message-id %1 of type %2")
                                 .arg(message.id())
                                 .arg(QString::fromLatin1(message.type())));

    GCF::IpcSocket *socket = qobject_cast<GCF::IpcSocket*>(this->sender());
    if(!socket)
    {
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                      "Unable to determine the sender socket!");
        return;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Message came from %1:%2")
                                  .arg(socket->peerAddress().toString())
                                  .arg(socket->peerPort()));

    if(message.type() == GCF::IpcMessage::IPC_CALL)
    {
        QString object = message.data().value("object").toString();
        QString method = message.data().value("method").toString();
        QVariantList args = message.data().value("arguments").toList();
        GCF::LogMessageBranch branch( QString("Processing call %1::%2(%3 args)")
                                      .arg(object).arg(method).arg(args.count()));

        GCF::Result result = gAppService->invokeMethod(object, method, args);
        connect(socket, SIGNAL(writeBufferEmpty()), socket, SLOT(deleteLater()));

        GCF::IpcMessage response(message.id(), message.type());
        response.setResult(result);
        socket->sendMessage(response);
    }
    else if(message.type() == GCF::IpcMessage::REQUEST_OBJECT)
    {
        QString object = message.data().value("object").toString();
        GCF::LogMessageBranch branch( QString("Process remote object request for '%1'").arg(object) );

        GCF::IpcRemoteObjectHandler *remoteObjectHandler =
                new GCF::IpcRemoteObjectHandler(message, socket, this);
        disconnect(socket, nullptr, this, nullptr);
        Q_UNUSED(remoteObjectHandler);
    }
}

///////////////////////////////////////////////////////////////////////////////

QVariantMap objectProperties(QObject *object)
{
    QVariantMap retMap;
    const QMetaObject *mo = object->metaObject();
    for(int i=0; i<mo->propertyCount(); i++)
    {
        QMetaProperty prop = mo->property(i);
        retMap[prop.name()] = prop.read(object);
    }

    return retMap;
}

QStringList objectMembers(QObject *object, bool signalsOnly=false)
{
    const QMetaObject *mo = object->metaObject();
    QStringList retList;
    for(int i=0; i<mo->methodCount(); i++)
    {
        QMetaMethod method = mo->method(i);
        if(signalsOnly && method.methodType() != QMetaMethod::Signal)
            continue;

        GCF::Result result = gAppService->isMethodInvokable(method, object);
        if(result.isSuccess() == false)
            continue;

#if QT_VERSION >= 0x050000
        QString methodSignature = QString::fromLatin1(method.methodSignature());
#else
        QString methodSignature = QString::fromLatin1(method.signature());
#endif

        retList.append(methodSignature);
    }

    return retList;
}

GCF::IpcRemoteObjectHandler::IpcRemoteObjectHandler(const IpcMessage &request,
                       IpcSocket *socket,
                       QObject *parent)
    : QObject(parent), m_socket(socket)
{
    m_socket->setParent(this);

    QString objectPath = request.data().value("object").toString();
    m_object = gAppService->objectTree()->object(objectPath);

    GCF::IpcMessage response(request.id(), request.type());
    if(m_object)
    {
        GCF::ObjectTreeNode *node = gAppService->objectTree()->node(m_object);
        if(node->info().value("allowmetaaccess", false).toBool())
        {
            response.setResult( GCF::Result(true) );
            response.data()["properties"] = ::objectProperties(m_object);
            response.data()["signals"] = ::objectMembers(m_object, true);
            response.data()["members"] = ::objectMembers(m_object);
        }
        else
        {
            GCF::Result result(false, QString(), QString("Meta access to object '%1' not enabled").arg(objectPath));
            response.setResult(result);
        }
    }
    else
    {
        QString msg = QString("Object '%1' could not be found").arg(objectPath);
        response.setResult( GCF::Result(false, QString(), msg) );
    }

    if(response.result().isSuccess())
    {
        connect(m_socket, SIGNAL(incomingMessage(GCF::IpcMessage)),
                this, SLOT(onIncomingMessage(GCF::IpcMessage)));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
        connect(m_object, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    }
    else
        connect(m_socket, SIGNAL(writeBufferEmpty()), this, SLOT(deleteLater()));

    m_socket->sendMessage(response);
}

GCF::IpcRemoteObjectHandler::~IpcRemoteObjectHandler()
{

}

void GCF::IpcRemoteObjectHandler::onIncomingMessage(const GCF::IpcMessage &message)
{
    GCF::IpcMessage response(message.id(), message.type());

    if( message.type() == GCF::IpcMessage::GET_PROPERTY_VALUE )
    {
        QString propertyName = message.data().value("propertyName").toString();
        QVariant propertyValue = m_object->property( qPrintable(propertyName) );

        response.setResult( GCF::Result(true, QString(), QString(), propertyValue) );
        response.data()["propertyName"] = propertyName;
    }
    else if( message.type() == GCF::IpcMessage::SET_PROPERTY_VALUE )
    {
        QString propertyName = message.data().value("propertyName").toString();
        QVariant propertyValue = message.data().value("propertyValue");
        bool success = m_object->setProperty(qPrintable(propertyName), propertyValue);

        response.setResult( GCF::Result(success, QString(), QString(), propertyValue) );
        response.data()["propertyName"] = propertyName;
    }
    else if( message.type() == GCF::IpcMessage::REQUEST_CONNECTION )
    {
        QByteArray signal = message.data().value("signal").toByteArray();
        signal.prepend('2');
        GCF::IpcRemoteSignalDespatch *signalDespatch =
                new GCF::IpcRemoteSignalDespatch(signal, this);
        if(!signalDespatch || !signalDespatch->isValid())
        {
            delete signalDespatch;
            signalDespatch = nullptr;
        }

        response.data()["signal"] = signal;
        response.setResult( signalDespatch != nullptr );
    }

    m_socket->sendMessage(response);
}

