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

#ifndef QMLAPPLICATION_H
#define QMLAPPLICATION_H

#include "QuickCommon.h"
#include "../Core/Application.h"

#include <QGuiApplication>

class QQmlEngine;

#ifdef QAPPLICATION_H
#define QtApplicationClass QApplication
#else
#define QtApplicationClass QGuiApplication
#endif

namespace GCF
{

struct QmlApplicationServicesData;
class GCF_QUICK_EXPORT QmlApplicationServices : public GCF::ApplicationServices
{
public:
    static QmlApplicationServices *instance();

    void setQmlEngine(QQmlEngine *engine);
    QQmlEngine *qmlEngine() const;

protected:
    QmlApplicationServices();
    ~QmlApplicationServices();
    virtual void initQmlEngine(QQmlEngine *) { }
    Q_DISABLE_COPY(QmlApplicationServices)

private:
    QmlApplicationServicesData *d;
};

GCF_INTERFACE_BEGIN

class QmlApplication : public QtApplicationClass, public QmlApplicationServices
{
public:
    QmlApplication(int &argc, char **argv)
        : QtApplicationClass(argc, argv) { }
    ~QmlApplication() { }

    int processArgumentsAndExec(const QStringList &additionalArgs=QStringList()) {
        this->processArguments(additionalArgs);
        return this->exec();
    }
};

GCF_INTERFACE_END

}

#ifdef gApp
#undef gApp
#endif

inline GCF::QmlApplication *qmlApplicationInstance() {
    return dynamic_cast<GCF::QmlApplication*>(qApp);
}

#define gApp qmlApplicationInstance()

#endif // QMLAPPLICATION_H
