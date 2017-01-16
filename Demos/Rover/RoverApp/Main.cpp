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

#include <GCF3/GuiApplication>
#include "RoverApp.h"

int main(int argc, char **argv)
{
    GCF::GuiApplication a(argc, argv);

    RoverApp *roverApp = new RoverApp;
    roverApp->load();

    a.loadComponent("RoverComponents/CoverFlowView");
    a.loadComponent("RoverComponents/SpiralFlowView");

    return a.processArgumentsAndExec();
}
