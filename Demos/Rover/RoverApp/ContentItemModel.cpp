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

#include "ContentItemModel.h"
#include "ThumbnailImageProvider.h"
#include <QStringList>

struct ContentItemModelData
{
    GCF::ObjectList contentItemList;
    QStringList contentTitleList;
};

ContentItemModel::ContentItemModel(QObject *parent) :
    QAbstractListModel(parent)
{
    d = new ContentItemModelData;
    d->contentItemList.setEventListener(this);
}

ContentItemModel::~ContentItemModel()
{
    delete d;
}

void ContentItemModel::addContentItem(QObject *object, const QString &title)
{
    this->beginInsertRows(QModelIndex(), d->contentItemList.count(), d->contentItemList.count());
    d->contentItemList.add(object);
    d->contentTitleList.append(title);
    this->endInsertRows();
}

void ContentItemModel::removeContentItem(QObject *object)
{
    d->contentItemList.remove(object);
}

QHash<int,QByteArray> ContentItemModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[ContentItemRole] = "contentItem";
    roles[ContentTitleRole] = "contentTitle";
    return roles;
}

int ContentItemModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return d->contentItemList.count();
}

int ContentItemModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return 1;
}

QVariant ContentItemModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case ContentItemRole: return QVariant::fromValue<QObject*>(d->contentItemList.at(index.row()));
    case ContentTitleRole: return d->contentTitleList.at(index.row());
    default: break;
    }

    return QVariant();
}

void ContentItemModel::objectRemoved(int index, QObject *obj)
{
    Q_UNUSED(obj);

    this->beginRemoveRows(QModelIndex(), index, index);
    d->contentTitleList.removeAt(index);
    this->endRemoveRows();
}
