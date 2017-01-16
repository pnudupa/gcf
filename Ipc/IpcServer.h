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

#ifndef IPCSERVER_H
#define IPCSERVER_H

#include "IpcCommon.h"

#include <QList>
#include <QTcpServer>

namespace GCF
{

class IpcMessage;
class GCF_IPC_EXPORT IpcServer : public QTcpServer
{
    Q_OBJECT

public:
    static QList<GCF::IpcServer*> servers();

    IpcServer(QObject *parent=0);
    ~IpcServer();

protected:
#if QT_VERSION >= 0x050000
    void incomingConnection(qintptr handle);
#else
    void incomingConnection(int handle);
#endif

private slots:
    void onIncomingMessage(const GCF::IpcMessage &message);
};

}

#endif // IPCSERVER_H
