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

#ifndef SIMPLECOMPONENT_H
#define SIMPLECOMPONENT_H

#include "TestService.h"

#include <GCF3/Component>
#include <GCF3/Application>

class SimpleCoreComponent : public GCF::Component
{
    Q_OBJECT

public:
    SimpleCoreComponent(QObject *parent=0)
        : GCF::Component(parent) {
        m_testService = 0;
    }

    Q_INVOKABLE void dummyService() { }

protected:
    void initializeEvent(GCF::InitializeEvent *e) {
        if(e->isPreInitialize())
            return;

        GCF::ObjectTreeNode *node = gAppService->objectTree()->node(m_testService);
        if(node)
            node->writableInfo()["allowmetaaccess"] = true;
    }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/Content/SimpleCoreComponent.xml");
        GCF::Component::contentLoadEvent(e);
    }

    QObject *loadObject(const QString &name, const QVariantMap &) {
        if(name == "Core")
            return this;

        if(name == "TestService") {
            if(!m_testService)
                m_testService = new TestService(this);

            return m_testService;
        }

        return 0;
    }

private:
    TestService *m_testService;
};

#endif // SIMPLECOMPONENT_H
