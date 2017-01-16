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

#include "GuiApplication.h"

/**
\class GCF::GuiApplication GuiApplication.h <GCF3/GuiApplication>
\brief Extends \ref GCF::Application for GUI Applications
\ingroup gcf_gui


GUI applications are those that are built using \c QApplication on Qt 4 and
Qt 5. Specifically in Qt 5 this class should be used for creating a
widgets-only GUI application.

The \c gApp macro would point to an instance of this class when the header
file for this class is included.
*/

GCF::GuiApplication *GCF::GuiApplication::instance()
{
    return qobject_cast<GCF::GuiApplication*>(qApp);
}

/**
Constructor.

@param argc reference to an integer that holds number of command-line arguments
passed to the applicaiton.
@param argv pointer to an array of character strings that hold the command-line
arguments.
*/
GCF::GuiApplication::GuiApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAppAboutToQuit()));
}

/**
Destructor
*/
GCF::GuiApplication::~GuiApplication()
{

}

/**
\fn void GCF::GuiApplication::processArgumentsAndExec(const QStringList &additionalArgs=QStringList())

Process all arguments from \c qApp->arguments() and \c additionalArgs and invokes
\ref QApplication::exec().

\sa GCF::Application::processArgumentsAndExec()
 */

void GCF::GuiApplication::onAppAboutToQuit()
{
    GCF::LogMessageBranch branch("Application Shutdown");
    this->unloadAllComponents();
}
