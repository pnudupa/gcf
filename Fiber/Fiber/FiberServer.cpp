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

#include "FiberServer.h"
#include "Json.h"
#include "FiberControlCentre.h"
#include "FiberCommon.h"
#include "FiberConfigurationSettings.h"

#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QThread>
#include <QElapsedTimer>
#include <QDateTime>
#include <QTimer>
#include <QPointer>

#include <GCF3/Log>

FiberServer::FiberServer(const QString &connectionString, QObject *parent) :
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

    qDebug() << "Fiber server listening on : " << this->fullServerName();
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Fiber listening on " + this->fullServerName());
}

FiberServer::~FiberServer()
{
    QLocalServer::removeServer(this->serverName());
}

void FiberServer::incomingConnection(quintptr socketDescriptor)
{
    FiberMainLocalSocket *socket = new FiberMainLocalSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    socket->setProperty("__SERVERNAME__", this->serverName());
}

///////////////////////////////////////////////////////////////////////////////
/// FiberMainLocalSocket
///
///////////////////////////////////////////////////////////////////////////////

FiberMainLocalSocket::FiberMainLocalSocket(QObject *parent) :
    QLocalSocket(parent)
{
    connect(this, SIGNAL(connected()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

void FiberMainLocalSocket::slotReadyRead()
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QByteArray content = this->readAll();

    if(content.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Empty data received as request"));
        return;
    }

    QVariantMap json = Json().parse(content).toMap();

    if(json.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Corrupt data received as request"));
        return;
    }

    this->processRequest(json);
}

void FiberMainLocalSocket::slotHandlerSocketUpdated(const QString &handlerName,
                                const QString &handlerSocket)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    if(m_handlerName == handlerName)
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Handler socket updated " + handlerSocket);
        QVariantMap jObj;
        jObj["handlerName"] = handlerSocket;
        QByteArray jsonData = Json().serialize(jObj);
        this->writeResponse(jsonData);
    }
}

void FiberMainLocalSocket::processRequest(const QVariantMap &requestObj)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString requestType = requestObj.value("requestType").toString();
    if(requestType.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid request format received as request"));
        return;
    }

    if(requestType == "HANDLER")
    {
        this->processHandlerRequest(requestObj);
        return;
    }
    else if(requestType == "FIBER_CONTROL")
    {
        this->processFiberControlRequest(requestObj);
        return;
    }
    else if(requestType == ("SESSION_MGMT"))
    {
        this->processSessionMgmtRequest(requestObj);
        return;
    }

    this->writeResponse(Fiber::ErrorStringToJSON("Unrecognized request"));
}

void FiberMainLocalSocket::processFiberControlRequest(const QVariantMap &requestObj)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString controlCommand = requestObj.value("controlCommand").toString();

    if(controlCommand.isEmpty())
        return;

    if(controlCommand == "terminate")
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Terminating fiber on request");

        FiberServer *server = qobject_cast<FiberServer*>(this->parent());
        if(server)
            server->close();

        this->close();
        this->thread()->quit();
    }
}

void FiberMainLocalSocket::processHandlerRequest(const QVariantMap &requestObj)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString sessionName = requestObj.value("sessionName", "NULL").toString();
    m_handlerName = FiberControlCentre::instance()->handlerForSession(sessionName);

    if(m_handlerName.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid session requested."));
        return;
    }

    QString handlerSocket = FiberControlCentre::instance()->localSocketForHandler(m_handlerName);

    if(!handlerSocket.isEmpty())
    {
        QVariantMap jObj;
        jObj["handlerName"] = handlerSocket;
        QByteArray jsonData = Json().serialize(jObj);
        this->writeResponse(jsonData);
    }
    else
    {
        connect(FiberControlCentre::instance(),
                SIGNAL(handlerSocketUpdated(QString,QString)),
                this,
                SLOT(slotHandlerSocketUpdated(QString,QString)));
    }
}

void FiberMainLocalSocket::processSessionMgmtRequest(const QVariantMap &requestObj)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString sessionCommand = requestObj.value("sessionCommand").toString();
    Json json;

    if(sessionCommand.toLower() == "create")
    {
        QString clientIP = requestObj.value("clientIP").toString();
        QString sessionName = FiberControlCentre::instance()->createSessionForIP(clientIP);

        QVariantMap jObj;
        jObj["success"] = true;
        jObj["result"] = sessionName;
        jObj["error"] = QString();
        QByteArray jsonData = json.serialize(jObj);
        this->writeResponse(jsonData);
        return;
    }
    else if(sessionCommand.toLower() == "terminate")
    {
        QString sessionName = requestObj.value("sessionName").toString();
        bool success = FiberControlCentre::instance()->terminateSession(sessionName);

        QVariantMap jObj;
        jObj["success"] = success;
        jObj["result"] = QString();
        jObj["error"] = QString();
        jObj["sessionName"] = sessionName;
        QByteArray jsonData = json.serialize(jObj);
        this->writeResponse(jsonData);
        return;
    }

    this->writeResponse(Fiber::ErrorStringToJSON("Unrecognized request"));
}

void FiberMainLocalSocket::writeResponse(const QByteArray &jsonData)
{
    this->write(jsonData);
    this->flush();
    this->waitForBytesWritten();
    this->close();
}
