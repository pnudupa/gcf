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

#include "FileBrowserComponent.h"

#include <QTreeView>
#include <QHeaderView>
#include <QFileSystemModel>

struct FileBrowserComponentData
{
    FileBrowserComponentData() :
        fileSystemModel(0)
        {}

    QFileSystemModel *fileSystemModel;
};

FileBrowserComponent::FileBrowserComponent(QObject *parent) :
     GCF::GuiComponent(parent)
{
    d = new FileBrowserComponentData;
    d->fileSystemModel = new QFileSystemModel;
}

FileBrowserComponent::~FileBrowserComponent()
{
    delete d;
}

void FileBrowserComponent::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
        e->setContentFile(":/FileBrowserComponent/FileBrowserComponent.xml");

    GCF::GuiComponent::contentLoadEvent(e);
}

QWidget *FileBrowserComponent::loadWidget(const QString &name, const QVariantMap &info)
{
    Q_UNUSED(info);

    if(name == "FileBrowser")
    {
        QTreeView *view = new QTreeView;

        d->fileSystemModel->setRootPath(QDir::homePath());
        view->setModel(d->fileSystemModel);
        QModelIndex index = d->fileSystemModel->index(QDir::homePath());
        view->setRootIndex(index);
        view->hideColumn(1);
        view->hideColumn(2);
        view->hideColumn(3);
        view->header()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
        view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
        view->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
        connect(view, SIGNAL(activated(QModelIndex)),
                this, SLOT(onTreeViewActivated(QModelIndex)));

        return view;
    }

    return GCF::GuiComponent::loadWidget(name, info);
}

void FileBrowserComponent::onTreeViewActivated(const QModelIndex &index)
{
    QFileInfo fileInfo = d->fileSystemModel->filePath(index);

    if(fileInfo.isFile())
        emit fileDoubleClicked(fileInfo.absoluteFilePath());
}

