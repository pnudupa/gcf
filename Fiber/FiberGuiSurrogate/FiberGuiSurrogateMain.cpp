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

#include "FiberSurrogateServer.h"
#include "FiberConfigurationSettings.h"

#include <GCF3/GuiApplication>

int main(int argc, char **argv)
{
    GCF::GuiApplication app(argc, argv);

    if(argc != 3)
    {
        qDebug() << "\nIncorrect parameters!";
        return -1;
    }

    // TODO : Perhaps we should allow this file to be elsewhere (I wouldn't like that)
    QString settingsFile = app.applicationDirPath() + "/FiberConfig/FiberGuiSurrogate.ini";
    FiberConfigurationSettings::instance()->setConfigurationFile(settingsFile);

    FiberSurrogateServer server(argv[1], argv[2]);

    return app.exec();
}
