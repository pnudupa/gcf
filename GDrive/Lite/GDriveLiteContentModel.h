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

#ifndef GDRIVELITECONTENTMODEL_H
#define GDRIVELITECONTENTMODEL_H

#include <QAbstractItemModel>
#include "IGDriveLiteContentModel.h"
#include "GDriveLite.h"

class GDriveLiteFileUploader;
class GDriveLiteContentNodeObject;

struct GDriveLiteContentModelData;
class GDriveLiteContentModel : public GCF::IGDriveLiteContentModel
{
    Q_OBJECT
    Q_INTERFACES(GCF::IGDriveLiteContentModel)

public:
    GDriveLiteContentModel(GDriveLite *parent = 0);
    ~GDriveLiteContentModel();

    GDriveLite *gDriveLite() const;

    void setContentVisibility(int flags);
    int contentVisibility() const;
    void setColumns(const QList<Field> &fields);
    QList<Field> columns() const;
    GCF::Result refresh(const QModelIndex &index=QModelIndex());
    GCF::GDriveContent::Item item(const QModelIndex &index) const;
    GCF::GDriveContent::Item item(const QString &id) const;
    QModelIndex indexForId(const QString &id, int column=0) const;
    QModelIndexList indexListForId(const QString &id, int column=0) const;
    QModelIndex specialItemIndex(SpecialItem item) const;
    Q_SIGNAL void fileItemLoaded(const QModelIndex &index);
    Q_SIGNAL void folderItemLoaded(const QModelIndex &index);

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QHash<int,QByteArray> roleNames() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void onAuthenticatedUserChanged();
    void onContentNodeObjectDestroyed(GCF::ObjectTreeNode *node);
    void onIconLoaded(const QString &url, const QIcon &icon);

private:
    void clear();
    void fetchFilesList();
    QModelIndex index(GCF::ObjectTreeNode *node, int column=0) const;
    QIcon itemIcon(GCF::ObjectTreeNode *node, const GCF::GDriveContent::Item &item) const;
    void addItem(const QString &folderId, const GCF::GDriveContent::Item &item);
    void removeItem(const QString &itemId, const QString &folderId=QString());
    void changeItem(const QString &itemId, const GCF::GDriveContent::Item &item);

private:
    friend class GDriveLiteContentNodeObject;
    friend class GDriveLiteFileUploader;
    friend class GDriveLite;
    GDriveLiteContentModelData *d;
};

#endif // GDRIVELITECONTENTMODEL_H
