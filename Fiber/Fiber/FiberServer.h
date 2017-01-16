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

#ifndef FIBERSERVER_H
#define FIBERSERVER_H

#include <QLocalServer>
#include <QLocalSocket>

class FiberServer : public QLocalServer
{
    Q_OBJECT

public:
    FiberServer(const QString &connectionString="Fiber", QObject *parent = 0);
    ~FiberServer();

protected:
    void incomingConnection(quintptr socketDescriptor);
};

class FiberMainLocalSocket : public QLocalSocket
{
    Q_OBJECT

public:
    FiberMainLocalSocket(QObject *parent=0);

protected slots:
    void slotReadyRead();
    void slotHandlerSocketUpdated(const QString &handlerName,
                                    const QString &handlerSocket);

protected:
    void processRequest(const QVariantMap &requestObj);
    void processFiberControlRequest(const QVariantMap &requestObj);
    void processHandlerRequest(const QVariantMap &requestObj);
    void processSessionMgmtRequest(const QVariantMap &requestObj);
    void writeResponse(const QByteArray &jsonData);

private:
    friend class FiberServer;
    QString m_handlerName;
};

#endif // FIBERSERVER_H
