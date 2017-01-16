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

#ifndef GDRIVECONTENTLISTMODEL_P_H
#define GDRIVECONTENTLISTMODEL_P_H

#include <QAbstractItemModel>
#include <GCF3/IGDriveLiteContentModel>

class GDriveContentItemPropertiesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    GDriveContentItemPropertiesModel(QObject *parent=0)
        : QAbstractListModel(parent) { }
    ~GDriveContentItemPropertiesModel() { }

    void setContentItem(const GCF::GDriveContent::Item &item) {
        this->beginResetModel();
        m_item = item;
        this->endResetModel();
    }

    GCF::GDriveContent::Item contentItem() const { return m_item; }

    int rowCount(const QModelIndex &parent) const {
        if(parent.isValid())
            return 0;
        return m_item.data().count();
    }
    int columnCount(const QModelIndex &parent) const {
        if(parent.isValid())
            return 0;
        return 1;
    }

    enum Fields
    {
        KeyField,
        ValueField
    };
    QHash<int,QByteArray> roleNames() const {
        static QHash<int,QByteArray> roles;
        if(roles.isEmpty()) {
            roles[KeyField] = "propertyKey";
            roles[ValueField] = "propertyValue";
        }
        return roles;
    }

    QVariant data(const QModelIndex &index, int role) const {
        if(!index.isValid() || index.row() < 0 || index.row() >= m_item.data().count())
            return QVariant();

        QStringList keys = m_item.data().keys();
        QString key = keys.at(index.row());
        if(role == KeyField)
            return key;
        if(role == ValueField) {
            QVariant value = m_item.data().value(key);
            if(value.type() == QVariant::List)
                return "Array";
            if(value.type() == QVariant::Map)
                return "Object";
            return value.toString();
        }

        return QVariant();
    }

private:
    GCF::GDriveContent::Item m_item;
};

#endif // GDRIVECONTENTLISTMODEL_P_H
