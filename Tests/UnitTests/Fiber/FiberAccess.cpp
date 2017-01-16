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

#include "FiberAccess.h"
#include "HTTPCaller.h"
#include "Json.h"

#include <QNetworkAccessManager>
#include <QTimer>
#include <QDebug>

struct FiberAccessData
{
    FiberAccessData()
        : sessionType(true)
        , requestID(0)
        , availableResponseSize(-1)
        , downloadChunkSize(-1)
        , status(FiberAccess::NotInAnyCall)
    {
        requestAvailabilityPingTimer.setInterval(1000);
    }

    QString sessionType;
    int requestID;
    int availableResponseSize;
    int downloadChunkSize;
    FiberAccess::Status status;

    QString fiberServerAddress;
    QString sessionID;

    QVariantMap request;

    QByteArray responseData;
    QVariantMap response;

    QNetworkAccessManager nam;
    QTimer requestAvailabilityPingTimer;

    void reset() {
        requestAvailabilityPingTimer.stop();
        sessionType = QString();
        requestID = 0;
        availableResponseSize = -1;
        downloadChunkSize = -1;
        status = FiberAccess::NotInAnyCall;
        request.clear();
        response.clear();
        responseData.clear();
    }
};

FiberAccess::FiberAccess(const QString &fiberAddress, QObject *parent) :
    QObject(parent)
{
    d = new FiberAccessData;

    this->setFiberAddress(fiberAddress);
}

FiberAccess::~FiberAccess()
{
    delete d;
    d = 0;
}

void FiberAccess::setFiberAddress(const QString &fiberAddress)
{
    if(fiberAddress.isEmpty())
        return;

    if(d->fiberServerAddress == fiberAddress)
        return;

    d->fiberServerAddress = fiberAddress;
}

QString FiberAccess::fiberAddress() const
{
    return d->fiberServerAddress;
}

bool FiberAccess::hasSession() const
{
    return !d->sessionID.isEmpty();
}

bool FiberAccess::createSession()
{
    if(this->hasSession())
        return false;

    d->status = FiberAccess::RequestingSessionCreation;
    QVariantMap requestData;
    requestData["requestType"] = "SESSION_MGMT";
    requestData["sessionCommand"] = "create";

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
    connect(caller, SIGNAL(requestSent(QByteArray)), this, SLOT(slotCallerRequestSent(QByteArray)));
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotCreateSessionResult(QByteArray)));
    caller->call();

    return true;
}

bool FiberAccess::terminateSession()
{
    if(!this->hasSession())
        return false;

    d->status = FiberAccess::RequestingSessionTermination;
    QVariantMap requestData;
    requestData["requestType"] = "SESSION_MGMT";
    requestData["sessionCommand"] = "terminate";
    requestData["sessionName"] = d->sessionID;

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotTerminateSessionResult(QByteArray)));
    caller->call();

    return true;
}

bool FiberAccess::copySessionFrom(const FiberAccess &fiberAccess)
{
    Q_UNUSED(fiberAccess)
    return false;
}

void FiberAccess::asyncCall(const QString &libraryName,
                             const QString &componentName,
                             const QString &objectName,
                             const QString &methodName,
                             const QString &componentType,
                             const QVariantList &args)
{
    if(this->status() != FiberAccess::NotInAnyCall)
        return;

    if(componentName.isEmpty() ||
       objectName.isEmpty() ||
       methodName.isEmpty() ||
       (args.count() > 9))
    {
        return;
    }

    if(d->fiberServerAddress.isEmpty())
        return;

    d->reset();

    d->sessionType = componentType.toUpper();

    QVariantMap requestData;
    requestData["requestType"] = "SERVICE_REQUEST";
    requestData["sessionType"] = d->sessionType;
    requestData["sessionName"] = d->sessionID;
    requestData["serviceLibrary"] = libraryName;
    requestData["serviceComponent"] = componentName;
    requestData["serviceObject"] = objectName;
    requestData["serviceMethod"] = methodName;
    requestData["args"] = args;

    d->request = requestData;

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);

    connect(caller, SIGNAL(requestSent(QByteArray)), this, SLOT(slotCallerRequestSent(QByteArray)));
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotAsyncCallResponse(QByteArray)));

    d->status = FiberAccess::PlacingCall;
    caller->call();
}

