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

#include "RoverApp.h"
#include "RoverAppWindow.h"
#include "ContentItemModel.h"

#include <QQmlEngine>
#include <QQmlComponent>

struct RoverAppData
{
    RoverAppData() : listView(0), gridView(0) { }

    RoverAppWindow *window;

    QObject *listView;
    QObject *gridView;

    QObject *loadQml(const QUrl &url) {
        QQmlEngine *qmlEngine = this->window->contentViewQmlEngine();
        QQmlComponent *qmlComp = new QQmlComponent(qmlEngine, qmlEngine);
        qmlComp->loadUrl(url);
        QObject *qmlObj = qmlComp->create();
        return qmlObj;
    }
};

RoverApp::RoverApp()
{
    d = new RoverAppData;
}

RoverApp::~RoverApp()
{
    delete d;
}

void RoverApp::initializeEvent(GCF::InitializeEvent *e)
{
    if(e->isPreInitialize())
        d->window = new RoverAppWindow;
    else if(e->isPostInitialize())
        d->window->showMaximized();

    GCF::GuiComponent::initializeEvent(e);
}

void RoverApp::finalizeEvent(GCF::FinalizeEvent *e)
{
    if(e->isPreFinalize())
        d->window->hide();

    GCF::GuiComponent::finalizeEvent(e);
}

void RoverApp::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
        e->setContentFile(":/Content/RoverApp.xml");

    GCF::GuiComponent::contentLoadEvent(e);
}

QObject *RoverApp::loadObject(const QString &name, const QVariantMap &info)
{
    if(name == "window")
        return d->window;

    if(name == "contentItemModel")
        return d->window->contentItemModel();

    if(name == "contentViewQmlEngine")
        return d->window->contentViewQmlEngine();

    if(name == "listView")
    {
        if(!d->listView)
            d->listView = d->loadQml(QUrl("qrc:///Qml/FolderListView.qml"));
        return d->listView;
    }

    if(name == "gridView")
    {
        if(!d->gridView)
            d->gridView = d->loadQml(QUrl("qrc:///Qml/FolderGridView.qml"));
        return d->gridView;
    }

    return GCF::GuiComponent::loadObject(name, info);
}

bool RoverApp::mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);

    if(parent == d->window->contentItemModel())
    {
        QString title = childInfo.value("title").toString();
        d->window->contentItemModel()->addContentItem(child, title);
        return true;
    }

    return GCF::GuiComponent::mergeObject(parent, child, parentInfo, childInfo);
}

bool RoverApp::unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if(parent == d->window->contentItemModel())
    {
        d->window->contentItemModel()->removeContentItem(child);
        return true;
    }

    return GCF::GuiComponent::unmergeObject(parent, child, parentInfo, childInfo);
}
