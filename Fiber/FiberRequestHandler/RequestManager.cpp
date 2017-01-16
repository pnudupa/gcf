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

#include "RequestManager.h"
#include "FiberCommon.h"
#include "Json.h"

#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QProcess>
#include <QPair>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>

#include <GCF3/Application>

class ServiceRequestManager2 : public RequestManager
{
public:
    ServiceRequestManager2() { }
    ~ServiceRequestManager2() { }
};

Q_GLOBAL_STATIC(ServiceRequestManager2, GlobalServiceRequestManager)
RequestManager *RequestManager::instance()
{
    return GlobalServiceRequestManager();
}

typedef QPair < QString, QPointer<QProcess> > SurrogateInfo;
typedef QPair < int, QByteArray > FiberMessage;
struct RequestManagerData
{
    RequestManagerData()
        : coreRequestID(0)
        , guiRequestID(0)
    { }

    // Counters for requestID
    int coreRequestID;
    int guiRequestID;

    QMutex managerMutex;

    QString handlerName;
    QLocalServer requestManagerServer;

    SurrogateInfo coreSurrogateInfo;
    SurrogateInfo guiSurrogateInfo;

    // Request and Response message queues
    QList<FiberMessage> coreRequests;
    QList<FiberMessage> guiRequests;
    QList<FiberMessage> coreResponses;
    QList<FiberMessage> guiResponses;

    // Objects to check whether a corresponding surrogate has gone on an infinite loop
    QPointer<FiberSurrogateDeadlockDetector> coreDeadlockDetector;
    QPointer<FiberSurrogateDeadlockDetector> guiDeadlockDetector;
};

RequestManager::RequestManager(QObject *parent) :
    QObject(parent)
{
    d = new RequestManagerData;

    // We need to process the request queues as soon as the surrogates
    // come online
    connect(this, SIGNAL(coreSurrogateConnected()),
            this, SLOT(processNextCoreRequest()), Qt::QueuedConnection);
    connect(this, SIGNAL(guiSurrogateConnected()),
            this, SLOT(processNextGuiRequest()), Qt::QueuedConnection);

    // Check whether we can process the request as soon as it was
    // placed
    connect(this, SIGNAL(newCoreServiceRequested(int)),
            this, SLOT(processNextCoreRequest()), Qt::QueuedConnection);
    connect(this, SIGNAL(newGuiServiceRequested(int)),
            this, SLOT(processNextGuiRequest()), Qt::QueuedConnection);

    // Check whether there are any more requests to process as soon as
    // we are done with one
    connect(this, SIGNAL(coreServiceResponseAvailable(int)),
            this, SLOT(processNextCoreRequest()), Qt::QueuedConnection);
    connect(this, SIGNAL(guiServiceResponseAvailable(int)),
            this, SLOT(processNextGuiRequest()), Qt::QueuedConnection);
}

RequestManager::~RequestManager()
{
    d->requestManagerServer.close();
    QLocalServer::removeServer(d->requestManagerServer.serverName());
    delete d;
    d = 0;
}

bool RequestManager::shutdown()
{
    // TODO : Do a memory dump perhaps
    emit requestManagerShutdown();
    return true;
}

void RequestManager::setRequestHandlerName(const QString &name)
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->handlerName.isEmpty())
        return;

    d->handlerName = name;
    d->coreSurrogateInfo.first = d->handlerName + "CoreSurrogate";
    d->guiSurrogateInfo.first = d->handlerName + "GuiSurrogate";

    QString managerSocket = name + "RequestManager";
    QLocalServer::removeServer(managerSocket);
    d->requestManagerServer.listen(managerSocket);

    connect(&d->requestManagerServer, SIGNAL(newConnection()),
            this, SLOT(slotRequestManagerServerNewConnection()));
}

QString RequestManager::requestHandlerName() const
{
    return d->handlerName;
}

