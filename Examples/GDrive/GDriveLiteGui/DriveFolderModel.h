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

#ifndef DRIVEFOLDERMODEL_H
#define DRIVEFOLDERMODEL_H

#include <QIdentityProxyModel>
#include <GCF3/IGDriveLite>

class DriveFolderModel : public QIdentityProxyModel
{
public:
    DriveFolderModel(GCF::IGDriveLite *parent=0) : QIdentityProxyModel(parent) {
        m_contentModel = parent->contentModel();
        this->setSourceModel(parent->contentModel());
    }
    ~DriveFolderModel() { }

    QString selectedFolderId() const { return m_selectedFolderId; }

    QModelIndex specialItemIndex(GCF::IGDriveLiteContentModel::SpecialItem item) const {
        QModelIndex sourceIndex = m_contentModel->specialItemIndex(item);
        return this->mapFromSource(sourceIndex);
    }

    int columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &proxyIndex, int role) const {
        QModelIndex sourceIndex = this->mapToSource(proxyIndex);
        if( role == Qt::CheckStateRole &&
                sourceIndex.data(GCF::IGDriveLiteContentModel::IsFolder).toBool() ) {
            if(m_selectedFolderId == sourceIndex.data(GCF::IGDriveLiteContentModel::Id).toString())
                return Qt::Checked;
            return Qt::Unchecked;
        }

        return sourceIndex.data(role);
    }

    bool setData(const QModelIndex &proxyIndex, const QVariant &value, int role) {
        if( role == Qt::CheckStateRole ) {
            QModelIndex sourceIndex = this->mapToSource(proxyIndex);

            // Uncheck the previously selected folder if any.
            if(sourceIndex.data(GCF::IGDriveLiteContentModel::Id).toString() != m_selectedFolderId) {
                QModelIndex selectedFolderSourceIndex = m_contentModel->indexForId(m_selectedFolderId);
                QModelIndex selectedFolderProxyIndex = this->mapFromSource(selectedFolderSourceIndex);
                m_selectedFolderId.clear();
                emit dataChanged(selectedFolderProxyIndex, selectedFolderProxyIndex);
            }

            // Now check the newly selected folder
            if(value.toBool())
                m_selectedFolderId = sourceIndex.data(GCF::IGDriveLiteContentModel::Id).toString();
            else
                m_selectedFolderId.clear();
            emit dataChanged(proxyIndex, proxyIndex);

            return true;
        }

        return false;
    }

    Qt::ItemFlags flags(const QModelIndex &proxyIndex) const {
        QModelIndex sourceIndex = this->mapToSource(proxyIndex);
        Qt::ItemFlags sourceFlags = this->sourceModel()->flags(sourceIndex);
        Qt::ItemFlags flags = 0;
        if(sourceIndex.data(GCF::IGDriveLiteContentModel::IsFolder).toBool())
            flags = sourceFlags | Qt::ItemIsUserCheckable;
        return flags;
    }

private:
    GCF::IGDriveLiteContentModel *m_contentModel;
    QString m_selectedFolderId;
};

#endif // DRIVEFOLDERMODEL_H
