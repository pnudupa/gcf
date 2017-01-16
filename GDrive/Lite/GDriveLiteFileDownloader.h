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

#ifndef GDRIVELITEFILEDOWNLOADER_H
#define GDRIVELITEFILEDOWNLOADER_H

#include "IGDriveLiteFileDownloader.h"
#include "GDriveLite.h"

struct GDriveLiteFileDownloaderData;
class GDriveLiteFileDownloader : public GCF::IGDriveLiteFileDownloader
{
    Q_OBJECT
    Q_PROPERTY(QString downloadedFileName READ downloadedFileName NOTIFY downloadedFileNameChanged)

public:
    GDriveLiteFileDownloader(const QString &id, GDriveLite *parent=0);
    ~GDriveLiteFileDownloader();

    GDriveLite *gDriveLite() const;

    // GCF::IGDriveLiteFileDownloader implementation
    QString id() const;
    GCF::GDriveContent::Item item() const;
    void setPreferredDownloadMimeType(const QString &mimeType);
    QString preferredDownloadMimeType() const;
    QString usedDownloadMimeType() const;
    void setDownloadedFileName(const QString &fName);
    QString downloadedFileName() const;
    Q_SIGNAL void downloadProgressChanged(int val);

protected:
    // GCF::AbstractJob implementation
    GCF::Result startJob();
    GCF::Result cancelJob();

private:
    GCF::Result beginDownload();
    Q_SLOT void onDownloadProgress(qint64 bytes, qint64 total);
    Q_SLOT void onError();
    Q_SLOT void onReadyRead();
    Q_SLOT void onFinished();
    void abortDownload(const QString &errMsg);
    GCF::Result fetchItem();
    Q_SIGNAL void downloadedFileNameChanged();

private:
    GDriveLiteFileDownloaderData *d;
};

#endif // GDRIVELITEFILEDOWNLOADER_H
