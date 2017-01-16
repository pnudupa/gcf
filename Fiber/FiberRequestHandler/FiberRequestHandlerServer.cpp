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

#include "FiberRequestHandlerServer.h"
#include "Json.h"
#include "FiberControlCentre.h"
#include "FiberCommon.h"
#include "RequestManager.h"
#include "FiberConfigurationSettings.h"

#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QThread>
#include <QDateTime>
#include <QBasicTimer>
#include <QPointer>
#include <QtConcurrentRun>
#include <QLocalSocket>
#include <QEventLoop>

#include <GCF3/Application>
#include <GCF3/Log>

FiberRequestHandlerServer::FiberRequestHandlerServer(const QString &connectionString, QObject *parent) :
    QLocalServer(parent)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QLocalServer::removeServer(connectionString);
    this->listen(connectionString);
    RequestManager::instance()->setRequestHandlerName(connectionString);

#ifdef Q_OS_UNIX
    // This is done because the user that runs PHP wouldn't have the write permission on the socket
    // created by the server, unless Fiber is launched by the same user (which is unlikely)
    QProcess::execute(QString("chmod 0777 %1").arg(this->fullServerName()));
#endif

    QString message = "Fiber Handler server listening on : " + connectionString;
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, message);
    qDebug() << message;

    QVariantMap jObj;
    jObj["contactType"] = "HANDSHAKE";
    jObj["handlerName"] = this->serverName();
    jObj["handlerSocket"] = this->fullServerName();
    QByteArray jsonData = Json().serialize(jObj);

    QLocalSocket *handshakeSocket = new QLocalSocket(this);
    connect(handshakeSocket, SIGNAL(disconnected()), this, SLOT(slotFiberDisconnected()));
    handshakeSocket->connectToServer("FiberControl");
    handshakeSocket->waitForConnected();
    handshakeSocket->write(jsonData);
    handshakeSocket->flush();

    // To periodically check whether Surrogate is idle
    bool killOnTimeOut = gFiberSettings.value("AutoTimeOut/KillOnTimeOut", true).toBool();
    int refreshInterval = gFiberSettings.value("AutoTimeOut/RefreshInterval", 5000).toInt();

    if(killOnTimeOut)
        m_suicideTimer.start(refreshInterval, this);
}

FiberRequestHandlerServer::~FiberRequestHandlerServer()
{
    QLocalServer::removeServer(this->serverName());
}

void FiberRequestHandlerServer::slotFiberDisconnected()
{
    // This is designed to happen only in the event of a termination of the Fiber
    // or as an indication of a termination command from Fiber.
    // So assuming the Handler is not required any more, let's kill the Handler

    // TODO : 1) Log 2) Clean up ?
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Fiber disconnected; committing suicide");

    this->close();
    RequestManager::instance()->shutdown();
    this->thread()->quit();
}

void FiberRequestHandlerServer::timerEvent(QTimerEvent *)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);
    m_suicideTimer.stop();
    static bool alreadyInIdleState = false;

    int timeoutInterval = gFiberSettings.value("AutoTimeOut/TimeOutInterval", 10000).toInt();
    int refreshInterval = gFiberSettings.value("AutoTimeOut/RefreshInterval", 5000).toInt();

    if(!RequestManager::instance()->isIdle())
    {
        alreadyInIdleState = false;
        m_suicideTimer.start(refreshInterval, this);
        return;
    }

    if(RequestManager::instance()->isIdle() &&
       !alreadyInIdleState )
    {
        alreadyInIdleState = true;
        m_suicideTimer.start(timeoutInterval, this);
        return;
    }

    QString message = QString("Terminating because idle-timeout[%1] encountered.").arg(timeoutInterval);
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, message);

    // Write Suicide letter to Fiber control
    QVariantMap jObj;
    jObj["contactType"] = "SUICIDE";
    jObj["handlerName"] = this->serverName();
    jObj["handlerSocket"] = this->fullServerName();
    QByteArray jsonData = Json().serialize(jObj);

    QLocalSocket *handshakeSocket = new QLocalSocket(this);
    connect(handshakeSocket, SIGNAL(disconnected()), handshakeSocket, SLOT(deleteLater()));
    handshakeSocket->connectToServer("FiberControl");
    handshakeSocket->waitForConnected();
    handshakeSocket->write(jsonData);
    handshakeSocket->flush();
}

void FiberRequestHandlerServer::incomingConnection(quintptr socketDescriptor)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QString message = QString("Fiber Surrogate server received a new connection : %1").arg(socketDescriptor);
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, message);

    QtConcurrent::run(this, &FiberRequestHandlerServer::createLocalSocket, socketDescriptor);
}

