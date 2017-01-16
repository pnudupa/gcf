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

#include "FiberControlCentre.h"
#include "Json.h"

#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QProcess>
#include <QThread>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUuid>

#include <GCF3/Log>
#include <GCF3/ObjectMap>
#include <GCF3/MapToObject>

class FiberControlCentre2 : public FiberControlCentre
{
public:
    FiberControlCentre2() { }
    ~FiberControlCentre2() { }
};

Q_GLOBAL_STATIC(FiberControlCentre2, GlobalFiberControlCentre)
FiberControlCentre *FiberControlCentre::instance()
{
    return GlobalFiberControlCentre();
}

Q_GLOBAL_STATIC(QStringList, ExistingRequestHandlerNames)

struct FiberControlCentreData
{
    QMutex fiberControlCentreMutex;
    QLocalServer fiberControlServer;

    // QMap< QString, QPointer<RequestHandlerAgent> > sessionHandlerAgentMap;
    GCF::MapToObject< QString > sessionHandlerAgentMap;
    QMap< QString, QString > sessionIPMap;

    QPointer<RequestHandlerAgent> handler(const QString &handlerName) const {
        if(handlerName.isEmpty())
            return 0;
        QObjectList handlers = sessionHandlerAgentMap.values();
        if(!handlers.count())
            return 0;
        Q_FOREACH(QObject *handlerObject, handlers) {
            RequestHandlerAgent *handler = qobject_cast<RequestHandlerAgent*>(handlerObject);
            if(handler && (handler->m_handlerName == handlerName))
                return handler;
        }
        return 0;
    }
};

FiberControlCentre::FiberControlCentre(QObject *parent) :
    QObject(parent)
{
    d = new FiberControlCentreData;

    // 'FiberControl' is the server through which individual request handlers communicate
    // with FiberControlCentre for Handshake, TerminateRequest etc
    QLocalServer::removeServer("FiberControl");
    d->fiberControlServer.listen("FiberControl");

    connect(&d->fiberControlServer, SIGNAL(newConnection()),
            this, SLOT(fiberControlServerNewConnection()));
}

FiberControlCentre::~FiberControlCentre()
{
    d->fiberControlServer.close();
    QLocalServer::removeServer(d->fiberControlServer.serverName());
    delete d;
    d = 0;
}

QString FiberControlCentre::createSessionForIP(const QString &clientIP)
{
    QMutexLocker locker(&d->fiberControlCentreMutex);

    QString uuid = QUuid::createUuid().toString();
    d->sessionIPMap[uuid] = clientIP;
    return uuid;
}

bool FiberControlCentre::terminateSession(const QString &sessionName)
{
    QMutexLocker locker(&d->fiberControlCentreMutex);

    if(sessionName.isEmpty() ||
       d->sessionIPMap.isEmpty() ||
       !d->sessionIPMap.contains(sessionName))
        return false;

    d->sessionIPMap.remove(sessionName);
    RequestHandlerAgent *handler =
            qobject_cast<RequestHandlerAgent*>(d->sessionHandlerAgentMap.value(sessionName));

    if(handler)
    {
        if(handler->m_handledSessions.count())
            handler->m_handledSessions.removeAll(sessionName);

        if(!handler->m_handledSessions.count())
            handler->deactivate();
    }

    d->sessionHandlerAgentMap.remove(sessionName);

    return true;
}

QString FiberControlCentre::handlerForSession(const QString &sessionName)
{
    QMutexLocker locker(&d->fiberControlCentreMutex);

    if(sessionName.isEmpty())
        return QString();

    if( (sessionName != "NULL") &&
        (!d->sessionIPMap.contains(sessionName)) )
        return QString();

    RequestHandlerAgent *handler =
            qobject_cast<RequestHandlerAgent*>(d->sessionHandlerAgentMap.value(sessionName));

    if(handler && !handler->m_deactivating)
        return handler->m_handlerName;

    handler = new RequestHandlerAgent();
    connect(handler, SIGNAL(deactivated()),
            this, SLOT(handlerDeactivated()),
            Qt::QueuedConnection);

    handler->m_handledSessions << sessionName;
    d->sessionHandlerAgentMap.insert(sessionName, handler);
    handler->activate();

    return handler->m_handlerName;
}

QString FiberControlCentre::localSocketForHandler(const QString &handlerName) const
{
    QPointer<RequestHandlerAgent> handler = d->handler(handlerName);

    if(handler)
        return handler->m_handlerSocket;

    return QString();
}

void FiberControlCentre::fiberControlServerNewConnection()
{
    QLocalSocket *localSocket = d->fiberControlServer.nextPendingConnection();
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(fiberControlServerReadyRead()));
}

