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
#include <GCF3/Log>

#include <QFile>

#include "TestClient.h"

const int ServerPort = 49772;

int main(int argc, char **argv)
{
    GCF::Application a(argc, argv);

    GCF::Log::instance()->setLogFileName("IpcLog.txt");
    QFile::remove( GCF::Log::instance()->logFileName() );

    int index = a.arguments().indexOf("--function");
    QString fn = a.arguments().value(index+1);

    TestClient client(fn, ::ServerPort);

    int retVal = a.exec();

    if(retVal)
    {
        QFile file( GCF::Log::instance()->logFileName() );
        file.open( QFile::ReadOnly );
        QString log = file.readAll();
        file.close();

        qDebug("%s\n\nFAIL", qPrintable(log));
    }
    else
        qDebug() << "SUCCESS";

    return retVal;
}


