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
#include "FiberConfigurationSettings.h"
#include "FiberServer.h"

#include <QCoreApplication>
#include <QThread>

#include <GCF3/Log>
#include <GCF3/Application>

int main(int argc, char **argv)
{
    GCF::Application app(argc, argv);

    // TODO : Perhaps we should allow this file to be elsewhere (I wouldn't like that)
    QString settingsFile = app.applicationDirPath() + "/FiberConfig/FiberMain.ini";
    FiberConfigurationSettings::instance()->setConfigurationFile(settingsFile);

    FiberControlCentre::instance();

    FiberServer *server = new FiberServer;

    QThread thread;
    server->moveToThread(&thread);
    thread.start(QThread::TimeCriticalPriority);

    QObject::connect(&thread, SIGNAL(finished()), server, SLOT(deleteLater()));
    QObject::connect(&thread, SIGNAL(finished()), &app, SLOT(quit()));

    return app.exec();
}
