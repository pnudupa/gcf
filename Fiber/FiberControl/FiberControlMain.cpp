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

#include "Json.h"

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStringList>
#include <QDebug>
#include <QProcess>

bool startFiber();
bool stopFiber();
bool isFiberUp();

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if(app.arguments().count() != 2)
    {
        qDebug() << "\nInsufficient number of parameters."
                    "\nUsage : FiberCtl <start>|<stop>";
        return -1;
    }

    QString controlCommand = app.arguments().last();
    controlCommand = controlCommand.toLower();

    if( (controlCommand != "start") &&
        (controlCommand != "stop") &&
        (controlCommand != "status") )
    {
        qDebug() << "\nUnrecognized parameter " + controlCommand +
                    "\nUsage : FiberCtl <start>|<stop>|<status>";
        return -1;
    }

    if(controlCommand == "status")
    {
        if(isFiberUp())
            qDebug() << "\nSTATUS: Fiber is running!";
        else
            qDebug() << "\nSTATUS: Fiber is not running!";
    }
    else if(controlCommand == "start")
    {
        if(isFiberUp())
        {
            qDebug() << "\nERROR: Fiber is already running!";
            return -1;
        }

        bool success = startFiber();
        if(!success)
        {
            qDebug() << "\nERROR: Couldn't start Fiber!";
            return -1;
        }

        qDebug() << "\nSUCCESS: Fiber is running now!";
    }
    else if(controlCommand == "stop")
    {
        if(!isFiberUp())
        {
            qDebug() << "\nERROR: Fiber is not running!";
            return -1;
        }

        bool success = stopFiber();
        if(!success)
        {
            qDebug() << "\nERROR: Couldn't stop Fiber!";
            return -1;
        }

        qDebug() << "\nSUCCESS: Fiber shut down!";
    }

    return 0;
}

bool startFiber()
{
    if(isFiberUp())
        return false;

    if(!QProcess::startDetached("Fiber"))
    {
        qDebug() << "FATAL: Couldn't launch fiber";
        return false;
    }

    return true;
}

bool stopFiber()
{
    if(!isFiberUp())
        return false;

    QVariantMap jObj;
    jObj["requestType"] = "FIBER_CONTROL";
    jObj["controlCommand"] = "terminate";
    QByteArray jsonData = Json().serialize(jObj);

    QLocalSocket fiberSocket;
    fiberSocket.connectToServer("Fiber");
    fiberSocket.write(jsonData);
    fiberSocket.flush();
    fiberSocket.waitForBytesWritten();
    fiberSocket.disconnect();

    return true;
}

bool isFiberUp()
{
    QLocalSocket fiberSocket;
    fiberSocket.connectToServer("FiberControl");
    return fiberSocket.waitForConnected();
}
