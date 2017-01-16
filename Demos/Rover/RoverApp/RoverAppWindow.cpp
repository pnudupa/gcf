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

#include "RoverAppWindow.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QQuickView>
#include <QQmlContext>
#include <QFileSystemModel>

#include "ContentItemModel.h"
#include "FolderContentModel.h"
#include "ThumbnailImageProvider.h"

struct RoverAppWindowData
{
    QFileSystemModel *fsModel;
    FolderContentModel *folderContentModel;
    ContentItemModel *contentItemModel;

    QTreeView *treeView;
    QWidget *qmlViewWidget;
    QQuickView *qmlView;
};

RoverAppWindow::RoverAppWindow(QWidget *parent) :
    QWidget(parent)
{
    d = new RoverAppWindowData;

    this->setLayout(new QHBoxLayout);

    QSplitter *splitter = new QSplitter(this);
    this->layout()->addWidget(splitter);

    d->fsModel = new QFileSystemModel(this);
    d->fsModel->setRootPath(QDir::homePath());

    d->folderContentModel = new FolderContentModel(this);
    d->folderContentModel->setFolder(QDir::homePath());
    connect(d->folderContentModel, SIGNAL(folderChanged(QString)), this, SLOT(onFolderChanged(QString)));

    d->contentItemModel = new ContentItemModel(this);

    QModelIndex home = d->fsModel->index(QDir::homePath());

    d->treeView = new QTreeView(splitter);
    d->treeView->setModel(d->fsModel);
    d->treeView->setRootIndex(home);
    d->treeView->setMaximumWidth(350);
    d->treeView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    for(int i=1; i<d->fsModel->columnCount(); i++)
        d->treeView->setColumnHidden(i, true);
    connect(d->treeView, SIGNAL(activated(QModelIndex)), this, SLOT(onTreeViewActivated(QModelIndex)));

    d->qmlView = new QQuickView;
    d->qmlViewWidget = QWidget::createWindowContainer(d->qmlView, splitter);

    d->qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
    d->qmlView->rootContext()->setContextProperty("folderContentModel", d->folderContentModel);
    d->qmlView->rootContext()->setContextProperty("contentItemModel", d->contentItemModel);
    d->qmlView->engine()->addImageProvider(QLatin1String("thumbnail"), new ThumbnailImageProvider);
    d->qmlView->setSource(QUrl("qrc:///Qml/ContentView.qml"));

    d->qmlViewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

RoverAppWindow::~RoverAppWindow()
{
    delete d;
}

QQmlEngine *RoverAppWindow::contentViewQmlEngine() const
{
    return d->qmlView->engine();
}

ContentItemModel *RoverAppWindow::contentItemModel() const
{
    return d->contentItemModel;
}

void RoverAppWindow::onTreeViewActivated(const QModelIndex &index)
{
    d->folderContentModel->setFolder( d->fsModel->fileInfo(index).absoluteFilePath() );
}

void RoverAppWindow::onFolderChanged(const QString &folder)
{
    QModelIndex index = d->fsModel->index(folder);
    d->treeView->setCurrentIndex(index);
}
