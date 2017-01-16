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

#include <GCF3/Application>
#include <GCF3/IpcServer>
#include <GCF3/Version>
#include <GCF3/IpcServerDiscovery>

#include "TestService.h"

static int ServerPort = 49772;

int main(int argc, char **argv)
{
    GCF::Application a(argc, argv);

    ::ServerPort = a.argumentsMap().value("--serverPort", ::ServerPort).toInt();

    GCF::Log::instance()->setLogFileName("IpcLog.txt");

    TestService testService;
    QVariantMap testServiceInfo;
    testServiceInfo["allowmetaaccess"] = true;
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(),
                            "TestService", &testService,
                            testServiceInfo);

    QObject object; // this object, though exposed, is never accessible for remote access.
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(),
                            "Object", &object);

    GCF::IpcServer ipcServer;
    if( ipcServer.listen( QHostAddress::LocalHost, ::ServerPort ) )
        qDebug() << "IpcRemoteApp Server. Listening on " << ipcServer.serverPort();
    else
    {
        qDebug() << "Port " << ::ServerPort << " not available for listening. Another process maybe using it.";
        return 0;
    }

    GCF::IpcServerDiscovery serverDiscovery;
    if(a.argumentsMap().contains("--enableDiscovery"))
    {
        GCF::Result result = serverDiscovery.start();
        if(result)
            qDebug() << "GCF::IpcServerDiscovery started";
        else
            qDebug() << "GCF::IpcServerDiscovery could not start " << result.message();
    }

    return a.exec();
}