int RequestManager::coreServiceRequest(const QByteArray &request)
{
    QMutexLocker locker(&d->managerMutex);

    ++d->coreRequestID;
    if(d->coreRequestID == 0) // 0 is reserved for invalid requestID
        ++d->coreRequestID;

    FiberMessage info;
    info.first = d->coreRequestID;
    info.second = request;
    d->coreRequests << info;

    emit newCoreServiceRequested(d->coreRequestID);
    return d->coreRequestID;
}

int RequestManager::guiServiceRequest(const QByteArray &request)
{
    QMutexLocker locker(&d->managerMutex);

    ++d->guiRequestID;
    if(d->guiRequestID == 0) // 0 is reserved for invalid requestID
        ++d->guiRequestID;

    FiberMessage info;
    info.first = d->guiRequestID;
    info.second = request;
    d->guiRequests << info;

    emit newGuiServiceRequested(d->guiRequestID);
    return d->guiRequestID;
}

bool RequestManager::cancelCoreServiceRequest(int requestID)
{
    QMutexLocker locker(&d->managerMutex);

    if(d->coreRequests.count())
    {
        Q_FOREACH(const FiberMessage &info, d->coreRequests)
        {
            if(info.first == requestID)
            {
                d->coreRequests.removeAll(info);
                return true;
            }
        }
    }

    // Lets remove the response as well, if for some reason its there
    if(d->coreResponses.count())
    {
        Q_FOREACH(const FiberMessage &info, d->coreResponses)
        {
            if(info.first == requestID)
            {
                d->coreResponses.removeAll(info);
                return true;
            }
        }
    }

    return false;
}

bool RequestManager::cancelGuiServiceRequest(int requestID)
{
    QMutexLocker locker(&d->managerMutex);

    if(d->guiRequests.count())
    {
        Q_FOREACH(const FiberMessage &info, d->guiRequests)
        {
            if(info.first == requestID)
            {
                d->guiRequests.removeAll(info);
                return true;
            }
        }
    }

    // Lets remove the response as well, if for some reason its there
    if(d->guiResponses.count())
    {
        Q_FOREACH(const FiberMessage &info, d->guiResponses)
        {
            if(info.first == requestID)
            {
                d->guiResponses.removeAll(info);
                return true;
            }
        }
    }

    return false;
}

bool RequestManager::isIdle() const
{
    QMutexLocker locker(&d->managerMutex);

    bool idle = (!d->coreRequests.count()) &&
            (!d->coreResponses.count()) &&
            (!d->guiRequests.count()) &&
            (!d->guiResponses.count());
    return idle;
}

bool RequestManager::isRequestQueueEmpty() const
{
    QMutexLocker locker(&d->managerMutex);

    return (!d->coreRequests.count() && !d->guiRequests.count());
}

bool RequestManager::isResponseQueueEmpty() const
{
    QMutexLocker locker(&d->managerMutex);

    return (!d->guiRequests.count() && !d->guiResponses.count());
}

QList<int> RequestManager::pendingCoreServiceRequests() const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->coreRequests.count())
        return QList<int>();

    QList<int> pendingList;
    Q_FOREACH(const FiberMessage &info, d->coreRequests)
        pendingList << info.first;

    return pendingList;
}

QList<int> RequestManager::pendingGuiServiceRequests() const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->guiRequests.count())
        return QList<int>();

    QList<int> pendingList;
    Q_FOREACH(const FiberMessage &info, d->guiRequests)
        pendingList << info.first;

    return pendingList;
}

QList<int> RequestManager::pendingCoreServiceResponses() const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->coreResponses.count())
        return QList<int>();

    QList<int> pendingList;
    Q_FOREACH(const FiberMessage &info, d->coreResponses)
        pendingList << info.first;

    return pendingList;
}

QList<int> RequestManager::pendingGuiServiceResponses() const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->guiResponses.count())
        return QList<int>();

    QList<int> pendingList;
    Q_FOREACH(const FiberMessage &info, d->guiResponses)
        pendingList << info.first;

    return pendingList;
}