void FiberAccess::lazyAsyncCall(const QString &libraryName,
                                 const QString &componentName,
                                 const QString &objectName,
                                 const QString &methodName,
                                 const QString &componentType,
                                 const QVariantList &args)
{
    if(this->status() != FiberAccess::NotInAnyCall)
        return;

    if(componentName.isEmpty() ||
       objectName.isEmpty() ||
       methodName.isEmpty() ||
       (args.count() > 9))
    {
        return;
    }

    if(d->fiberServerAddress.isEmpty())
        return;

    d->reset();

    d->sessionType = componentType.toUpper();
    QVariantMap requestData;
    requestData["requestType"] = "SERVICE_REQUEST";
    requestData["sessionType"] = d->sessionType;
    requestData["sessionName"] = d->sessionID;
    requestData["blockingCall"] = false;
    requestData["serviceLibrary"] = libraryName;
    requestData["serviceComponent"] = componentName;
    requestData["serviceObject"] = objectName;
    requestData["serviceMethod"] = methodName;
    requestData["args"] = args;

    d->request = requestData;

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);

    connect(caller, SIGNAL(requestSent(QByteArray)), this, SLOT(slotCallerRequestSent(QByteArray)));
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotLazyAsyncCallRequestResult(QByteArray)));

    d->status = FiberAccess::PlacingCall;
    caller->call();
}

void FiberAccess::cancelLazyAsyncCall()
{
    if( (this->status() == NotInAnyCall) )
        return;

    d->status = FiberAccess::CancellingLazyAsyncCall;

    QVariantMap requestData;
    requestData["requestType"] = "SERVICE_REQUEST";
    requestData["sessionName"] = d->sessionID;
    requestData["sessionType"] = d->sessionType;
    requestData["metaCall"] = true;
    requestData["metaMethod"] = "cancelRequests";
    requestData["requestIDs"] = QVariantList() << d->requestID;

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotLazyAsyncCallCancelRequestsResult(QByteArray)));
    caller->call();
}

FiberAccess::Status FiberAccess::status() const
{
    return d->status;
}

int FiberAccess::availableResponseSize() const
{
    return d->availableResponseSize;
}

QVariantMap FiberAccess::lastCallRequest() const
{
    return d->request;
}

QVariantMap FiberAccess::lastCallResult() const
{
    return d->response;
}

void FiberAccess::downloadLazyAsyncCallResult(int chunkSize)
{
    if( (this->status() == FiberAccess::NotInAnyCall) )
        return;

    d->downloadChunkSize = chunkSize;

    QVariantMap requestData;
    requestData["requestType"] = "SERVICE_REQUEST";
    requestData["sessionName"] = d->sessionID;
    requestData["sessionType"] = d->sessionType;
    requestData["metaCall"] = true;
    requestData["metaMethod"] = "response";
    requestData["requestID"] = d->requestID;
    requestData["responseLength"] = d->downloadChunkSize;

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotLazyAsyncCallResponseDownload(QByteArray)));
    caller->call();
}

void FiberAccess::slotCallerRequestSent(const QByteArray &jsonData)
{
    Q_UNUSED(jsonData)

    switch(d->status)
    {
    case FiberAccess::PlacingCall:
        d->status = FiberAccess::PlacedCall;
        return;
    case FiberAccess::RequestingSessionCreation:
        d->status = FiberAccess::RequestedSessionCreation;
        return;
    case FiberAccess::RequestingSessionTermination:
        d->status = FiberAccess::RequestedSessionTermination;
        return;
    default:
        d->status = FiberAccess::Unknown;
        break;
    }
}

void FiberAccess::slotAsyncCallResponse(const QByteArray &jsonData)
{
    this->sender()->deleteLater();

    QVariantMap response = Json().parse(jsonData).toMap();
    d->status = FiberAccess::NotInAnyCall;
    d->response = response;

    emit asyncCallResultAvailable();
}

void FiberAccess::slotPingTimerTimeout()
{
    d->requestAvailabilityPingTimer.stop();

    QVariantMap requestData;
    requestData["requestType"] = "SERVICE_REQUEST";
    requestData["sessionName"] = d->sessionID;
    requestData["sessionType"] = d->sessionType;
    requestData["metaCall"] = true;
    requestData["metaMethod"] = "availableResponses";

    HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
    connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotLazyAsyncCallAvailableResponsesResult(QByteArray)));
    caller->call();
}

void FiberAccess::slotLazyAsyncCallRequestResult(const QByteArray& jsonData)
{
    this->sender()->deleteLater();
    QVariantMap response = Json().parse(jsonData).toMap();

    if(response.isEmpty() || !response.value("success").toBool())
    {
        d->response = response;
        emit lazyAsyncCallResultAvailable();
    }

    d->requestID = response.value("result").toInt();

    if(d->requestID == 0)
    {
        d->response = response;
        emit lazyAsyncCallResultAvailable();
    }

    connect(&d->requestAvailabilityPingTimer, SIGNAL(timeout()),
            this, SLOT(slotPingTimerTimeout()), Qt::UniqueConnection);

    d->requestAvailabilityPingTimer.start();
}

