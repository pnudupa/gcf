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

#include "CoverFlowComponent.h"

#include <QQmlEngine>
#include <QQmlComponent>

struct CoverFlowComponentData
{
    CoverFlowComponentData() : qmlEngine(0), coverFlowItem(0) { }

    QQmlEngine *qmlEngine;
    QObject *coverFlowItem;

    QObject *loadQml(const QUrl &url) {
        if(!qmlEngine)
            return 0;

        QQmlComponent *qmlComp = new QQmlComponent(qmlEngine, qmlEngine);
        qmlComp->loadUrl(url);
        QObject *qmlObj = qmlComp->create();
        return qmlObj;
    }
};

CoverFlowComponent::CoverFlowComponent()
{
    d = new CoverFlowComponentData;
}

CoverFlowComponent::~CoverFlowComponent()
{
    delete d;
}

void CoverFlowComponent::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
    {
        e->setContentFile(":/Content/CoverFlow.xml");

        d->qmlEngine = gFindObject<QQmlEngine>();
        Q_ASSERT(d->qmlEngine != 0);
    }

    GCF::GuiComponent::contentLoadEvent(e);
}

QObject *CoverFlowComponent::loadObject(const QString &name, const QVariantMap &info)
{
    if(name == "coverFlowView")
    {
        if(!d->coverFlowItem)
            d->coverFlowItem = d->loadQml(QUrl("qrc:///Qml/FolderCoverFlowView.qml"));
        return d->coverFlowItem;
    }

    return GCF::GuiComponent::loadObject(name, info);
}

GCF_EXPORT_COMPONENT(CoverFlowComponent)
