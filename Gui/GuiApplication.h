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

#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include "GuiCommon.h"
#include "../Core/Application.h"

#include <QApplication>

namespace GCF
{

class GCF_GUI_EXPORT GuiApplication : public QApplication,
                                      public GCF::ApplicationServices
{
    Q_OBJECT

public:
    static GuiApplication *instance();
    GuiApplication(int &argc, char **argv);
    ~GuiApplication();

    int processArgumentsAndExec(const QStringList &additionalArgs=QStringList()) {
        this->processArguments(additionalArgs);
        return this->exec();
    }

private slots:
    void onAppAboutToQuit();
};

}

#ifdef gApp
#undef gApp
#endif
#define gApp GCF::GuiApplication::instance()

#endif // GUIAPPLICATION_H
