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

#include "GDriveLiteContentModel.h"
#include "GDriveLiteAPI.h"
#include "GDriveLiteContentModel_p.h"

#include <GCF3/ObjectTree>

#include <QUrl>
#include <QSet>
#include <QFont>
#include <QColor>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

typedef QMap<QString,QIcon> QIconMap;
Q_GLOBAL_STATIC(QIconMap, GDriveIconMap)

struct GDriveLiteContentModelData
{
    GDriveLite *gDriveLite;
    QList<GDriveLiteContentModel::Field> columnFields;
    int contentVisibility;

    GCF::ObjectTree content;

    QList<GCF::ObjectTreeNode*> contentNodes(const QString &id) {
        if(id.isEmpty())
            return QList<GCF::ObjectTreeNode*>();
        QSet<GCF::ObjectTreeNode*> nodes = this->contentNodes( content.rootNode(), id );
        return nodes.toList();
    }

    GCF::ObjectTreeNode *contentNode(const QModelIndex &index) {
        GCF::ObjectTreeNode *node = reinterpret_cast<GCF::ObjectTreeNode *>(index.internalPointer());
        return node;
    }

    QMap< QString, QList<GCF::ObjectTreeNode*> > iconNodesMap;

private:
    QSet<GCF::ObjectTreeNode*> contentNodes(GCF::ObjectTreeNode *node, const QString &id) {
        QSet<GCF::ObjectTreeNode*> nodes;
        if(id.isEmpty())
            return nodes;
        if(node->name() == id)
            nodes += node;
        Q_FOREACH(GCF::ObjectTreeNode *child, node->children()) {
            QSet<GCF::ObjectTreeNode*> childNodes = this->contentNodes(child, id);
            nodes += childNodes;
        }
        return nodes;
    }
};

GDriveLiteContentModel::GDriveLiteContentModel(GDriveLite *parent) :
    IGDriveLiteContentModel(parent)
{
    d = new GDriveLiteContentModelData;

#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    d->gDriveLite = parent;

    connect(d->gDriveLite, SIGNAL(authenticatedUserChanged()),
            this, SLOT(onAuthenticatedUserChanged()),
            Qt::QueuedConnection);

    // Default set of columns
    d->columnFields << Title;
    d->columnFields << OwnerNames;
    d->columnFields << MimeType;
    d->columnFields << ModifiedDate;

    // Default visibility
    d->contentVisibility = ShowAll;

    connect(&d->content, SIGNAL(nodeObjectDestroyed(GCF::ObjectTreeNode*)),
            this, SLOT(onContentNodeObjectDestroyed(GCF::ObjectTreeNode*)));

    this->clear();
}

GDriveLiteContentModel::~GDriveLiteContentModel()
{
    delete d;
}

GDriveLite *GDriveLiteContentModel::gDriveLite() const
{
    return d->gDriveLite;
}

void GDriveLiteContentModel::setContentVisibility(int flags)
{
    if(d->contentVisibility == flags)
        return;

    d->contentVisibility = flags;
    this->refresh(); // ??? FIXME
}

int GDriveLiteContentModel::contentVisibility() const
{
    return d->contentVisibility;
}

void GDriveLiteContentModel::setColumns(const QList<Field> &fields)
{
    this->beginResetModel();
    d->columnFields = fields;
    this->endResetModel();
}

QList<GDriveLiteContentModel::Field> GDriveLiteContentModel::columns() const
{
    return d->columnFields;
}

