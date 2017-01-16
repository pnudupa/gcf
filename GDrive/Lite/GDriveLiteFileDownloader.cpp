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

#include "GDriveLiteFileDownloader.h"
#include "GDriveLiteAPI.h"

#include <QDir>
#include <QFile>
#include <QThread>
#include <QPointer>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <GCF3/SignalSpy>

struct GDriveLiteFileDownloaderData
{
    GDriveLite *gDriveLite;
    QString id;
    GCF::GDriveContent::Item item;

    QString downloadedFileName;
    QString preferredDownloadMimeType;
    QString usedDownloadMimeType;

    QPointer<QNetworkReply> networkReply;

    void evaluateDownloadMimeType();
    QString suggestDownloadedFileName() const;
};

GDriveLiteFileDownloader::GDriveLiteFileDownloader(const QString &id, GDriveLite *parent)
    : GCF::IGDriveLiteFileDownloader(parent)
{
    d = new GDriveLiteFileDownloaderData;
    d->gDriveLite = parent;
    d->id = id;

    connect(this, SIGNAL(completed(GCF::AbstractJob*)), this, SIGNAL(downloadedFileNameChanged()));
}

GDriveLiteFileDownloader::~GDriveLiteFileDownloader()
{
    delete d;
}

GDriveLite *GDriveLiteFileDownloader::gDriveLite() const
{
    return d->gDriveLite;
}

QString GDriveLiteFileDownloader::id() const
{
    return d->id;
}

GCF::GDriveContent::Item GDriveLiteFileDownloader::item() const
{
    return d->item;
}

void GDriveLiteFileDownloader::setPreferredDownloadMimeType(const QString &mimeType)
{
    if(this->isRunning())
        return;

    d->preferredDownloadMimeType = mimeType;
}

QString GDriveLiteFileDownloader::preferredDownloadMimeType() const
{
    return d->preferredDownloadMimeType;
}

QString GDriveLiteFileDownloader::usedDownloadMimeType() const
{
    return d->usedDownloadMimeType;
}

void GDriveLiteFileDownloader::setDownloadedFileName(const QString &fName)
{
    if(this->isRunning())
        return;

    d->downloadedFileName = fName;
}

QString GDriveLiteFileDownloader::downloadedFileName() const
{
    return d->downloadedFileName;
}

GCF::Result GDriveLiteFileDownloader::startJob()
{
    if(QThread::currentThread() != this->thread())
        return GCF::Result(false, QString(), "This function cannot be called from a thread");

    if(!d->gDriveLite->isAuthenticated())
        return GCF::Result(false, QString(), "User not activated to begin the download");

    // First fetch item information
    GCF::Result result = this->fetchItem();
    this->setTitle(d->item.title());
    this->setDescription(d->item.title());
    this->setIconUrl(d->item.iconLink());
    if(result.isSuccess() == false)
        return result;

    // Now validate the item to be downloaded and continue.
    if( !(d->item.isValid() && d->item.id() == d->id && !d->item.title().isEmpty()) )
        return GCF::Result(false, QString(), "Requested file-id is not valid");

    QString localFileName = d->downloadedFileName;
    d->downloadedFileName.clear();
    if(localFileName.isEmpty() || QFileInfo(localFileName).isDir())
    {
        QString fName = d->suggestDownloadedFileName();
        if(localFileName.isEmpty())
            d->downloadedFileName = QDir::current().absoluteFilePath(fName);
        else
            d->downloadedFileName = QDir(localFileName).absoluteFilePath(fName);
    }
    else
        d->downloadedFileName = localFileName;

    d->downloadedFileName = QFileInfo(d->downloadedFileName).absoluteFilePath();

    QFile file(d->downloadedFileName);
    bool writable = file.open(QFile::WriteOnly);
    file.close();
    QFile::remove(d->downloadedFileName);
    if(!writable)
    {
        QString fName = d->downloadedFileName;
        d->downloadedFileName.clear();
        return GCF::Result(false, QString(),
                           QString("Cannot open '%1' for writing contents of the downloaded file").arg(fName));
    }

    return this->beginDownload();
}

GCF::Result GDriveLiteFileDownloader::cancelJob()
{
    if(d->networkReply.data())
    {
        disconnect(d->networkReply, 0, this, 0);
        QFile::remove(d->downloadedFileName);
        d->networkReply->deleteLater();
        return true;
    }

    return GCF::Result(false, QString(), "There was no download to abort");
}

GCF::Result GDriveLiteFileDownloader::beginDownload()
{
    d->evaluateDownloadMimeType();
    QUrl url;
    if(d->usedDownloadMimeType == d->item.mimeType())
        url = QUrl(d->item.downloadUrl());
    else
        url = QUrl( d->item.exportLinks().value(d->usedDownloadMimeType).toString() );

    if(url.isEmpty() || !url.isValid())
        return GCF::Result(false, QString(), QString("No download-link available for mime-type '%1'").arg(d->usedDownloadMimeType));

    QString authString = QString("Bearer %1").arg(d->gDriveLite->accessToken());

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", authString.toLatin1());
    request.setRawHeader("Content-Type", d->usedDownloadMimeType.toLatin1());

    d->networkReply = d->gDriveLite->networkAccessManager()->get(request);
    if(d->networkReply.isNull())
        return GCF::Result(false, QString(), "Couldnt send download request");

    d->networkReply->setParent(this);

    connect(d->networkReply.data(), SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(onDownloadProgress(qint64,qint64)));
    connect(d->networkReply.data(), SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onError()));
    connect(d->networkReply.data(), SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(d->networkReply.data(), SIGNAL(finished()), this, SLOT(onFinished()));

    emit downloadProgressChanged(this->progress());
    return true;
}