int RequestManager::coreServiceResponseSize(int requestID) const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->coreResponses.count())
        return -1;

    Q_FOREACH(FiberMessage info, d->coreResponses)
        if(info.first == requestID)
            return info.second.size();

    return -1;
}

QByteArray RequestManager::coreServiceResponse(int requestID, int numberOfBytes)
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->coreResponses.count())
        return QByteArray();

    for(int i=0; i<d->coreResponses.count(); ++i)
    {
        FiberMessage &info = d->coreResponses[i];

        if(info.first == requestID)
        {
            numberOfBytes = (numberOfBytes <= 0) ?
                        info.second.size() :
                        numberOfBytes;

            QByteArray data = info.second.left(numberOfBytes);
            info.second.remove(0, numberOfBytes);

            if(!info.second.size())
                d->coreResponses.removeAt(i);

            return data;
        }
    }

    return QByteArray();
}

int RequestManager::guiServiceResponseSize(int requestID) const
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->guiResponses.count())
        return -1;

    Q_FOREACH(FiberMessage info, d->guiResponses)
        if(info.first == requestID)
            return info.second.size();

    return -1;
}

QByteArray RequestManager::guiServiceResponse(int requestID, int numberOfBytes)
{
    QMutexLocker locker(&d->managerMutex);

    if(!d->guiResponses.count())
        return QByteArray();

    for(int i=0; i<d->guiResponses.count(); ++i)
    {
        FiberMessage &info = d->guiResponses[i];

        if(info.first == requestID)
        {
            numberOfBytes = (numberOfBytes <= 0) ?
                        info.second.size() :
                        numberOfBytes;

            QByteArray data = info.second.left(numberOfBytes);
            info.second.remove(0, numberOfBytes);

            if(!info.second.size())
                d->guiResponses.removeAt(i);

            return data;
        }
    }

    return QByteArray();
}

void RequestManager::slotRequestManagerServerNewConnection()
{
    QLocalSocket *socket = d->requestManagerServer.nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotRequestManagerSocketReadyRead()));
}

void RequestManager::slotRequestManagerSocketReadyRead()
{
    QLocalSocket *localSocket = qobject_cast<QLocalSocket*>(sender());
    QByteArray data = localSocket->readAll();

    if(data.isEmpty())
    {
        qDebug() << "Empty handshake received";
        localSocket->deleteLater();
        return;
    }

    QVariantMap json = Json().parse(data).toMap();
    if(json.isEmpty())
    {
        qDebug() << "Incorrect JSON format in handshake";
        localSocket->deleteLater();
        return;
    }

    QString surrogateName = json.value("surrogateName").toString();
    QString surrogateSocket = json.value("surrogateSocket").toString();
    QString contactType = json.value("contactType").toString();

    if(surrogateName.isEmpty() ||
       surrogateSocket.isEmpty() ||
       contactType.isEmpty() )
    {
        qDebug() << "Incomplete data received at handshake";
        localSocket->deleteLater();
        return;
    }

    if(contactType == "HANDSHAKE")
    {
        if(surrogateName == d->coreSurrogateInfo.first)
            emit coreSurrogateConnected();
        else
            emit guiSurrogateConnected();

        // We let the connection to be alive so that the handler can know
        // whether the surrogate is alive or not.
        connect(localSocket, SIGNAL(disconnected()), localSocket, SLOT(deleteLater()));

        return;
    }
}

void RequestManager::processNextCoreRequest()
{
    if(!d->coreRequests.count())
        return;

    if(d->coreSurrogateInfo.first.isEmpty())
        return;

    if(d->coreSurrogateInfo.second.isNull() ||
       d->coreSurrogateInfo.second->state() != QProcess::Running)
    {
        this->launchCoreSurrogate();
        return;
    }

    if(!d->coreRequests.count())
        return;

    FiberMessage info = d->coreRequests.first();
    RequestManagerLocalSocket *localSocket =
            new RequestManagerLocalSocket(info.first, this);
    localSocket->connectToServer(d->coreSurrogateInfo.first);
    localSocket->waitForConnected();

    if(localSocket->state() != QLocalSocket::ConnectedState)
    {
        // We need to wait for the process to do a handshake.
        localSocket->deleteLater();
        return;
    }

    connect(localSocket, SIGNAL(responseAvailable()),
            this, SLOT(slotCoreResponseAvailable()));
    localSocket->write(info.second);
}