GCF::Result GDriveLiteContentModel::refresh(const QModelIndex &index)
{
    // FIXME: At this point, we will support only one refresh at any given
    // point of time. In a future release we should support any kind of
    // updating mechanism.
    GDriveLiteContentNodeObject *nodeObject = this->findChild<GDriveLiteContentNodeObject*>();
    if(nodeObject)
    {
        QString title = nodeObject->treeNode()->info().value("title", "Unknown").toString();
        QString msg;
        if(title.isEmpty())
            msg = "Another refresh is already underway. ";
        else
            msg = QString("Refresh of '%1' is underway. ").arg(title);
        msg += "Please try again after sometime.";
        return GCF::Result(false, QString(), msg);
    }

    if(index.isValid())
    {
        GCF::ObjectTreeNode *node = d->contentNode(index);
        QVariantMap nodeData = node->info();

        if(nodeData.value("_loading", true).toBool())
            return GCF::Result(false, QString(),
                               QString("'%1' is already being refreshed").arg(nodeData.value("title").toString()));

        node->writableInfo().remove("_loading");

        // If the index is already being refreshed, then dont bother
        if(node->parent() == d->content.rootNode())
        {
            // If the folder already has children information downloaded
            // then we delete it and create a dummy "loading contents..." item
            this->beginRemoveRows( this->index(node), 0, node->children().count()-1 );
            qDeleteAll(node->children());
            this->endRemoveRows();

            // This would be a top level node
            int kind = nodeData.value("_kind", -1).toInt();
            switch(kind)
            {
            case MyDrive:
                (new FolderLister(d->gDriveLite, this))->setTreeNode(node);
                break;
            case SharedWithMe:
                (new SharedWithMeLister(d->gDriveLite, this))->setTreeNode(node);
                break;
            case Trash:
                (new TrashLister(d->gDriveLite, this))->setTreeNode(node);
                break;
            case Starred:
                (new StarredLister(d->gDriveLite, this))->setTreeNode(node);
                break;
            }

            return true;
        }

        // If the index is a folder node, we need to use a folder lister.
        if(index.data(IsFolder).toBool())
        {
            // If the folder has never been "expanded" once, then it will
            // have a dummy child node with _listing = false and _folder = true
            if(node->children().count() == 1)
            {
                GCF::ObjectTreeNode *childNode = node->children().first();
                QVariantMap childNodeData = childNode->info();
                if(childNodeData.contains("_folder") && childNodeData.contains("_listing"))
                {
                    if(childNodeData.value("_listing", false).toBool())
                        return GCF::Result(false, QString(),
                                           QString("'%1' is already being refreshed").arg(nodeData.value("title").toString()));

                    (new FolderLister(node->name(), d->gDriveLite, this))->setTreeNode(node);
                    childNode->writableInfo()["_listing"] = true;
                    return true;
                }
            }

            // If the folder already has children information downloaded
            // then we delete it and create a dummy "loading contents..." item
            this->beginRemoveRows( this->index(node), 0, node->children().count()-1 );
            qDeleteAll(node->children());
            this->endRemoveRows();

            this->beginInsertRows(this->index(node), 0, 0);
            QVariantMap info;
            info["title"] = "Fetching contents...";
            info["_listing"] = true;
            info["_folder"] = true;
            QString dummyName = QString("%1_dummy").arg(node->name());
            new GCF::ObjectTreeNode(node, dummyName, this, info);
            this->endInsertRows();

            // Now force update of the folder node
            (new FolderLister(node->name(), d->gDriveLite, this))->setTreeNode(node);
            return true;
        }

        // Node is a file node
        QVariantMap info;
        info["title"] = index.data(Title).toString();
        node->writableInfo() = info;
        (new ItemInfoFetcher(node->name(), d->gDriveLite, this))->setTreeNode(node);
        return true;
    }

    if(!index.isValid())
    {
        this->clear();
        this->fetchFilesList();
        return true;
    }

    return GCF::Result(false, QString(), "An unknown error occured during refresh");
}

GCF::GDriveContent::Item GDriveLiteContentModel::item(const QModelIndex &index) const
{
    GCF::ObjectTreeNode *node = d->contentNode(index);
    if(!node || node->parent() == d->content.rootNode() || node->info().value("_loading", true).toBool())
        return GCF::GDriveContent::Item();

    return GCF::GDriveContent::Item(node->info());
}

