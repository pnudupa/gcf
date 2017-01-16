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

#ifndef FIBERSURROGATESERVER_H
#define FIBERSURROGATESERVER_H

#include <QLocalServer>
#include <QLocalSocket>

class FiberSurrogateServer : public QLocalServer
{
    Q_OBJECT

public:
    FiberSurrogateServer(const QString &connectionString,
                      const QString &reportingHandler,
                      QObject *parent = 0);
    ~FiberSurrogateServer();

protected slots:
    void slotHandlerDisconnected();

protected:
    void incomingConnection(quintptr socketDescriptor);
    void createLocalSocket(quintptr socketDescriptor);
};

class FiberSurrogateLocalSocket : public QLocalSocket
{
    Q_OBJECT

public:
    FiberSurrogateLocalSocket(QObject *parent=0);

protected slots:
    void slotReadyRead();

protected:
    void handleRequest(const QVariantMap &jsonRequest);
    void writeResponse(const QByteArray &jsonData);

private:
    friend class FiberSurrogateServer;
    QString m_sessionServerName;
    QByteArray m_requestData;
};

#endif // FIBERSURROGATESERVER_H
