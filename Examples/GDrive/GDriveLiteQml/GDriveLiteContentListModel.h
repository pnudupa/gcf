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

#ifndef GDRIVECONTENTLISTMODEL_H
#define GDRIVECONTENTLISTMODEL_H

#include <QAbstractListModel>
#include <GCF3/IGDriveLiteContentModel>

/**
 * The function of this class is to expose the tree structure
 * of GCF::IGDriveLiteContentModel as a list structure,
 * one hierarchy at a time.
 */

struct GDriveLiteContentListModelData;
class GDriveLiteContentListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    GDriveLiteContentListModel(GCF::IGDriveLiteContentModel *parent);
    ~GDriveLiteContentListModel();

    GCF::IGDriveLiteContentModel *gDriveContentModel() const;

    void setRootIndex(const QModelIndex &index);
    void setTopmostIndex(const QModelIndex &index);
    QModelIndex rootIndex() const;

    void setRootIndex(GCF::IGDriveLiteContentModel::SpecialItem specialItem) {
        QModelIndex idx = this->gDriveContentModel()->specialItemIndex(specialItem);
        this->setRootIndex(idx);
    }

    void setTopmostIndexAsMyDrive() {
        QModelIndex idx = this->gDriveContentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive);
        this->setTopmostIndex(idx);
    }

    Q_INVOKABLE void setRootIndexAsMyDrive() {
        this->setRootIndex(GCF::IGDriveLiteContentModel::MyDrive);
    }
    Q_INVOKABLE void setRootIndexAsSharedWithMe() {
        this->setRootIndex(GCF::IGDriveLiteContentModel::SharedWithMe);
    }
    Q_INVOKABLE void setRootIndexAsTrash() {
        this->setRootIndex(GCF::IGDriveLiteContentModel::Trash);
    }
    Q_INVOKABLE void setRootIndexAsStarred() {
        this->setRootIndex(GCF::IGDriveLiteContentModel::Starred);
    }

    Q_INVOKABLE bool cd(int index, bool isFolderView = false); // shortcut
    Q_INVOKABLE bool cdUp();
    Q_INVOKABLE QString getItemId(const int &index);
    Q_INVOKABLE QModelIndex getItemIndex(const int &index);
    Q_INVOKABLE QString getItemIdForSpecialItem(const int &index);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Q_PROPERTY(QObject *contentItemProperties READ contentItemProperties CONSTANT)
    QObject *contentItemProperties() const;
    Q_INVOKABLE void setContentItem(int row);

private:
    Q_SLOT void onBeginReset() { this->beginResetModel(); }
    Q_SLOT void onEndReset() { this->endResetModel(); }
    Q_SLOT void onRowsAboutToBeInserted(const QModelIndex &index, int start, int end) {
        if(index == this->rootIndex())
            this->beginInsertRows(QModelIndex(), start, end);
    }
    Q_SLOT void onRowsInserted(const QModelIndex &index, int, int) {
        if(index == this->rootIndex())
            this->endInsertRows();
    }
    Q_SLOT void onRowsAboutToBeRemoved(const QModelIndex &index, int start, int end) {
        if(index == this->rootIndex())
            this->beginRemoveRows(QModelIndex(), start, end);
    }
    Q_SLOT void onRowsRemoved(const QModelIndex &index, int, int) {
        if(index == this->rootIndex())
            this->endRemoveRows();
    }
    Q_SLOT void onDataChanged(const QModelIndex &start, const QModelIndex &end);

private:
    GDriveLiteContentListModelData *d;
};

#endif // GDRIVECONTENTLISTMODEL_H