GCF::GDriveContent::Item GDriveLiteContentModel::item(const QString &id) const
{
    if(id.isEmpty() || id == "null")
        return GCF::GDriveContent::Item();

    QList<GCF::ObjectTreeNode*> nodes = d->contentNodes(id);
    GCF::ObjectTreeNode *node = nodes.count() ? nodes.first() : 0;
    if(!node || node->parent() == d->content.rootNode() || node->info().value("_loading", true).toBool())
        return GCF::GDriveContent::Item();

    return GCF::GDriveContent::Item(node->info());
}

QModelIndex GDriveLiteContentModel::indexForId(const QString &id, int column) const
{
    if(id.isEmpty() || id == "null")
        return QModelIndex();

    QList<GCF::ObjectTreeNode*> nodes = d->contentNodes(id);
    if(nodes.count())
        return this->index(nodes.first(), column);

    return QModelIndex();
}

QModelIndexList GDriveLiteContentModel::indexListForId(const QString &id, int column) const
{
    if(id.isEmpty() || id == "null")
        return QModelIndexList();

    QList<GCF::ObjectTreeNode*> nodes = d->contentNodes(id);
    QModelIndexList indexList;
    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
        indexList.append( this->index(node, column) );

    return indexList;
}

QModelIndex GDriveLiteContentModel::specialItemIndex(SpecialItem item) const
{
    GCF::ObjectTreeNode *node = 0;
    switch(item)
    {
    case MyDrive: node = d->content.rootNode()->node(d->gDriveLite->authenticatedUser().rootFolderId()); break;
    case SharedWithMe: node = d->content.rootNode()->node("sharedWithMe"); break;
    case Trash: node = d->content.rootNode()->node("trash"); break;
    case Starred: node = d->content.rootNode()->node("starred"); break;
    default: break;
    }

    return this->index(node);
}

int GDriveLiteContentModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        GCF::ObjectTreeNode *parentNode = d->contentNode(parent);
        if(!parentNode)
            return 0;

        return parentNode->children().count();
    }

    return d->content.rootNode()->children().count();
}

int GDriveLiteContentModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if(d->columnFields.isEmpty())
        return 1;

    return d->columnFields.count();
}

