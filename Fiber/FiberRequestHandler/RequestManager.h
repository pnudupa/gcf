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

#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QTimer>
#include <QPointer>

struct RequestManagerData;
class RequestManager : public QObject
{
    Q_OBJECT

public:
    static RequestManager *instance();
    ~RequestManager();

    bool shutdown();

    void setRequestHandlerName(const QString &name);
    QString requestHandlerName() const;

    int coreServiceRequest(const QByteArray &request);
    int guiServiceRequest(const QByteArray &request);

    bool cancelCoreServiceRequest(int requestID);
    bool cancelGuiServiceRequest(int requestID);

    bool isIdle() const;
    bool isRequestQueueEmpty() const;
    bool isResponseQueueEmpty() const;

    QList<int> pendingCoreServiceRequests() const;
    QList<int> pendingGuiServiceRequests() const;
    QList<int> pendingCoreServiceResponses() const;
    QList<int> pendingGuiServiceResponses() const;

    int coreServiceResponseSize(int requestID) const;
    QByteArray coreServiceResponse(int requestID, int numberOfBytes=-1);
    int guiServiceResponseSize(int requestID) const;
    QByteArray guiServiceResponse(int requestID, int numberOfBytes=-1);

protected slots:
    void slotRequestManagerServerNewConnection();
    void slotRequestManagerSocketReadyRead();

    void processNextCoreRequest();
    void processNextGuiRequest();

    void slotCoreResponseAvailable();
    void slotGuiResponseAvailable();

    void slotCoreSurrogateExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotGuiSurrogateExited(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    RequestManager(QObject *parent = 0);

    void launchCoreSurrogate();
    void launchGuiSurrogate();

signals:
    void requestManagerShutdown();
    void newCoreServiceRequested(int requestID);
    void newGuiServiceRequested(int requestID);
    void coreServiceResponseAvailable(int requestID);
    void guiServiceResponseAvailable(int requestID);
    void coreSurrogateConnected();
    void guiSurrogateConnected();

private:
    RequestManagerData *d;
};

class RequestManagerLocalSocket : public QLocalSocket
{
    Q_OBJECT

public:
    RequestManagerLocalSocket(int requestID, QObject *parent=0);

    int requestID() const;

protected slots:
    void slotReadyRead();

signals:
    void responseAvailable();

private:
    friend class RequestManager;
    int m_requestID;
    QByteArray m_responseData;
};

class FiberSurrogateDeadlockDetector : public QObject
{
    Q_OBJECT

protected:
    FiberSurrogateDeadlockDetector(QProcess *surrogateProcess,
                                       const QString &surrogateAddress,
                                       QObject *parent=0);

protected slots:
    void slotDeadlockTimeout();

private:
    friend class RequestManager;
    QPointer<QProcess> m_surrogateProcess;
    QString m_surrogateAddress;
    QTimer m_deadlockTimer;
    bool m_deadLockDetected;
};

#endif // REQUESTMANAGER_H