void FiberAccess::slotLazyAsyncCallAvailableResponsesResult(const QByteArray &jsonData)
{
    this->sender()->deleteLater();
    QVariantMap response = Json().parse(jsonData).toMap();

    if(response.isEmpty() || !response.value("success").toBool())
    {
        d->response = response;
        emit lazyAsyncCallResultAvailable();
    }

    QVariantList availableResults = response.value("result").toList();

    if(!availableResults.isEmpty())
    {
        if(availableResults.contains(d->requestID))
        {
            QVariantMap requestData;
            requestData["requestType"] = "SERVICE_REQUEST";
            requestData["sessionName"] = d->sessionID;
            requestData["sessionType"] = d->sessionType;
            requestData["metaCall"] = true;
            requestData["metaMethod"] = "responseSize";
            requestData["requestID"] = d->requestID;

            HTTPCaller *caller = new HTTPCaller(requestData, d->nam, d->fiberServerAddress);
            connect(caller, SIGNAL(responseReceived(QByteArray)), this, SLOT(slotLazyAsyncCallAvailableResponseSizeResult(QByteArray)));
            caller->call();
            return;
        }
    }

    d->requestAvailabilityPingTimer.start();
}

void FiberAccess::slotLazyAsyncCallAvailableResponseSizeResult(const QByteArray& jsonData)
{
    QVariantMap response = Json().parse(jsonData).toMap();

    if(response.isEmpty() || !response.value("success").toBool())
    {
        d->response = Json().parse(d->responseData).toMap();
        emit lazyAsyncCallResultAvailable();
    }

    d->availableResponseSize = response.value("result").toInt();
    d->status = FiberAccess::LazyAsyncCallResultAnnounced;

    emit lazyAsyncCallResultAnnounced(d->availableResponseSize);
}

void FiberAccess::slotLazyAsyncCallResponseDownload(const QByteArray& jsonData)
{
    this->sender()->deleteLater();

    if(jsonData.isEmpty())
    {
        d->response = Json().parse(d->responseData).toMap();
        d->status = FiberAccess::NotInAnyCall;
        emit lazyAsyncCallResultDownloadProgress(d->responseData.size(), d->availableResponseSize);
        emit lazyAsyncCallResultAvailable();
        return;
    }

    d->responseData += jsonData;
    emit lazyAsyncCallResultDownloadProgress(d->responseData.size(), d->availableResponseSize);

    if(d->responseData.size() >= d->availableResponseSize)
    {
        d->response = Json().parse(d->responseData).toMap();
        d->status = FiberAccess::NotInAnyCall;
        emit lazyAsyncCallResultAvailable();
        return;
    }

    this->downloadLazyAsyncCallResult(d->downloadChunkSize);
}

void FiberAccess::slotLazyAsyncCallCancelRequestsResult(const QByteArray &jsonData)
{
    this->sender()->deleteLater();

    if(jsonData.isEmpty())
        return;

    QVariantList cancelledIDs = Json().parse(jsonData).toMap().value("result").toList();

    if(cancelledIDs.contains(d->requestID))
        emit lazyAsyncCallCancelled();
    else
        emit lazyAsyncCallCancelFailed();

    d->reset();
}

void FiberAccess::slotCreateSessionResult(const QByteArray &jsonData)
{
    this->sender()->deleteLater();
    d->status = FiberAccess::NotInAnyCall;

    if(jsonData.isEmpty())
    {
        qDebug() << "Session creation failed";
        emit sessionCreationFailed();
        return;
    }

    QVariantMap json = Json().parse(jsonData).toMap();

    if(json.isEmpty())
    {
        emit sessionCreationFailed();
        return;
    }

    if(!json.value("success").toBool())
    {
        qDebug() << json.value("error").toString();
        emit sessionCreationFailed();
        return;
    }

    d->sessionID = json.value("result").toString();
    emit sessionCreated();
}

void FiberAccess::slotTerminateSessionResult(const QByteArray &jsonData)
{
    this->sender()->deleteLater();
    d->status = FiberAccess::NotInAnyCall;

    if(jsonData.isEmpty())
    {
        emit sessionTerminationFailed();
        return;
    }

    QVariantMap json = Json().parse(jsonData).toMap();

    if(json.isEmpty())
    {
        emit sessionTerminationFailed();
        return;
    }

    if(!json.value("success").toBool())
    {
        emit sessionTerminationFailed();
        return;
    }

    d->reset();
    d->sessionID = QString();

    emit sessionTerminated();
}