QVariant GDriveLiteContentModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if( index.column() >= d->columnFields.count() )
            return QVariant();
        role = d->columnFields.at( index.column() );
    }

    GCF::ObjectTreeNode *node = d->contentNode(index);
    if(!node)
        return QVariant();

    if(node->info().value("_folder", false).toBool())
    {
        GCF::ObjectTreeNode *parentNode = node->parent();
        GCF::ObjectTreeNode *childNode = node;
        if(childNode->info().value("_listing", false).toBool() == false)
        {
            GDriveLiteContentModel *that = const_cast<GDriveLiteContentModel*>(this);
            FolderLister *folderLister = new FolderLister(parentNode->name(), d->gDriveLite, that);
            folderLister->setTreeNode(parentNode);
            childNode->writableInfo()["_listing"] = true;
        }
    }

    GCF::GDriveContent::Item item(node->info());
    switch(role)
    {
    case Kind: return item.kind();
    case Id: return node->name();
    case ETag: return item.etag();
    case SelfLink: return item.selfLink();
    case WebContentLink: return item.webContentLink();
    case WebViewLink: return item.webViewLink();
    case AlternateLink: return item.alternateLink();
    case EmbedLink: return item.embedLink();
    case OpenWithLinks: return item.openWithLinks();
    case DefaultOpenWithLink: return item.defaultOpenWithLink();
    case IconLink: {
        if(item.iconLink().isEmpty())
            return "qrc:///Icons/google-drive.png";
        return item.iconLink();
    }
    case ThumbnailLink: return item.thumbnailLink();
    case Title: {
        if(node->info().contains("_error"))
            return QString("[ %1 ]").arg(node->info().value("title").toString());
        return item.isValid() ? item.title() : node->info().value("title").toString();
        } break;
    case MimeType: return item.mimeType();
    case Description: return item.description();
    case IsStarred: return item.isStarred();
    case IsHidden: return item.isHidden();
    case IsTrashed: return item.isTrashed();
    case IsRestricted: return item.isRestricted();
    case IsViewed: return item.isViewed();
    case CreatedDate: return item.createdDate();
    case ModifiedDate: return item.modifiedDate();
    case ModifiedByMeDate: return item.modifiedByMeDate();
    case LastViewedByMeDate: return item.lastViewedByMeDate();
    case SharedWithMeDate: return item.sharedWithMeDate();
    case DownloadUrl: return item.downloadUrl();
    case ExportLinks: return item.exportLinks();
    case IndexableText: return item.indexableText();
    case UserPermissions: return item.userPermissions().data();
    case OriginalFileName: return item.originalFileName();
    case FileExtension: return item.fileExtension();
    case Md5Checksum: return item.md5Checksum();
    case FileSize: return item.fileSize();
    case QuataBytesUsed: return item.quotaBytesUsed();
    case OwnerNames: return item.ownerNames().join(", ");
    case Owners: return node->info().value("owners").toList();
    case LastModifyingUserName: return item.lastModifyingUserName();
    case LastModifyingUser: return item.lastModifyingUser().data();
    case IsEditable: return item.isEditable();
    case IsCopyable: return item.isCopyable();
    case CanWritersShare: return item.canWritersShare();
    case IsShared: return item.isShared();
    case IsExplicitlyTrashed: return item.isExplicitlyTrashed();
    case IsAppDataContents: return item.isAppDataContents();
    case HeadRevisionId: return item.headRevisionId();
    case Properties: return node->info().value("properties").toList();
    case ImageMediaMetaData: return item.imageMediaMetaData().data();
    case IsFolder: return item.isFolder() || !index.parent().isValid();
    case IsFile: return !(item.isFolder() || !index.parent().isValid());
    case IsSpecialItem: return !index.parent().isValid();
    case Qt::DecorationRole: return index.column() == 0 ? this->itemIcon(node, item) : QVariant();
    case IsFileInfoLoaded: return node->info().value("_fileInfoLoaded", false).toBool();
    case IsFolderContentLoaded: return node->info().value("_folderContentsLoaded", false).toBool();
    case Qt::ToolTipRole: {
        if(item.isShared())
            return "This item is shared";
        return node->info().value("_error").toString();
    }
    case Qt::ForegroundRole: return node->info().contains("_error") ? QVariant::fromValue<QColor>(Qt::red) : QVariant();
    case Qt::FontRole: {
        QFont font;
        if(node->info().contains("_error")) {
            font.setBold(true);
            font.setItalic(true);
            font.setUnderline(true);
        }
        else if(item.isStarred()) {
            font.setBold(true);
            font.setItalic(true);
        }
        return font;
        } break;
    default: break;
    }

    return QVariant();
}

QModelIndex GDriveLiteContentModel::index(int row, int column, const QModelIndex &parent) const
{
    if(column < 0)
        return QModelIndex();

    if(d->columnFields.count() && column >= d->columnFields.count())
        return QModelIndex();

    if(!d->columnFields.count() && column != 0)
        return QModelIndex();

    if(!parent.isValid())
    {
        if(row < 0 || row >= d->content.rootNode()->children().count())
            return QModelIndex();

        GCF::ObjectTreeNode *node = d->content.rootNode()->children().at(row);
        return this->createIndex(row, column, node);
    }

    GCF::ObjectTreeNode *parentNode = d->contentNode(parent);
    if(!parentNode)
        return QModelIndex(); // Obviously an error!

    if(row < 0 || row >= parentNode->children().count())
        return QModelIndex();

    GCF::ObjectTreeNode *node = parentNode->children().at(row);
    return this->createIndex(row, column, node);
}

