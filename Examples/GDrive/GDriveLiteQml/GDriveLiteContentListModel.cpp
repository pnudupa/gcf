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

#include "GDriveLiteContentListModel.h"
#include "GDriveLiteContentListModel_p.h"

#include <QDebug>
#include <QTimer>

struct GDriveLiteContentListModelData
{
    GDriveLiteContentListModelData() : contentModel(0), contentItemIndex(-1) { }

    GCF::IGDriveLiteContentModel *contentModel;
    GDriveContentItemPropertiesModel contentItemPropertiesModel;
    QModelIndex rootIndex;
    QModelIndex topmostIndex;
    int contentItemIndex;
};

GDriveLiteContentListModel::GDriveLiteContentListModel(GCF::IGDriveLiteContentModel *parent)
    : QAbstractListModel(parent), d(new GDriveLiteContentListModelData)
{
#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    d->contentModel = parent;
    connect(d->contentModel, SIGNAL(modelAboutToBeReset()), this, SLOT(onBeginReset()));
    connect(d->contentModel, SIGNAL(modelReset()), this, SLOT(onEndReset()));
    connect(d->contentModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(d->contentModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(d->contentModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(d->contentModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(d->contentModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onDataChanged(QModelIndex,QModelIndex)));
}

GDriveLiteContentListModel::~GDriveLiteContentListModel()
{
    delete d;
}

GCF::IGDriveLiteContentModel *GDriveLiteContentListModel::gDriveContentModel() const
{
    return d->contentModel;
}

void GDriveLiteContentListModel::setRootIndex(const QModelIndex &index)
{
    if(d->rootIndex == index)
        return;

    if(index.isValid() && index.model() != d->contentModel)
        return;

    this->beginResetModel();
    d->rootIndex = index;
    d->contentItemIndex = -1;
    d->contentItemPropertiesModel.setContentItem(GCF::GDriveContent::Item());
    this->endResetModel();
}

void GDriveLiteContentListModel::setTopmostIndex(const QModelIndex &index)
{
    if(d->topmostIndex == index)
        return;

    if(index.isValid() && index.model() != d->contentModel)
        return;

    d->topmostIndex = index;
    setRootIndex(index);
}

QModelIndex GDriveLiteContentListModel::rootIndex() const
{
    return d->rootIndex;
}

bool GDriveLiteContentListModel::cd(int index, bool isFolderView)
{
    QModelIndex contentModelIndex = d->contentModel->index(index, 0, d->rootIndex);
    if( !contentModelIndex.isValid() )
        return false;

    if(isFolderView)
    {
        bool containsFolder = false;
        for ( int i=0; i <rowCount(contentModelIndex); i++ )
        {
            QModelIndex curIndex = this->index(i, 0, contentModelIndex);
            if(curIndex.isValid() && curIndex.data(GCF::IGDriveLiteContentModel::IsFolder).toBool())
            {
                containsFolder = true;
                break;
            }
        }

        if(!containsFolder)
            return false;
    }

    this->setRootIndex(contentModelIndex);
    return true;
}

bool GDriveLiteContentListModel::cdUp()
{
    if(d->topmostIndex.isValid() && d->topmostIndex == d->rootIndex)
        return false;

    QModelIndex contentModelIndex = d->rootIndex.parent();
    this->setRootIndex(contentModelIndex);
    return true;
}

QString GDriveLiteContentListModel::getItemId(const int &index)
{
    GCF::GDriveContent::Item item = d->contentModel->item(d->rootIndex.child(index, 0));
    if(item.isValid())
        return item.id();

    return QString();
}

QModelIndex GDriveLiteContentListModel::getItemIndex(const int &index)
{
    return d->contentModel->index(index, 0, d->rootIndex);
}

QString GDriveLiteContentListModel::getItemIdForSpecialItem(const int &index)
{
    QModelIndex curIndex = getItemIndex(index);
    if(curIndex.isValid())
        return curIndex.data(GCF::IGDriveLiteContentModel::Id).toString();

    return QString();
}

int GDriveLiteContentListModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return d->contentModel->rowCount(d->rootIndex);
}

int GDriveLiteContentListModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return d->contentModel->columnCount(d->rootIndex);
}

QVariant GDriveLiteContentListModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    QModelIndex contentModelIndex = d->contentModel->index(index.row(), index.column(), d->rootIndex);
    return contentModelIndex.data(role);
}

QHash<int,QByteArray> GDriveLiteContentListModel::roleNames() const
{
    return d->contentModel->roleNames();
}

QVariant GDriveLiteContentListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->contentModel->headerData(section, orientation, role);
}

QObject *GDriveLiteContentListModel::contentItemProperties() const
{
    return &d->contentItemPropertiesModel;
}

void GDriveLiteContentListModel::setContentItem(int row)
{
    QModelIndex contentModelIndex = d->contentModel->index(row, 0, d->rootIndex);
    GCF::GDriveContent::Item contentItem = d->contentModel->item(contentModelIndex);
    d->contentItemPropertiesModel.setContentItem(contentItem);
    d->contentItemIndex = row;
}

void GDriveLiteContentListModel::onDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    QModelIndex myStart = this->index(start.row(), start.column());
    QModelIndex myEnd = this->index(end.row(), end.column());
    emit dataChanged(myStart, myEnd);

    if(d->contentItemIndex >= start.row() && d->contentItemIndex <= end.row())
        this->setContentItem(d->contentItemIndex);
}
