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

#ifndef IGDRIVELITEFILEUPLOADER_H
#define IGDRIVELITEFILEUPLOADER_H

#include <GCF3/GCFGlobal>
#include <GCF3/AbstractJob>
#include <QObject>

#include "GDriveContent.h"

namespace GCF
{

class IGDriveLiteFileUploader : public GCF::AbstractJob
{
public:
    ~IGDriveLiteFileUploader() { }

    virtual void setUploadFolderId(const QString &uploadFolderId) = 0;
    virtual QString uploadFolderId() const = 0;

    virtual QString uploadFileName() const = 0;

    virtual GCF::GDriveContent::Item uploadedItem() const = 0;
    Q_SIGNAL virtual void uploadProgressChanged(int val) = 0;

    // inline helpers
    bool isUploading() const { return this->isRunning(); }
    bool isUploadComplete() const { return this->isComplete(); }
    int uploadProgress() const { return this->progress(); }
    GCF::Result uploadCompleteResult() const {
        if(this->isComplete())
            return GCF::Result(!this->hasError(), QString(), this->error());

        return GCF::Result(false, QString(), "Upload not yet complete");
    }

    GCF::Result abortUpload() { return this->cancel(); }
    GCF::Result retryUpload() { return this->retry(); }
    GCF::Result upload(const QString &uploadFolderId=QString()) {
        if(!uploadFolderId.isEmpty())
            this->setUploadFolderId(uploadFolderId);
        return this->start();
    }

protected:
    IGDriveLiteFileUploader(QObject *parent=0)
        : GCF::AbstractJob("GDrive Uploads", parent) { }
};

}

#endif // IGDRIVELITEFILEUPLOADER_H