QModelIndex GDriveLiteContentModel::parent(const QModelIndex &child) const
{
    if(!child.isValid())
        return QModelIndex();

    GCF::ObjectTreeNode *node = d->contentNode(child);
    if(!node)
        return QModelIndex(); // Obviously an error

    GCF::ObjectTreeNode *parentNode = node->parent();
    if(parentNode == d->content.rootNode())
        return QModelIndex();

    int row = parentNode->parent()->children().indexOf(parentNode);
    return this->createIndex(row, 0, parentNode);
}

QVariant GDriveLiteContentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0)
        return QVariant();

    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            if(d->columnFields.count() == 0)
                return "Title";

            if( section >= d->columnFields.count() )
                return QVariant();

            role = d->columnFields.at( section );

            QString name = QString::fromLatin1( this->roleNames().value(role) );
            name = name.remove(0, 7);
            return name;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags GDriveLiteContentModel::flags(const QModelIndex &index) const
{
    GCF::GDriveContent::Item item = this->item(index);
    if(item.isValid())
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;

    GCF::ObjectTreeNode *node = d->contentNode(index);
    if(node->parent() == d->content.rootNode())
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;

    return 0;
}

QHash<int,QByteArray> GDriveLiteContentModel::roleNames() const
{
    static QHash<int,QByteArray> roles;
    if(roles.isEmpty())
    {
#define ADD_ROLE(x) roles[x] = "content" #x
        ADD_ROLE(Kind);
        ADD_ROLE(Id);
        ADD_ROLE(ETag);
        ADD_ROLE(SelfLink);
        ADD_ROLE(WebContentLink);
        ADD_ROLE(WebViewLink);
        ADD_ROLE(AlternateLink);
        ADD_ROLE(EmbedLink);
        ADD_ROLE(OpenWithLinks);
        ADD_ROLE(DefaultOpenWithLink);
        ADD_ROLE(IconLink);
        ADD_ROLE(ThumbnailLink);
        ADD_ROLE(Title);
        ADD_ROLE(MimeType);
        ADD_ROLE(Description);
        ADD_ROLE(IsStarred);
        ADD_ROLE(IsHidden);
        ADD_ROLE(IsTrashed);
        ADD_ROLE(IsRestricted);
        ADD_ROLE(IsViewed);
        ADD_ROLE(CreatedDate);
        ADD_ROLE(ModifiedDate);
        ADD_ROLE(ModifiedByMeDate);
        ADD_ROLE(LastViewedByMeDate);
        ADD_ROLE(SharedWithMeDate);
        ADD_ROLE(DownloadUrl);
        ADD_ROLE(ExportLinks);
        ADD_ROLE(IndexableText);
        ADD_ROLE(UserPermissions);
        ADD_ROLE(OriginalFileName);
        ADD_ROLE(FileExtension);
        ADD_ROLE(Md5Checksum);
        ADD_ROLE(FileSize);
        ADD_ROLE(QuataBytesUsed);
        ADD_ROLE(OwnerNames);
        ADD_ROLE(Owners);
        ADD_ROLE(LastModifyingUserName);
        ADD_ROLE(LastModifyingUser);
        ADD_ROLE(IsEditable);
        ADD_ROLE(IsCopyable);
        ADD_ROLE(CanWritersShare);
        ADD_ROLE(IsShared);
        ADD_ROLE(IsExplicitlyTrashed);
        ADD_ROLE(IsAppDataContents);
        ADD_ROLE(HeadRevisionId);
        ADD_ROLE(Properties);
        ADD_ROLE(ImageMediaMetaData);
        ADD_ROLE(IsFolder);
        ADD_ROLE(IsFile);
        ADD_ROLE(IsSpecialItem);
        ADD_ROLE(IsFileInfoLoaded);
        ADD_ROLE(IsFolderContentLoaded);
#undef ADD_ROLE
    }

    return roles;
}

void GDriveLiteContentModel::onAuthenticatedUserChanged()
{
    this->clear();

    if(!d->gDriveLite->isAuthenticated())
        return;

    if(!d->gDriveLite->authenticatedUser().isValid())
        return;

    this->fetchFilesList();
}

