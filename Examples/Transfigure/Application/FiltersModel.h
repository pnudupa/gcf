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

#ifndef FILTERCOMPONENTMODEL_H
#define FILTERCOMPONENTMODEL_H

#include <QAbstractItemModel>
#include <GCF3/Application>
#include <GCF3/ObjectTree>
#include <GCF3/Component>

#include "IImageFilter.h"

struct FilterInfo
{
    FilterInfo() : Filter(0) { }

    QString ComponentName;
    QString FilterName;
    IImageFilter *Filter;
};

class FiltersModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum filterComponentRole
    {
        FilterGroup = Qt::UserRole,
        FilterName = Qt::DisplayRole
    };

    explicit FiltersModel(QObject *parent=0)
        : QAbstractListModel(parent) {
        connect( gAppService->objectTree(), SIGNAL(nodeAdded(GCF::ObjectTreeNode*,GCF::ObjectTreeNode*)),
                 this, SLOT(onNodeAdded(GCF::ObjectTreeNode*,GCF::ObjectTreeNode*)));
    }
    ~FiltersModel() { }

    QImage applyFilterAt(int index, const QImage &image) {
        if(index < 0 || index >= m_filterInfoList.count())
            return image;

        FilterInfo info = m_filterInfoList.at(index);
        return info.Filter->applyFilterEffect( info.FilterName, image );
    }

    int columnCount(const QModelIndex &parent=QModelIndex()) const {
        Q_UNUSED(parent);
        return 1;
    }

    int rowCount(const QModelIndex &parent=QModelIndex()) const {
        if(parent.isValid())
            return 0;
        return m_filterInfoList.count();
    }

    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const {
        if(index.row() < 0 || index.row() >= m_filterInfoList.count())
            return QVariant();

        FilterInfo info = m_filterInfoList.at(index.row());
        switch(role) {
        case FilterGroup: return info.ComponentName;
        case FilterName: return info.FilterName;
        default: break;
        }

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const {
        QHash<int,QByteArray> roles;
        roles[FilterGroup] = "filterGroup";
        roles[FilterName] = "filterName";
        return roles;
    }

public slots:
    void onNodeAdded(GCF::ObjectTreeNode *parent, GCF::ObjectTreeNode *child) {
        GCF::Component *component = qobject_cast<GCF::Component*>(child->object());
        if(!component)
            component = qobject_cast<GCF::Component*>(parent->object());
        QString componentName = component ? component->name() : QString();
        QObject *object = child->object();
        IImageFilter *filter = qobject_cast<IImageFilter*>(object);
        if(filter) {
            QStringList names = filter->effectNames();
            this->beginInsertRows(QModelIndex(), m_filterInfoList.count(),
                                  m_filterInfoList.count()+names.count()-1);
            Q_FOREACH(QString name, names) {
                FilterInfo info;
                info.ComponentName = componentName;
                info.FilterName = name;
                info.Filter = filter;
                m_filterInfoList.append(info);
            }
            this->endInsertRows();
        }
    }

    // We assume that components dont get unloaded during the life-time of
    // the application

private:
    QList<FilterInfo> m_filterInfoList;
};

#endif // FILTERCOMPONENTMODEL_H