void RequestManager::processNextGuiRequest()
{
    if(d->guiRequests.isEmpty())
        return;

    if(d->guiSurrogateInfo.first.isEmpty())
        return;

    if(d->guiSurrogateInfo.second.isNull() ||
       d->guiSurrogateInfo.second->state() != QProcess::Running)
    {
        this->launchGuiSurrogate();
        return;
    }

    if(!d->guiRequests.count())
        return;

    FiberMessage info = d->guiRequests.first();
    RequestManagerLocalSocket *localSocket =
            new RequestManagerLocalSocket(info.first, this);
    localSocket->connectToServer(d->guiSurrogateInfo.first);
    localSocket->waitForConnected();

    if(localSocket->state() != QLocalSocket::ConnectedState)
    {
        // We need to wait for the handler to do a handshake.
        localSocket->deleteLater();
        return;
    }

    connect(localSocket, SIGNAL(responseAvailable()),
            this, SLOT(slotGuiResponseAvailable()));
    localSocket->write(info.second);
}

void RequestManager::slotCoreResponseAvailable()
{
    RequestManagerLocalSocket *socket =
            qobject_cast<RequestManagerLocalSocket*>(this->sender());

    if(d->coreRequests.isEmpty())
        return; // Somebody has cancelled the entire requests

    int requestID = socket->requestID();
    if(requestID != d->coreRequests.first().first)
        return; // Somebody has cancelled this request

    d->coreRequests.takeFirst();
    FiberMessage response;
    response.first = requestID;
    response.second = socket->m_responseData;
    d->coreResponses << response;

    socket->deleteLater();
    emit coreServiceResponseAvailable(requestID);
}

void RequestManager::slotGuiResponseAvailable()
{
    RequestManagerLocalSocket *socket =
            qobject_cast<RequestManagerLocalSocket*>(this->sender());

    if(d->guiRequests.isEmpty())
        return; // Somebody has cancelled the entire requests

    int requestID = socket->requestID();

    if(requestID != d->guiRequests.first().first)
        return; // Somebody has cancelled this request

    d->guiRequests.takeFirst();
    FiberMessage response;
    response.first = requestID;
    response.second = socket->m_responseData;
    d->guiResponses << response;

    socket->deleteLater();
    emit guiServiceResponseAvailable(requestID);
}

void RequestManager::slotCoreSurrogateExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(!d->coreRequests.count())
        return;

    if( (exitStatus == QProcess::CrashExit) ||
        (exitCode != 0) )
    {
        FiberMessage info = d->coreRequests.first();
        FiberMessage response;
        response.first = info.first;

        if(d->coreDeadlockDetector->m_deadLockDetected)
            response.second = Fiber::ErrorStringToJSON("Crashed the server with a deadlock");
        else
            response.second = Fiber::ErrorStringToJSON("Crashed the server");

        d->coreRequests.removeAll(info);
        d->coreResponses << response;
        emit coreServiceResponseAvailable(info.first);
    }

    if(d->coreRequests.count())
        this->launchCoreSurrogate();
}

void RequestManager::slotGuiSurrogateExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(!d->guiRequests.count())
        return;

    if( (exitStatus == QProcess::CrashExit) ||
        (exitCode != 0) )
    {
        FiberMessage info = d->guiRequests.first();
        FiberMessage response;
        response.first = info.first;

        if(d->guiDeadlockDetector->m_deadLockDetected)
            response.second = Fiber::ErrorStringToJSON("Crashed the server with a deadlock");
        else
            response.second = Fiber::ErrorStringToJSON("Crashed the server");

        d->guiRequests.removeAll(info);
        d->guiResponses << response;
        emit guiServiceResponseAvailable(info.first);
    }

    if(d->guiRequests.count())
        this->launchGuiSurrogate();
}

