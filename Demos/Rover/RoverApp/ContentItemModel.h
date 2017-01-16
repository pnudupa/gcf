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

#ifndef CONTENTITEMMODEL_H
#define CONTENTITEMMODEL_H

#include <QAbstractListModel>
#include <GCF3/ObjectList>

struct ContentItemModelData;
class ContentItemModel : public QAbstractListModel, public GCF::ObjectListEventListener
{
    Q_OBJECT

public:
    ContentItemModel(QObject *parent = 0);
    ~ContentItemModel();

    void addContentItem(QObject *object, const QString &title);
    void removeContentItem(QObject *object);

    enum Roles
    {
        ContentItemRole = Qt::UserRole,
        ContentTitleRole = Qt::DisplayRole
    };

protected:
    QHash<int,QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void objectRemoved(int index, QObject *obj);
    
private:
    ContentItemModelData *d;
};

#endif // CONTENTITEMMODEL_H