void FiberRequestHandlerServer::createLocalSocket(quintptr socketDescriptor)
{
    // qApp->processEvents() operates on the current thread only
    // And for some reason it is required to be done here!
    qApp->processEvents();

    FiberRequestHandlerLocalSocket *socket = new FiberRequestHandlerLocalSocket;
    socket->setProperty("__SERVERNAME__", this->serverName());
    socket->setSocketDescriptor(socketDescriptor);

    QEventLoop eventLoop;
    connect(socket, SIGNAL(done()), &eventLoop, SLOT(quit()));
    connect(socket, SIGNAL(destroyed()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

///////////////////////////////////////////////////////////////////////////////
/// FiberRequestHandlerLocalSocket
///
///////////////////////////////////////////////////////////////////////////////

FiberRequestHandlerLocalSocket::FiberRequestHandlerLocalSocket(QObject *parent) :
    QLocalSocket(parent)
{
    m_requestID = 0;

    connect(RequestManager::instance(),
            SIGNAL(requestManagerShutdown()),
            this,
            SLOT(deleteLater()),
            Qt::QueuedConnection);
    connect(RequestManager::instance(),
            SIGNAL(requestManagerShutdown()),
            this,
            SIGNAL(done()),
            Qt::QueuedConnection);

    connect(this, SIGNAL(connected()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

void FiberRequestHandlerLocalSocket::slotReadyRead()
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
        this->writeResponse(Fiber::ErrorStringToJSON("Corrupt data received as request at handler"));
        return;
    }

    m_requestData += Fiber::FiberMessageFooter;
    this->processRequest(json);
}

void FiberRequestHandlerLocalSocket::slotDisconnected()
{
    // We need to cancel the request if the Peer disconnects
    // after posting a request
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    // We "shouldn't" do this for a non-blocking call, because,
    // that would delete the request that was just posted
    QVariant type = this->property("__SESSIONTYPE__");

    if(!type.isValid())
    {
        this->deleteLater();
        return;
    }

    // We need to do this only if we are expecting result for a request
    if(!this->property("__DONE__").isValid())
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Had to cancel a request because peer disconnected");

        if(type.toString() == "CORE") // Was handling a core request
            RequestManager::instance()->cancelCoreServiceRequest(m_requestID);
        else if(type.toString() == "GUI")
            RequestManager::instance()->cancelGuiServiceRequest(m_requestID);
    }

    this->deleteLater();
}

void FiberRequestHandlerLocalSocket::slotCoreServiceResponseAvailable(int requestID)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    if(requestID != m_requestID)
        return;

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Writing core service response");

    QByteArray data = RequestManager::instance()->coreServiceResponse(m_requestID);
    this->writeResponse(data);
}

void FiberRequestHandlerLocalSocket::slotGuiServiceResponseAvailable(int requestID)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    if(requestID != m_requestID)
        return;

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Writing gui service response");

    QByteArray data = RequestManager::instance()->guiServiceResponse(m_requestID);
    this->writeResponse(data);
}

void FiberRequestHandlerLocalSocket::processRequest(const QVariantMap &jsonRequest)
{
    QString sessionType = jsonRequest.value("sessionType", "CORE").toString();
    bool blockingCall = jsonRequest.value("blockingCall", true).toBool();
    bool metaCall = jsonRequest.value("metaCall", false).toBool();

    // Meta calls are special requests that deal with existing non-blocking
    // requests.
    if(metaCall)
    {
        this->processMetaCall(jsonRequest);
        return;
    }

    if(sessionType == "CORE")
    {
        m_requestID = RequestManager::instance()->coreServiceRequest(m_requestData);

        if(blockingCall)
            connect(RequestManager::instance(), SIGNAL(coreServiceResponseAvailable(int)),
                this, SLOT(slotCoreServiceResponseAvailable(int)));
    }
    else if(sessionType == "GUI")
    {
        m_requestID = RequestManager::instance()->guiServiceRequest(m_requestData);

        if(blockingCall)
            connect(RequestManager::instance(), SIGNAL(guiServiceResponseAvailable(int)),
                this, SLOT(slotGuiServiceResponseAvailable(int)));
    }
    else
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid session type"));
        return;
    }

    // Only if it is not a blocking call we need to return something immediately
    // i.e the 'requestID'. In a normal call the actual result will be written back
    // after a while and hence we don't do anything here (other than wait for signal)
    if(!blockingCall)
    {
        QVariantMap response;
        response["success"] = true;
        response["result"] = m_requestID;
        response["error"] = QString();
        QByteArray responseMsg = Json().serialize(response);
        this->writeResponse(responseMsg);
        return;
    }

    this->setProperty("__SESSIONTYPE__", sessionType);
}

void FiberRequestHandlerLocalSocket::processMetaCall(const QVariantMap &jsonRequest)
{
    QString method = jsonRequest.value("metaMethod").toString();

    if(method.isEmpty())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid meta method requested"));
        return;
    }

    // We support the following methods
    // availableResponses   : List of responses available for download
    // response             : Response for a given requestID
    // responseSize         : Size of response available for a given requestID
    // cancelRequests       : Cancel the requests for the list of requestIDs provided

    if(method == "availableResponses")
    {
        this->processAvailableResponsesCall(jsonRequest);
        return;
    }
    else if(method == "response")
    {
        this->processResponseCall(jsonRequest);
        return;
    }
    else if(method == "responseSize")
    {
        this->processResponseSizeCall(jsonRequest);
        return;
    }
    else if(method == "cancelRequests")
    {
        this->processCancelRequestsCall(jsonRequest);
        return;
    }

    this->writeResponse(Fiber::ErrorStringToJSON("Invalid meta method requested"));
}