void RequestManager::launchCoreSurrogate()
{
    QProcess *process = d->coreSurrogateInfo.second;

    if(!process)
    {
        process = new QProcess(this);
        d->coreDeadlockDetector =
                new FiberSurrogateDeadlockDetector(process,
                                                       d->coreSurrogateInfo.first,
                                                       this);
    }

    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotCoreSurrogateExited(int,QProcess::ExitStatus)),
            Qt::UniqueConnection);

    QString program = QString("%1 %2 %3")
            .arg("FiberCoreSurrogate")
            .arg(d->coreSurrogateInfo.first)
            .arg(d->requestManagerServer.serverName());
    process->start(program);
    process->waitForStarted();
    d->coreSurrogateInfo.second = process;
}

void RequestManager::launchGuiSurrogate()
{
    QProcess *process = d->guiSurrogateInfo.second;

    if(!process)
    {
        process = new QProcess(this);
        d->guiDeadlockDetector =
                new FiberSurrogateDeadlockDetector(process,
                                                       d->guiSurrogateInfo.first,
                                                       this);
    }

    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotGuiSurrogateExited(int,QProcess::ExitStatus)),
            Qt::UniqueConnection);

    QString program = QString("%1 %2 %3")
            .arg("FiberGuiSurrogate")
            .arg(d->guiSurrogateInfo.first)
            .arg(d->requestManagerServer.serverName());
    process->start(program);
    process->waitForStarted();
    d->guiSurrogateInfo.second = process;
}

///////////////////////////////////////////////////////////////////////////////
/// ServiceRequestmanagerLocalSocket
///////////////////////////////////////////////////////////////////////////////

RequestManagerLocalSocket::RequestManagerLocalSocket(int requestID, QObject *parent)
    :QLocalSocket(parent), m_requestID(requestID)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

int RequestManagerLocalSocket::requestID() const
{
    return m_requestID;
}

void RequestManagerLocalSocket::slotReadyRead()
{
    QByteArray data = this->readAll();

    if(data.isEmpty())
        return;

    m_responseData += data;

    if(!data.endsWith(Fiber::FiberMessageFooter))
        return;

    m_responseData.replace(Fiber::FiberMessageFooter, "");
    emit responseAvailable();
}

///////////////////////////////////////////////////////////////////////////////
/// FiberSurrogateDeadlockInvestigator
///
///////////////////////////////////////////////////////////////////////////////

FiberSurrogateDeadlockDetector::FiberSurrogateDeadlockDetector(QProcess *surrogateProcess,
                                                                       const QString &surrogateAddress,
                                                                       QObject *parent)
    : QObject(parent)
    , m_surrogateProcess(surrogateProcess)
    , m_surrogateAddress(surrogateAddress)
{
    m_deadLockDetected = false;
    m_deadlockTimer.setInterval(10000);

    if(m_surrogateProcess &&
      (m_surrogateProcess->state() == QProcess::Running) )
    {
        m_deadlockTimer.start();
    }

    connect(&m_deadlockTimer, SIGNAL(timeout()),
            this, SLOT(slotDeadlockTimeout()));
    connect(surrogateProcess, SIGNAL(started()),
            &m_deadlockTimer, SLOT(start()));
    connect(surrogateProcess, SIGNAL(destroyed()),
            this, SLOT(deleteLater()), Qt::QueuedConnection);
    connect(surrogateProcess, SIGNAL(destroyed()),
            &m_deadlockTimer, SLOT(stop()));
    connect(surrogateProcess, SIGNAL(finished(int)),
            &m_deadlockTimer, SLOT(stop()));
}

void FiberSurrogateDeadlockDetector::slotDeadlockTimeout()
{
    QLocalSocket socket(this);
    socket.connectToServer(m_surrogateAddress);
    if(!socket.waitForConnected())
    {
        m_deadLockDetected = true;
        m_surrogateProcess->kill();
    }
    else
    {
        m_deadLockDetected = false;
    }
}