void GDriveLiteFileDownloader::onDownloadProgress(qint64 bytes, qint64 total)
{
    if(total <= 0)
        total = (qint64)d->item.fileSize();

    if(total <= 0)
        return;

    int percent = int((double(bytes)/double(total))*100.0);
    if(this->progress() == percent)
        return;

    QString text = QString("Downloaded %1 of %2 bytes").arg(bytes).arg(total);
    this->setProgress(percent, text);
    emit downloadProgressChanged(this->progress());
}

void GDriveLiteFileDownloader::onError()
{
    QString error = d->networkReply->errorString();
    this->abortDownload(error);
}

void GDriveLiteFileDownloader::onReadyRead()
{
    QFile file(d->downloadedFileName);
    if( !file.open(QFile::Append) )
    {
        this->abortDownload( QString("Cannot write downloaded content into '%1'").arg(d->downloadedFileName) );
        return;
    }

    file.write(d->networkReply->readAll());
}

void GDriveLiteFileDownloader::onFinished()
{
    this->onReadyRead(); // To write any remaining content from the network-reply

    if(this->isComplete())
        return; // could happen if onReadyRead() fails

    this->setProgress(100, "Download complete");
    this->clearError();
    d->networkReply->deleteLater();
    this->done();
    emit downloadProgressChanged(this->progress());
}

void GDriveLiteFileDownloader::abortDownload(const QString &errMsg)
{
    if(d->networkReply.data())
    {
        disconnect(d->networkReply, 0, this, 0);
        QFile::remove(d->downloadedFileName);
        d->networkReply->deleteLater();
        this->abort(errMsg);
        emit downloadProgressChanged(this->progress());
    }
}

GCF::Result GDriveLiteFileDownloader::fetchItem()
{
    if(d->item.isValid())
        return true;

    d->item = d->gDriveLite->contentModel()->item(d->id);
    if(d->item.isValid())
        return true;

    GDriveLiteAPI::Files::GetRequest *api = new GDriveLiteAPI::Files::GetRequest(d->gDriveLite, this);
    api->setFileId(d->id);
    api->execute();
    GCF::SignalSpy spy(api, SIGNAL(done(QVariant,GCF::Result)));
    if(!spy.wait(10000))
        return GCF::Result(false, QString(), QString("Could not fetch information about the item to be downloaded"));

    QVariantList args = spy.takeLast();
    GCF::Result sigResult = args.last().value<GCF::Result>();
    if(sigResult.isSuccess() == false)
        return sigResult;

    QVariantMap info = args.first().toMap();
    d->item = GCF::GDriveContent::Item(info);

    if(d->item.isValid() && d->item.id() == d->id)
        return true;

    d->item = GCF::GDriveContent::Item();
    return GCF::Result(false, QString(), "Invalid item information received from Google Drive");
}

void GDriveLiteFileDownloaderData::evaluateDownloadMimeType()
{
    this->usedDownloadMimeType.clear();

    if(!this->preferredDownloadMimeType.isEmpty())
    {
        this->usedDownloadMimeType = this->preferredDownloadMimeType;
        return;
    }

    if(!this->item.downloadUrl().isEmpty())
    {
        this->usedDownloadMimeType = this->item.mimeType();
        return;
    }

    QVariantMap exportLinks = this->item.exportLinks();
    if(exportLinks.count() == 0)
        return;

    this->usedDownloadMimeType = exportLinks.begin().key();
}

QString GDriveLiteFileDownloaderData::suggestDownloadedFileName() const
{
    if(!this->item.isValid() || this->item.id() != this->id)
        return QString();

    QString title = QFileInfo(this->item.title()).baseName();
    QString extension = this->item.fileExtension();

    if(extension.isEmpty())
    {
        // We guess the extension from the mime-type
        // https://developers.google.com/drive/mime-types
        static QMap<QString,QString> gappMimeType;
        if(gappMimeType.isEmpty())
        {
            gappMimeType["application/vnd.google-apps.audio"] = "gaudio";
            gappMimeType["application/vnd.google-apps.document"] = "gdoc";
            gappMimeType["application/vnd.google-apps.drawing"] = "gdraw";
            gappMimeType["application/vnd.google-apps.file"] = "gfile";
            gappMimeType["application/vnd.google-apps.folder"] = "gfolder";
            gappMimeType["application/vnd.google-apps.form"] = "gform";
            gappMimeType["application/vnd.google-apps.fusiontable"] = "gfusiontable";
            gappMimeType["application/vnd.google-apps.photo"] = "gphoto";
            gappMimeType["application/vnd.google-apps.presentation"] = "gpresentation";
            gappMimeType["application/vnd.google-apps.script"] = "gscript";
            gappMimeType["application/vnd.google-apps.sites"] = "gsites";
            gappMimeType["application/vnd.google-apps.spreadsheet"] = "gspreadsheet";
            gappMimeType["application/vnd.google-apps.unknown"] = "gunknown";
            gappMimeType["application/vnd.google-apps.video"] = "gvideo";
        }

        extension = gappMimeType.value(this->item.mimeType());
    }

    if(extension.isEmpty())
        return title;

    return QString("%1.%2").arg(title).arg(extension);
}