void FiberRequestHandlerLocalSocket::processAvailableResponsesCall(const QVariantMap &jsonRequest)
{
    QString sessionType = jsonRequest.value("sessionType", "CORE").toString();

    QList<int> availableResponses;

    if(sessionType == "CORE")
    {
        availableResponses = RequestManager::instance()->pendingCoreServiceResponses();
    }
    else if(sessionType == "GUI")
    {
        availableResponses = RequestManager::instance()->pendingGuiServiceResponses();
    }
    else
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid session type"));
        return;
    }

    QVariantList retList;

    if(availableResponses.count())
        Q_FOREACH(int val, availableResponses)
            retList << val;

    QVariantMap response;
    response["success"] = true;
    response["result"] = retList;
    response["error"] = QString();

    QByteArray jsonData = Json().serialize(response);
    this->writeResponse(jsonData);
}

void FiberRequestHandlerLocalSocket::processResponseCall(const QVariantMap &jsonRequest)
{
    QString sessionType = jsonRequest.value("sessionType", "CORE").toString();
    int requestID = jsonRequest.value("requestID", -1).toInt();
    int responseLength = jsonRequest.value("responseLength", -1).toInt();

    if(requestID < 0)
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid request ID"));
        return;
    }

    QByteArray response;

    if(sessionType == "CORE")
    {
        if(!RequestManager::instance()->pendingCoreServiceResponses().contains(requestID))
        {
            this->writeResponse(Fiber::ErrorStringToJSON(QString("No response found for %1").arg(requestID)));
            return;
        }
        response = RequestManager::instance()->coreServiceResponse(requestID, responseLength);
    }
    else if(sessionType == "GUI")
    {
        if(!RequestManager::instance()->pendingGuiServiceResponses().contains(requestID))
        {
            this->writeResponse(Fiber::ErrorStringToJSON(QString("No response found for %1").arg(requestID)));
            return;
        }
        response = RequestManager::instance()->guiServiceResponse(requestID, responseLength);
    }
    else
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid session type"));
        return;
    }

    this->writeResponse(response);
}

void FiberRequestHandlerLocalSocket::processResponseSizeCall(const QVariantMap &jsonRequest)
{
    QString sessionType = jsonRequest.value("sessionType", "CORE").toString();
    int requestID = jsonRequest.value("requestID", -1).toInt();

    if(requestID < 0)
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid request ID"));
        return;
    }

    if(!RequestManager::instance()->pendingCoreServiceResponses().contains(requestID) &&
       !RequestManager::instance()->pendingGuiServiceResponses().contains(requestID) )
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid request ID"));
        return;
    }

    int availableSize = -1;

    if(sessionType == "CORE")
    {
        availableSize = RequestManager::instance()->coreServiceResponseSize(requestID);
    }
    else if(sessionType == "GUI")
    {
        availableSize = RequestManager::instance()->guiServiceResponseSize(requestID);
    }
    else
    {
        this->writeResponse(Fiber::ErrorStringToJSON("Invalid session type"));
        return;
    }

    QVariantMap response;
    response["success"] = true;
    response["result"] = availableSize;
    response["error"] = QString();

    QByteArray jsonData = Json().serialize(response);
    this->writeResponse(jsonData);
}

void FiberRequestHandlerLocalSocket::processCancelRequestsCall(const QVariantMap &jsonRequest)
{
    QString sessionType = jsonRequest.value("sessionType", "CORE").toString();
    QVariantList requestIDs = jsonRequest.value("requestIDs").toList();

    if(!requestIDs.count())
    {
        this->writeResponse(Fiber::ErrorStringToJSON("No request ID provided"));
        return;
    }

    QVariantList cancelledIDs;

    Q_FOREACH(const QVariant &requestID, requestIDs)
    {
        if(!requestID.isValid() || requestID.isNull())
            continue;

        bool success = false;

        if(sessionType == "CORE")
        {
            success = RequestManager::instance()->cancelCoreServiceRequest(requestID.toInt());
        }
        else if(sessionType == "GUI")
        {
            success = RequestManager::instance()->cancelGuiServiceRequest(requestID.toInt());
        }
        else
        {
            this->writeResponse(Fiber::ErrorStringToJSON("Invalid session type"));
            return;
        }

        if(success)
            cancelledIDs << requestID;
    }

    QVariantMap response;
    response["success"] = true;
    response["result"] = cancelledIDs;
    response["error"] = QString();

    QByteArray jsonData = Json().serialize(response);
    this->writeResponse(jsonData);
}

void FiberRequestHandlerLocalSocket::writeResponse(const QByteArray &jsonData)
{
    this->write(jsonData);
    this->flush();
    this->waitForBytesWritten();
    this->close();
    this->setProperty("__DONE__", true);
    emit done();
    qApp->processEvents();
}