void GDriveLiteContentModel::onContentNodeObjectDestroyed(GCF::ObjectTreeNode *node)
{
    if(node->info().isEmpty())
    {
        delete node;
        return;
    }

    QModelIndex start = this->index(node, 0);
    QModelIndex end = this->index(node, this->columnCount());
    emit dataChanged(start, end);
}

void GDriveLiteContentModel::onIconLoaded(const QString &url, const QIcon &icon)
{
    if(url.isEmpty())
        return;

    if(!icon.isNull())
    {
#if QT_VERSION >= 0x050000
        ::GDriveIconMap()->insert(url, icon);
#else
        ::GDriveIconMap()->insert(url, icon);
#endif
    }

    QList<GCF::ObjectTreeNode*> nodes = d->iconNodesMap.take(url);
    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
    {
        QModelIndex start = this->index(node, 0);
        QModelIndex end = this->index(node, this->columnCount());
        emit dataChanged(start, end);
    }
}

void GDriveLiteContentModel::clear()
{
    this->beginResetModel();

    QList<GCF::ObjectTreeNode*> nodes = d->content.rootNode()->children();
    qDeleteAll(nodes);

    this->endResetModel();
}

void GDriveLiteContentModel::fetchFilesList()
{
    this->beginResetModel();

    if(d->contentVisibility & ShowMyDrive)
    {
        QString rootId = d->gDriveLite->authenticatedUser().rootFolderId();

        QVariantMap info;
        info["title"] = "My Drive";
        info["_kind"] = MyDrive;
        info["id"] = rootId;

        FolderLister *lister = new FolderLister(d->gDriveLite, this);
        lister->setTreeNode(new GCF::ObjectTreeNode(d->content.rootNode(), rootId, lister, info));
    }

    if(d->contentVisibility & ShowSharedWithMe)
    {
        QVariantMap info;
        info["title"] = "Shared with me";
        info["_kind"] = SharedWithMe;

        SharedWithMeLister *lister = new SharedWithMeLister(d->gDriveLite, this);
        lister->setTreeNode(new GCF::ObjectTreeNode(d->content.rootNode(), "sharedWithMe", lister, info));
    }

    if(d->contentVisibility & ShowTrashed)
    {
        QVariantMap info;
        info["title"] = "Trash";
        info["_kind"] = Trash;

        TrashLister *lister = new TrashLister(d->gDriveLite, this);
        lister->setTreeNode(new GCF::ObjectTreeNode(d->content.rootNode(), "trash", lister, info));
    }

    if(d->contentVisibility & ShowStarred)
    {
        QVariantMap info;
        info["title"] = "Starred";
        info["_kind"] = Starred;

        StarredLister *lister = new StarredLister(d->gDriveLite, this);
        lister->setTreeNode(new GCF::ObjectTreeNode(d->content.rootNode(), "starred", lister, info));
    }

    this->endResetModel();
}

QModelIndex GDriveLiteContentModel::index(GCF::ObjectTreeNode *node, int column) const
{
    if(!node)
        return QModelIndex();

    if(node == d->content.rootNode())
        return QModelIndex();

    GCF::ObjectTreeNode *parent = node->parent();
    return this->createIndex( parent->children().indexOf(node), column, node );
}

QIcon GDriveLiteContentModel::itemIcon(GCF::ObjectTreeNode *node, const GCF::GDriveContent::Item &item) const
{
    QString iconLink = item.iconLink();
    static QIcon gDriveIcon(":/Icons/google-drive.png");
    if(iconLink.isEmpty())
        return gDriveIcon;

#if QT_VERSION >= 0x050000
    if( ::GDriveIconMap()->contains(iconLink) )
        return ::GDriveIconMap()->value(iconLink);
#else
    if( ::GDriveIconMap()->contains(iconLink) )
        return ::GDriveIconMap()->value(iconLink);
#endif

    bool downloadIcon = (d->iconNodesMap.contains(iconLink) == false);
    d->iconNodesMap[iconLink].append(node);

    if(downloadIcon)
    {
        GDriveLiteAPI::Resource::IconRequest *api =
                new GDriveLiteAPI::Resource::IconRequest(d->gDriveLite, const_cast<GDriveLiteContentModel*>(this));
        api->setIconUrl(iconLink);
        connect(api, SIGNAL(done(QString,QIcon)), this, SLOT(onIconLoaded(QString,QIcon)));
        api->execute();
    }

    return gDriveIcon;
}

