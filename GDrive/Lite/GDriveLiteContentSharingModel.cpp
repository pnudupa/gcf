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

#include "GDriveLiteContentSharingModel.h"
#include "GDriveLiteAPI.h"

struct GDriveLiteContentSharingModelData
{
    GDriveLiteContentSharingModelData(const QString &itemId) : contentItemId(itemId) { }

    GDriveLite *gDriveLite;
    const QString contentItemId;
    GCF::GDriveContent::Item contentItem;
    QList<GCF::GDriveContent::Permission> contentPermissions;
    QList<int> columns;

    int shareRole(const QString &str) const {
        if(str == "owner") return GCF::IGDriveLiteContentSharingModel::Owner;
        if(str == "reader") return GCF::IGDriveLiteContentSharingModel::Reader;
        if(str == "writer") return GCF::IGDriveLiteContentSharingModel::Writer;
        if(str == "commenter") return GCF::IGDriveLiteContentSharingModel::Commenter;
        return GCF::IGDriveLiteContentSharingModel::Unknown;
    }
};

GDriveLiteContentSharingModel::GDriveLiteContentSharingModel(const QString &itemId, GDriveLite *parent)
    :GCF::IGDriveLiteContentSharingModel(parent), d(new GDriveLiteContentSharingModelData(itemId))
{
    d->gDriveLite = parent;
#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    this->initialize();

    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(modified()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(modified()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(modified()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(modified()));
}

GDriveLiteContentSharingModel::~GDriveLiteContentSharingModel()
{
    delete d;
}

int GDriveLiteContentSharingModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return d->contentPermissions.count();
}

int GDriveLiteContentSharingModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    if(d->columns.count())
        return d->columns.count();

    return 1;
}

QVariant GDriveLiteContentSharingModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(index.row() < 0 || index.row() >= d->contentPermissions.count())
        return QVariant();

    if(index.column() < 0 || index.column() >= this->columnCount())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        if(d->columns.count())
            role = d->columns.at(index.column());
        else
            role = Title;
    }

    GCF::GDriveContent::Permission permission = d->contentPermissions.at( index.row() );
    switch(role)
    {
    case Kind: return permission.kind();
    case ETag: return permission.etag();
    case PermissionId: return permission.id();
    case Name: return permission.name();
    case EMailAddress: return permission.emailAddress();
    case Domain: return permission.domain();
    case RoleType: {
        int ret = d->shareRole(permission.role());
        Q_FOREACH(QString arole, permission.additionalRoles())
            ret += d->shareRole(arole);
        return ret;
    } break;
    case RoleTypeStr: {
        QStringList ret;
        ret << permission.role();
        ret += permission.additionalRoles();
        return ret.join(", ");
    } break;
    case Title: return QString("%1 (%2)").arg(permission.name()).arg(permission.domain());
    default: break;
    }

    return QVariant();
}

QHash<int,QByteArray> GDriveLiteContentSharingModel::roleNames() const
{
    static QHash<int,QByteArray> roles;
    if(roles.isEmpty())
    {
        roles[Kind] = "shareKind";
        roles[ETag] = "shareETag";
        roles[PermissionId] = "sharePermissionId";
        roles[Name] = "shareUserName";
        roles[EMailAddress] = "shareEMailAddress";
        roles[Domain] = "shareDomain";
        roles[RoleType] = "shareRoleType";
        roles[RoleTypeStr] = "shareRoles";
        roles[Title] = "shareTitle";
    }

    return roles;
}

QVariant GDriveLiteContentSharingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(section >= this->columnCount())
        return QVariant();

    if(orientation == Qt::Horizontal)
    {
        if(d->columns.count() == 0)
            return "Title";

        QString header = this->roleNames().value( d->columns.at(section) );
        header.remove(0, 5);
        return header;
    }

    return QVariant();
}

QString GDriveLiteContentSharingModel::contentItemId() const
{
    return d->contentItemId;
}

GCF::GDriveContent::Item GDriveLiteContentSharingModel::contentItem() const
{
    return d->contentItem;
}

QUrl GDriveLiteContentSharingModel::shareLink() const
{
    QString urlStr = QString("https://docs.google.com/file/d/%1/edit?usp=sharing").arg(d->contentItemId);
    return QUrl(urlStr);
}

void GDriveLiteContentSharingModel::setColumns(const QList<int> &fields)
{
    if(d->contentItem.isValid() && d->contentPermissions.count())
    {
        this->beginResetModel();
        d->columns = fields;
        this->endResetModel();
    }
}

QList<int> GDriveLiteContentSharingModel::columns() const
{
    return d->columns;
}

GCF::GDriveContent::Permission GDriveLiteContentSharingModel::permission(const QModelIndex &index) const
{
    if(index.isValid() && index.row() < d->contentPermissions.count())
        return d->contentPermissions.at( index.row() );

    return GCF::GDriveContent::Permission();
}