void FiberControlCentre::fiberControlServerReadyRead()
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    QLocalSocket *localSocket = qobject_cast<QLocalSocket*>(sender());
    QByteArray data = localSocket->readAll();

    if(data.isEmpty())
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Empty handshake received");
        qDebug() << "Empty handshake received";
        localSocket->deleteLater();
        return;
    }

    QVariantMap json = Json().parse(data).toMap();

    if(json.isEmpty())
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Incorrect JSON format received");
        qDebug() << "Incorrect JSON format in handshake";
        localSocket->deleteLater();
        return;
    }

    QString handlerName = json.value("handlerName").toString();
    QString handlerSocket = json.value("handlerSocket").toString();
    QString contactType = json.value("contactType").toString();

    if(handlerName.isEmpty() ||
       handlerSocket.isEmpty() ||
       contactType.isEmpty() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Incomplete data received at handshake");
        qDebug() << "Incomplete data received at handshake";
        localSocket->deleteLater();
        return;
    }

    if(contactType == "HANDSHAKE")
    {
        QPointer<RequestHandlerAgent> handler = d->handler(handlerName);

        if(!handler)
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Invalid handler name received at handshake " + handlerName);
            qDebug() << "Invalid handler name received at handshake " + handlerName;
            localSocket->deleteLater();
            return;
        }

        // We let the connection to be alive so that the handler can know
        // whether the server is alive or not.
        connect(localSocket, SIGNAL(disconnected()), localSocket, SLOT(deleteLater()));

        handler->m_handlerSocket = handlerSocket;
        handler->m_handhshakeSocket = localSocket;
        emit handlerSocketUpdated(handlerName, handlerSocket);
        return;
    }

    if(contactType == "SUICIDE")
    {
        QPointer<RequestHandlerAgent> handler = d->handler(handlerName);

        if(!handler)
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Invalid handler name received for suicide " + handlerName);
            qDebug() << "Invalid handler name received for suicide " + handlerName;
            localSocket->deleteLater();
            return;
        }

        handler->deactivate();
    }
}

void FiberControlCentre::handlerDeactivated()
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    RequestHandlerAgent *handler = qobject_cast<RequestHandlerAgent*>(this->sender());

    if(!handler)
        return;

    if(d->sessionHandlerAgentMap.isEmpty() ||
       handler->m_handledSessions.isEmpty())
    {
        delete handler;
        return;
    }

    Q_FOREACH(const QString &session, handler->m_handledSessions)
        d->sessionHandlerAgentMap.remove(session);

    ::ExistingRequestHandlerNames()->removeAll(handler->m_handlerName);

    delete handler;
}

///////////////////////////////////////////////////////////////////////////////
/// RequestHandlerAgent
///////////////////////////////////////////////////////////////////////////////

RequestHandlerAgent::RequestHandlerAgent(QObject *parent)
    :QObject(parent)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    m_deactivating = false;

    QString baseName = "Handler";
    int count = 1;
    QString handlerName = QString("%1%2").arg(baseName).arg(count);

    while(::ExistingRequestHandlerNames()->contains(handlerName))
        handlerName = QString("%1%2").arg(baseName).arg(++count);

    ::ExistingRequestHandlerNames()->append(handlerName);

    m_handlerName = handlerName;
    m_handlerProcess = new QProcess(parent);

    connect(m_handlerProcess.data(),
            SIGNAL(finished(int,QProcess::ExitStatus)),
            this,
            SLOT(handlerProcessExited(int,QProcess::ExitStatus)));
}

RequestHandlerAgent::~RequestHandlerAgent()
{
    if(m_handlerProcess)
    {
        m_handlerProcess->deleteLater();
        m_handlerProcess = 0;
    }

    if(m_handhshakeSocket)
    {
        m_handhshakeSocket->deleteLater();
        m_handhshakeSocket = 0;
        emit deactivated();
    }
}

void RequestHandlerAgent::handlerProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, QString("%1 exited with error code %2").arg(m_handlerName).arg(exitCode));

    // We dont't want anybody contacting the process when it is down
    m_handlerSocket = QString();

    if(m_deactivating)
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Terminating " + m_handlerName);
        emit deactivated();
        m_handlerProcess->deleteLater();
        m_handlerProcess = 0;
        return;
    }

    if(exitStatus == QProcess::CrashExit)
    {
        QString message = QString("Handler process for %1 crashed. Attempting restart.").arg(m_handlerName);
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, message);
        qDebug() << message;
    }
    else
    {
        QString message = QString("Handler process for %1 terminated without permission. Attempting restart.").arg(m_handlerName);
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, message);
        qDebug() << message;
    }

    if(!m_handlerProcess)
    {
        QString message = QString("Handler agent for %1 in a contaminated state. Not rebooting the handler after restart").arg(m_handlerName);
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, message);
        qDebug() << message;
        return;
    }

    m_handlerProcess->start(QString("%1 %2").arg("FiberRequestHandler").arg(m_handlerName));
}

void RequestHandlerAgent::activate()
{
    m_handlerProcess->start(QString("%1 %2").arg("FiberRequestHandler").arg(m_handlerName));

    if(m_handlerProcess->waitForStarted())
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, m_handlerName + " started succesfully");
        emit activated();
    }
    else
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, m_handlerName + " couldn't be started");
    }
}

void RequestHandlerAgent::deactivate()
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    if(m_deactivating)
        return;

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Deactivating " + m_handlerName);
    m_deactivating = true;
    m_handhshakeSocket->deleteLater();
}