void GDriveLiteContentModel::addItem(const QString &folderId, const GCF::GDriveContent::Item &item)
{
    if(!item.isValid() || item.id().isEmpty())
        return;

    QString resolvedFolderId = folderId;
    if(resolvedFolderId == "root" || resolvedFolderId.isEmpty())
        resolvedFolderId = d->gDriveLite->authenticatedUser().id();

    QList<GCF::ObjectTreeNode*> nodes = d->contentNodes(resolvedFolderId);
    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
    {
        if(node->info().value("_folderContentsLoaded", false).toBool() == false)
            continue; // the folder has not been opened even once in the model.
                      // Lets wait for the normal refresh to happen

        QModelIndex folderIndex = this->index(node);

        // If the node corresponds to an empty folder, then we need to
        // reset that.
        if(node->children().count() == 1)
        {
            GCF::ObjectTreeNode *childNode = node->children().first();

            QVariantMap info;
            info["title"] = "Folder empty";
            info["_fileInfoLoaded"] = true;
            if(childNode->info() == info)
            {
                this->beginRemoveRows(folderIndex, 0, 0);
                delete childNode;
                this->endRemoveRows();
            }
        }

        int insertIndex = node->children().count();
        this->beginInsertRows(folderIndex, insertIndex, insertIndex);
        GCF::ObjectTreeNode *childNode = new GCF::ObjectTreeNode(node, item.id(), this, item.data());
        childNode->writableInfo()["_loading"] = false;
        this->endInsertRows();

        // If the newly added item a folder, then we insert a dummy node under the folder
        // to allow for expansion.
        if(item.isFolder())
        {
            this->beginInsertRows(this->index(childNode), 0, 0);
            QVariantMap info;
            info["title"] = "Fetching contents...";
            info["_listing"] = false;
            info["_folder"] = true;
            QString dummyName = QString("%1_dummy").arg(childNode->name());
            new GCF::ObjectTreeNode(childNode, dummyName, this, info);
            this->endInsertRows();
        }
    }
}

void GDriveLiteContentModel::removeItem(const QString &childId, const QString &folderId)
{
    if(childId.isEmpty())
        return;

    QList<GCF::ObjectTreeNode*> nodes;

    if(folderId.isEmpty())
        nodes = d->contentNodes(childId);
    else
    {
        QList<GCF::ObjectTreeNode*> folderNodes = d->contentNodes(folderId);
        Q_FOREACH(GCF::ObjectTreeNode *folderNode, folderNodes)
            nodes << folderNode->node(childId);
    }

    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
    {
        GCF::ObjectTreeNode *parentNode = node->parent();
        int index = parentNode->children().indexOf(node);
        this->beginRemoveRows(this->index(parentNode), index, index);
        delete node;
        this->endRemoveRows();
    }
}

void GDriveLiteContentModel::changeItem(const QString &itemId, const GCF::GDriveContent::Item &item)
{
    if(itemId.isEmpty() || !item.isValid() || item.id() != itemId)
        return;

    QList<GCF::ObjectTreeNode*> nodes = d->contentNodes(itemId);
    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
    {
        QStringList keys = item.data().keys();
        Q_FOREACH(QString key, keys)
            node->writableInfo()[key] = item.data().value(key);
        node->writableInfo().remove("_error");
        QModelIndex start = this->index(node, 0);
        QModelIndex end = this->index(node, this->columnCount()-1);
        emit dataChanged(start, end);
    }
}