GCF::Result GDriveLiteContentSharingModel::share(const GCF::GDriveLiteShareRequest &request)
{
    if(request.EMailAddress.isEmpty())
        return GCF::Result(false, QString(), "Email address has to be specified");

    if(request.Role == 0)
        return GCF::Result(false, QString(), "A share role has to be specified (as a reader, writer or commenter)");

    GDriveLiteAPI::Permissions::InsertRequest *api
            = new GDriveLiteAPI::Permissions::InsertRequest(d->gDriveLite, this);
    api->setFileId(d->contentItemId);
    api->setValue(request.EMailAddress);
    api->setEMailMessage(request.EMailMessage);
    api->setSendNotificationEMails(request.SendNotificationEMails);
    if(request.Role & Owner)
        api->setRole("owner");
    else if(request.Role & Writer)
        api->setRole("writer");
    else if(request.Role & Reader)
        api->setRole("reader");
    if(request.Role & Commenter)
        api->setAdditionalRoles( QStringList() << "commenter" );

    GCF::Result apiResult = api->blockingExecute();
    delete api;

    if(apiResult.isSuccess())
    {
        GCF::GDriveContent::Permission permission(apiResult.data().toMap());
        this->beginInsertRows(QModelIndex(), d->contentPermissions.count(), d->contentPermissions.count());
        d->contentPermissions.append(permission);
        this->endInsertRows();
        return true;
    }

    return apiResult;
}

GCF::Result GDriveLiteContentSharingModel::unshare(const QModelIndex &index)
{
    if(!index.isValid() || index.row() < 0 || index.row() >= d->contentPermissions.count())
        return GCF::Result(false, QString(), "Invalid or out-dated model-index passed");

    GDriveLiteAPI::Permissions::DeleteRequest *api
            = new GDriveLiteAPI::Permissions::DeleteRequest(d->gDriveLite, this);
    api->setFileId(d->contentItemId);
    api->setPermissionId(index.data(PermissionId).toString());

    GCF::Result apiResult = api->blockingExecute();
    delete api;

    if(apiResult.isSuccess())
    {
        int row = index.row();
        this->beginRemoveRows(QModelIndex(), row, row);
        d->contentPermissions.removeAt(row);
        this->endRemoveRows();

        return true;
    }

    return apiResult;
}

GCF::Result GDriveLiteContentSharingModel::modifyShareRole(const QModelIndex &index, int role)
{
    if(!index.isValid() || index.row() < 0 || index.row() >= d->contentPermissions.count())
        return GCF::Result(false, QString(), "Invalid or out-dated model-index passed");

    if(role == Unknown)
        return GCF::Result(false, QString(), "A share role has to be specified (as a reader, writer or commenter)");

    GDriveLiteAPI::Permissions::UpdateRequest *api
            = new GDriveLiteAPI::Permissions::UpdateRequest(d->gDriveLite, this);
    api->setFileId(d->contentItemId);
    api->setPermissionId(index.data(PermissionId).toString());
    if(role & Owner)
        api->setRole("owner");
    else if(role & Writer)
        api->setRole("writer");
    else if(role & Reader)
        api->setRole("reader");
    if(role & Commenter)
        api->setAdditionalRoles( QStringList() << "commenter" );

    GCF::Result apiResult = api->blockingExecute();
    delete api;

    if(apiResult.isSuccess())
    {
        GCF::GDriveContent::Permission permission(apiResult.data().toMap());
        d->contentPermissions[index.row()] = permission;
        QModelIndex start = this->index(index.row(), 0);
        QModelIndex end = this->index(index.row(), this->columnCount()-1);
        emit dataChanged(start, end);
        return true;
    }

    return apiResult;
}

void GDriveLiteContentSharingModel::onFileGetApiDone(const QVariant &v, const GCF::Result &result)
{
    d->contentItem = GCF::GDriveContent::Item();
    if(!result.isSuccess())
    {
        emit error(result.message());
        return;
    }

    d->contentItem = GCF::GDriveContent::Item(v.toMap());
    if(!d->contentItem.isValid())
    {
        emit error( QString("Could not fetch information for content-id: %1").arg(d->contentItemId) );
        return;
    }

    if(d->contentItem.isValid() && d->contentPermissions.count())
        emit modified();
}

void GDriveLiteContentSharingModel::onPermissionsListApiDone(const QVariant &v, const GCF::Result &result)
{
    d->contentPermissions.clear();
    if(!result.isSuccess())
    {
        emit error(result.message());
        return;
    }

    QVariantList list = v.toList();
    if(list.isEmpty())
    {
        emit error( QString("Could not fetch permissions for content-id: %1").arg(d->contentItemId) );
        return;
    }

    this->beginResetModel();
    Q_FOREACH(QVariant item, list)
    {
        GCF::GDriveContent::Permission permission(item.toMap());
        d->contentPermissions.append(permission);
    }
    this->endResetModel();

    if(d->contentItem.isValid() && d->contentPermissions.count())
        emit modified();
}

void GDriveLiteContentSharingModel::initialize()
{
    // Get item information
    GDriveLiteAPI::Files::GetRequest *getApi = new GDriveLiteAPI::Files::GetRequest(d->gDriveLite, this);
    getApi->setFileId(d->contentItemId);
    getApi->setAutoDelete(true);
    connect(getApi, SIGNAL(done(QVariant,GCF::Result)), this, SLOT(onFileGetApiDone(QVariant,GCF::Result)));
    GCF::Result result = getApi->execute();
    if(!result.isSuccess())
    {
        emit error(result.message());
        delete getApi;
        return;
    }

    // Get permissions
    GDriveLiteAPI::Permissions::ListRequest *listApi = new GDriveLiteAPI::Permissions::ListRequest(d->gDriveLite, this);
    listApi->setFileId(d->contentItemId);
    listApi->setAutoDelete(true);
    connect(listApi, SIGNAL(done(QVariant,GCF::Result)), this, SLOT(onPermissionsListApiDone(QVariant,GCF::Result)));
    result = listApi->execute();
    if(!result.isSuccess())
    {
        emit error(result.message());
        delete getApi;
        delete listApi;
        return;
    }
}

