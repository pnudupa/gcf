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

#ifndef FIBERREQUESTHANDLERSERVER_H
#define FIBERREQUESTHANDLERSERVER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QBasicTimer>

class FiberRequestHandlerServer : public QLocalServer
{
    Q_OBJECT

public:
    FiberRequestHandlerServer(const QString& connectionString="Fiber", QObject *parent = 0);
    ~FiberRequestHandlerServer();

protected slots:
    void slotFiberDisconnected();

protected:
    void timerEvent(QTimerEvent *);
    void incomingConnection(quintptr socketDescriptor);
    void createLocalSocket(quintptr socketDescriptor);

private:
    QBasicTimer m_suicideTimer;
};

class FiberRequestHandlerLocalSocket : public QLocalSocket
{
    Q_OBJECT

public:
    FiberRequestHandlerLocalSocket(QObject *parent=0);

protected slots:
    void slotReadyRead();
    void slotDisconnected();
    void slotCoreServiceResponseAvailable(int requestID);
    void slotGuiServiceResponseAvailable(int requestID);

protected:
    void processRequest(const QVariantMap &jsonRequest);
    void processMetaCall(const QVariantMap &jsonRequest);
    void processAvailableResponsesCall(const QVariantMap &jsonRequest);
    void processResponseCall(const QVariantMap &jsonRequest);
    void processResponseSizeCall(const QVariantMap &jsonRequest);
    void processCancelRequestsCall(const QVariantMap &jsonRequest);
    void writeResponse(const QByteArray &jsonData);

signals:
    void done();

private:
    friend class FiberRequestHandlerServer;
    int m_requestID;
    QByteArray m_requestData;
};

#endif // FIBERREQUESTHANDLERSERVER_H
