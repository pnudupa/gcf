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

#ifndef IGDRIVELITECONTENTSHARINGMODEL_H
#define IGDRIVELITECONTENTSHARINGMODEL_H

#include "GDriveContent.h"
#include <QAbstractListModel>

namespace GCF
{

struct GDriveLiteShareRequest
{
    GDriveLiteShareRequest() : SendNotificationEMails(true), Role(0) { }
    QString EMailAddress;
    QString EMailMessage;
    bool SendNotificationEMails;
    int Role; // IGDriveLiteContentSharingModel::ShareRole
};

class IGDriveLiteContentSharingModel : public QAbstractListModel
{
public:
    ~IGDriveLiteContentSharingModel() { }

    virtual QString contentItemId() const = 0;
    virtual GCF::GDriveContent::Item contentItem() const = 0;
    Q_INVOKABLE virtual QUrl shareLink() const = 0;

    enum Field
    {
        Kind = Qt::UserRole+1,
        ETag,
        PermissionId,
        Id = PermissionId,
        Name,
        EMailAddress,
        Domain,
        RoleType, // Of type ShareRole
        RoleTypeStr,
        Title
    };

    enum ShareRole
    {
        Owner = 1,
        Reader = 2,
        Writer = 4,
        Commenter = 8,
        Unknown = 0
    };

    virtual void setColumns(const QList<int> &fields) = 0;
    virtual QList<int> columns() const = 0;
    virtual GCF::GDriveContent::Permission permission(const QModelIndex &index) const = 0;

    Q_INVOKABLE virtual GCF::Result share(const GCF::GDriveLiteShareRequest &request) = 0;
    Q_INVOKABLE virtual GCF::Result unshare(const QModelIndex &index) = 0;
    Q_INVOKABLE virtual GCF::Result modifyShareRole(const QModelIndex &index, int role) = 0;
    Q_SIGNAL virtual void modified() = 0;
    Q_SIGNAL virtual void error(const QString &msg) = 0;

protected:
    IGDriveLiteContentSharingModel(QObject *parent) : QAbstractListModel(parent) { }
};

}

#endif // IGDRIVELITECONTENTSHARINGMODEL_H
