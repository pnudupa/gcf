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

#ifndef IGDRIVELITEFILEDOWNLOADER_H
#define IGDRIVELITEFILEDOWNLOADER_H

#include <GCF3/GCFGlobal>
#include <GCF3/AbstractJob>
#include <QObject>

#include "GDriveContent.h"

namespace GCF
{

class IGDriveLiteFileDownloader : public GCF::AbstractJob
{
public:
    ~IGDriveLiteFileDownloader() { }

    virtual QString id() const = 0;
    virtual GCF::GDriveContent::Item item() const = 0;

    virtual void setPreferredDownloadMimeType(const QString &mimeType) = 0;
    virtual QString preferredDownloadMimeType() const = 0;
    virtual QString usedDownloadMimeType() const = 0;

    virtual void setDownloadedFileName(const QString &fName) = 0;
    virtual QString downloadedFileName() const = 0;

    Q_SIGNAL virtual void downloadProgressChanged(int) = 0;

protected:
    IGDriveLiteFileDownloader(QObject *parent=0)
        : GCF::AbstractJob("GDrive Downloads", parent) { }

public:
    // Inline helpers
    bool isDownloading() const { return this->isRunning(); }
    bool isDownloadComplete() const { return this->isComplete(); }
    int downloadProgress() const { return this->progress(); }
    GCF::Result downloadCompleteResult() const {
        if(this->isComplete())
            return GCF::Result(!this->hasError(), QString(), this->error());
        return GCF::Result(false, QString(), "Download not yet complete");
    }

    GCF::Result abortDownload() { return this->cancel(); }
    GCF::Result retryDownload() { return this->retry(); }
    GCF::Result download(const QString &localFileName=QString()) {
        if(!localFileName.isEmpty())
            this->setDownloadedFileName(localFileName);
        return this->start();
    }
};

}

#endif // IGDRIVELITEDOWNLOADER_H
