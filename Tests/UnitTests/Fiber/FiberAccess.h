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

#ifndef FIBERACCESS_H
#define FIBERACCESS_H

#include <QObject>
#include <QVariantMap>
#include <QByteArray>

struct FiberAccessData;
class FiberAccess : public QObject
{
    Q_OBJECT

public:
    enum Status
    {
        NotInAnyCall = 0,
        RequestingSessionCreation,
        RequestedSessionCreation,
        RequestingSessionTermination,
        RequestedSessionTermination,
        PlacingCall,
        PlacedCall,
        LazyAsyncCallResultAnnounced,
        LazyAsyncCallResultDownloading,
        CancellingLazyAsyncCall,
        Unknown
    };

    FiberAccess(const QString &fiberAddress=QString(), QObject *parent = 0);
    ~FiberAccess();

    void setFiberAddress(const QString &fiberAddress);
    QString fiberAddress() const;

    bool hasSession() const;
    bool createSession();
    bool terminateSession();
    bool copySessionFrom(const FiberAccess &fiberAccess);

    // NOTE: ArgsCount <= 9
    void asyncCall(const QString &libraryName,
             const QString &componentName,
             const QString &objectName,
             const QString &methodName,
             const QString &componentType="CORE",
             const QVariantList &args=QVariantList());

    // NOTE: ArgsCount <= 9
    void lazyAsyncCall(const QString &libraryName,
             const QString &componentName,
             const QString &objectName,
             const QString &methodName,
             const QString &componentType="CORE",
             const QVariantList &args=QVariantList());

    void cancelLazyAsyncCall();

    FiberAccess::Status status() const;
    int availableResponseSize() const;
    QVariantMap lastCallRequest() const;
    QVariantMap lastCallResult() const;

public slots:
    // -1 stands to download the entire thing in one shot
    void downloadLazyAsyncCallResult(int chunkSize=-1);

protected slots:
    // Slot to update status after sending each request (both Async & LazyAsync)
    void slotCallerRequestSent(const QByteArray &jsonData);

    // Slot to deal with Async call response
    void slotAsyncCallResponse(const QByteArray &jsonData);

    // Slots to deal with LazyAsync call responses
    void slotPingTimerTimeout();
    void slotLazyAsyncCallRequestResult(const QByteArray& jsonData);
    void slotLazyAsyncCallAvailableResponsesResult(const QByteArray &jsonData);
    void slotLazyAsyncCallAvailableResponseSizeResult(const QByteArray& jsonData);
    void slotLazyAsyncCallResponseDownload(const QByteArray& jsonData);
    void slotLazyAsyncCallCancelRequestsResult(const QByteArray &jsonData);

    // Slots to deal with session handling request responses
    void slotCreateSessionResult(const QByteArray &jsonData);
    void slotTerminateSessionResult(const QByteArray &jsonData);

signals:
    void asyncCallResultAvailable();

    void lazyAsyncCallResultAnnounced(int responseSize);
    void lazyAsyncCallResultDownloadProgress(int progress, int available);
    void lazyAsyncCallResultAvailable();
    void lazyAsyncCallCancelled();
    void lazyAsyncCallCancelFailed();

    void sessionCreationFailed();
    void sessionCreated();
    void sessionTerminated();
    void sessionTerminationFailed();

private:
    FiberAccessData *d;
};

#endif // FIBERACCESS_H
