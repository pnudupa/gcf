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

#ifndef IGDRIVELITECONTENTMODEL_H
#define IGDRIVELITECONTENTMODEL_H

#include "GDriveContent.h"
#include <QAbstractItemModel>

namespace GCF
{

class IGDriveLiteContentModel : public QAbstractItemModel
{
public:
    enum Field
    {
        Kind = Qt::UserRole,
        Id,
        ETag,
        SelfLink,
        WebContentLink,
        WebViewLink,
        AlternateLink,
        EmbedLink,
        OpenWithLinks,
        DefaultOpenWithLink,
        IconLink,
        ThumbnailLink,
        Title,
        MimeType,
        Description,
        IsStarred,
        IsHidden,
        IsTrashed,
        IsRestricted,
        IsViewed,
        CreatedDate,
        ModifiedDate,
        ModifiedByMeDate,
        LastViewedByMeDate,
        SharedWithMeDate,
        DownloadUrl,
        ExportLinks,
        IndexableText,
        UserPermissions,
        OriginalFileName,
        FileExtension,
        Md5Checksum,
        FileSize,
        QuataBytesUsed,
        OwnerNames,
        Owners,
        LastModifyingUserName,
        LastModifyingUser,
        IsEditable,
        IsCopyable,
        CanWritersShare,
        IsShared,
        IsExplicitlyTrashed,
        IsAppDataContents,
        HeadRevisionId,
        Properties,
        ImageMediaMetaData,
        IsFolder,
        IsFile,
        IsSpecialItem, // MyDrive, Trash etc..
        IsFileInfoLoaded,
        IsFolderContentLoaded,
        MinField = Kind,
        MaxField = IsFolderContentLoaded
    };

    enum ContentVisibility
    {
        ShowMyDrive = 1,
        ShowSharedWithMe = 2,
        ShowTrashed = 4,
        ShowStarred = 8,
        ShowAll = ShowMyDrive|ShowSharedWithMe|ShowTrashed|ShowStarred
    };

    virtual void setContentVisibility(int flags) = 0;
    virtual int contentVisibility() const = 0;

    virtual void setColumns(const QList<Field> &fields) = 0;
    virtual QList<Field> columns() const = 0;

    virtual GCF::Result refresh(const QModelIndex &index=QModelIndex()) = 0;

    virtual GCF::GDriveContent::Item item(const QModelIndex &index) const = 0;
    virtual GCF::GDriveContent::Item item(const QString &id) const = 0;
    virtual QModelIndex indexForId(const QString &id, int column=0) const = 0;
    virtual QModelIndexList indexListForId(const QString &id, int column=0) const = 0;

    enum SpecialItem
    {
        MyDrive,
        SharedWithMe,
        Trash,
        Starred
    };
    virtual QModelIndex specialItemIndex(SpecialItem item) const = 0;

    // These functions only improve testablity. They add nothing else
    // for usability of this model class.
    Q_SIGNAL virtual void fileItemLoaded(const QModelIndex &index) = 0;
    Q_SIGNAL virtual void folderItemLoaded(const QModelIndex &index) = 0;

protected:
    IGDriveLiteContentModel(QObject *parent=0) : QAbstractItemModel(parent) { }
    ~IGDriveLiteContentModel() { }
};

}

Q_DECLARE_INTERFACE(GCF::IGDriveLiteContentModel, "com.vcreatelogic.IGDriveLiteContentModel/3.0.0")

#endif // IGDRIVELITECONTENTMODEL_H
