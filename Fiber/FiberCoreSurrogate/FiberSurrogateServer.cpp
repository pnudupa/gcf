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

#include "FiberSurrogateServer.h"
#include "Json.h"
#include "FiberCommon.h"

#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QDateTime>
#include <QPointer>
#include <QLocalSocket>

#include <GCF3/Application>
#include <GCF3/Log>

FiberSurrogateServer::FiberSurrogateServer(const QString &connectionString,
                                     const QString &reportingHandler,
                                     QObject *parent) :
    QLocalServer(parent)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QLocalServer::removeServer(connectionString);
    this->listen(connectionString);

#ifdef Q_OS_UNIX
    // This is done because the user that runs PHP wouldn't have the write permission on the socket
    // created by the server, unless Fiber is launched by the same user (which is unlikely)
    QProcess::execute(QString("chmod 0777 %1").arg(this->fullServerName()));
#endif

    QString message = "Surrogate server listening on : " + connectionString;
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, message);
    qDebug() << message;

    QVariantMap jObj;
    jObj["contactType"] = "HANDSHAKE";
    jObj["surrogateName"] = this->serverName();
    jObj["surrogateSocket"] = this->fullServerName();
    QByteArray jsonData = Json().serialize(jObj);

    QLocalSocket *handshakeSocket = new QLocalSocket(this);
    connect(handshakeSocket, SIGNAL(disconnected()), this, SLOT(slotHandlerDisconnected()));
    handshakeSocket->connectToServer(reportingHandler);
    handshakeSocket->waitForConnected();
    handshakeSocket->write(jsonData);
    handshakeSocket->flush();
}

FiberSurrogateServer::~FiberSurrogateServer()
{
    QLocalServer::removeServer(this->serverName());
}

void FiberSurrogateServer::slotHandlerDisconnected()
{
    // This is designed to happen only in the event of a termination of the Surrogate
    // or as an indication of a termination command from Surrogate.
    // So assuming the Handler is not required any more, let's kill the Surrogate

    // TODO : Clean up ?
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Handler disconnected; committing suicide");

    this->close();
    this->deleteLater();
}

void FiberSurrogateServer::incomingConnection(quintptr socketDescriptor)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString message = QString("Fiber Surrogate server received a new connection : %1").arg(socketDescriptor);
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, message);

    this->createLocalSocket(socketDescriptor);
}

void FiberSurrogateServer::createLocalSocket(quintptr socketDescriptor)
{
    FiberSurrogateLocalSocket *socket = new FiberSurrogateLocalSocket(this);
    socket->setProperty("__SERVERNAME__", this->serverName());
    socket->setSocketDescriptor(socketDescriptor);
}

///////////////////////////////////////////////////////////////////////////////
/// FiberSurrogateLocalSocket
///
///////////////////////////////////////////////////////////////////////////////

FiberSurrogateLocalSocket::FiberSurrogateLocalSocket(QObject *parent) :
    QLocalSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

void FiberSurrogateLocalSocket::slotReadyRead()
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QByteArray content = this->readAll();

    if(content.isEmpty())
        return;

    m_requestData += content;

    if(!content.endsWith(Fiber::FiberMessageFooter))
        return;

    m_requestData.replace(Fiber::FiberMessageFooter, "");

    QVariantMap json = Json().parse(m_requestData).toMap();

    if(json.isEmpty())
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, m_requestData);
        this->writeResponse(Fiber::ErrorStringToJSON("Corrupt data received as request"));
        return;
    }

    this->handleRequest(json);
}

void FiberSurrogateLocalSocket::handleRequest(const QVariantMap &jsonRequest)
{
    QString library = jsonRequest.value("serviceLibrary").toString();
    QString component = jsonRequest.value("serviceComponent").toString();
    QString object = jsonRequest.value("serviceObject").toString();
    QString method = jsonRequest.value("serviceMethod").toString();
    QVariantList args = jsonRequest.value("args").toList();

    if(component.isEmpty() || object.isEmpty() || method.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Insufficient data for method invocation"));
        return;
    }

    if(library.isEmpty())
        library = component;

    QString objectPath = "Application." + component + "." + object;
    QObject *obj = gFindObject(objectPath);

    if(!obj)
    {
        GCF::Component *comp = gAppService->loadComponent(library);
        if(!comp)
        {
            this->writeResponse(Fiber::ErrorStringToJSON("Couldn't load service library " + library));
            return;
        }
    }

    obj = gFindObject(objectPath);

    if(!obj)
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Couldn't find the specified service object " + objectPath));
        return;
    }

    GCF::Result result = gAppService->invokeMethod(obj, method, args);

    QVariantMap jObj;
    jObj["success"] = result.isSuccess();
    jObj["result"] = result.data();
    jObj["error"] = result.message();
    jObj["server"] = this->property("__SERVERNAME__").toString();
    QByteArray jsonData = Json().serialize(jObj);
    this->writeResponse(jsonData);
    return;
}

void FiberSurrogateLocalSocket::writeResponse(const QByteArray &jsonData)
{
    this->write(jsonData + Fiber::FiberMessageFooter);
    this->flush();
    this->waitForBytesWritten();
    this->close();
    qApp->processEvents();
}
