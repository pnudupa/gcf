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

#include "SpiralFlowComponent.h"

#include <QQmlEngine>
#include <QQmlComponent>

struct SpiralFlowComponentData
{
    SpiralFlowComponentData() : qmlEngine(0), spiralFlowItem(0) { }

    QQmlEngine *qmlEngine;
    QObject *spiralFlowItem;

    QObject *loadQml(const QUrl &url) {
        if(!qmlEngine)
            return 0;

        QQmlComponent *qmlComp = new QQmlComponent(qmlEngine, qmlEngine);
        qmlComp->loadUrl(url);
        QObject *qmlObj = qmlComp->create();
        return qmlObj;
    }
};

SpiralFlowComponent::SpiralFlowComponent()
{
    d = new SpiralFlowComponentData;
}

SpiralFlowComponent::~SpiralFlowComponent()
{
    delete d;
}

void SpiralFlowComponent::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
    {
        e->setContentFile(":/Content/SpiralFlow.xml");

        d->qmlEngine = gFindObject<QQmlEngine>();
        Q_ASSERT(d->qmlEngine != 0);
    }

    GCF::GuiComponent::contentLoadEvent(e);
}

QObject *SpiralFlowComponent::loadObject(const QString &name, const QVariantMap &info)
{
    if(name == "spiralFlowView")
    {
        if(!d->spiralFlowItem)
            d->spiralFlowItem = d->loadQml(QUrl("qrc:///Qml/FolderSpiralFlowView.qml"));
        return d->spiralFlowItem;
    }

    return GCF::GuiComponent::loadObject(name, info);
}

GCF_EXPORT_COMPONENT(SpiralFlowComponent)
