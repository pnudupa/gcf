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

#ifndef GDRIVELITECONTENTSHARINGMODEL_H
#define GDRIVELITECONTENTSHARINGMODEL_H

#include "IGDriveLiteContentSharingModel.h"
#include "GDriveLite.h"

struct GDriveLiteContentSharingModelData;
class GDriveLiteContentSharingModel : public GCF::IGDriveLiteContentSharingModel
{
    Q_OBJECT

public:
    GDriveLiteContentSharingModel(const QString &itemId, GDriveLite *parent);
    ~GDriveLiteContentSharingModel();

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QString contentItemId() const;
    GCF::GDriveContent::Item contentItem() const;
    Q_INVOKABLE QUrl shareLink() const;
    void setColumns(const QList<int> &fields);
    QList<int> columns() const;
    GCF::GDriveContent::Permission permission(const QModelIndex &index) const;
    Q_INVOKABLE GCF::Result share(const GCF::GDriveLiteShareRequest &request);
    Q_INVOKABLE GCF::Result unshare(const QModelIndex &index);
    Q_INVOKABLE GCF::Result modifyShareRole(const QModelIndex &index, int role);
    Q_SIGNAL void modified();
    Q_SIGNAL void error(const QString &msg);

private:
    Q_SLOT void onFileGetApiDone(const QVariant &v, const GCF::Result &result);
    Q_SLOT void onPermissionsListApiDone(const QVariant &v, const GCF::Result &result);
    void initialize();

private:
    GDriveLiteContentSharingModelData *d;
};

#endif // GDRIVELITECONTENTSHARINGMODEL_H
