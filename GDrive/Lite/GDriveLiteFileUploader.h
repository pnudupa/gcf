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

#ifndef GDRIVELITEFILEUPLOADER_H
#define GDRIVELITEFILEUPLOADER_H

#include "GDriveLite.h"
#include "GDriveContent.h"
#include "IGDriveLiteFileUploader.h"

#include <QVariantMap>

struct GDriveLiteFileUploaderData;
class GDriveLiteFileUploader : public GCF::IGDriveLiteFileUploader
{
    Q_OBJECT
    Q_PROPERTY(QString uploadFileName READ uploadFileName NOTIFY uploadFileNameChanged)

public:
    GDriveLiteFileUploader( const QString &fileName, GDriveLite *gDriveLite);
    ~GDriveLiteFileUploader();

    void setUploadFolderId(const QString &uploadFolderId);
    QString uploadFolderId() const;

    QString uploadFileName() const;

    GCF::GDriveContent::Item uploadedItem() const;
    Q_SIGNAL virtual void uploadProgressChanged(int val);

protected:
    // GCF::AbstractJob implementation
    GCF::Result startJob();
    GCF::Result cancelJob();

private:
    QString uploadMimeType(const QString &fileName) const;
    GCF::Result fetchUploadFolderItem();
    GCF::Result beginUpload();
    Q_SLOT void onMetaDataUploadError();
    Q_SLOT void onMetaDataUploadFinished();
    Q_SLOT void onUploadProgress(qint64 bytes, qint64 total);
    Q_SLOT void onError();
    Q_SLOT void onUploadFinished();
    GCF::Result abortUpload(const QString &errMsg);
    Q_SIGNAL void uploadFileNameChanged();

private:
    GDriveLiteFileUploaderData* d;
};

#endif // GDRIVELITEFILEUPLOADER_H
