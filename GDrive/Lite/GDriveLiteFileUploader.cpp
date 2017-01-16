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

#include "Json.h"
#include "GDriveLiteAPI.h"
#include "GDriveLiteFileUploader.h"
#include "GDriveLiteContentModel.h"

#include <QThread>
#include <QFileInfo>
#include <QPointer>
#include <QHttpPart>
#include <QNetworkReply>
#include <QNetworkAccessManager>

struct GDriveLiteFileUploaderData
{
    GDriveLiteFileUploaderData():
      gDriveLite(0),
      networkReply(0)
      {}

    QString uploadFolderId;
    GDriveLite *gDriveLite;
    QString uploadFileName;
    QPointer<QNetworkReply> networkReply;
    GCF::GDriveContent::Item folderItem;
    GCF::GDriveContent::Item uplodedItem;
};

GDriveLiteFileUploader::GDriveLiteFileUploader(const QString &fileName, GDriveLite *gDriveLite) :
    IGDriveLiteFileUploader(gDriveLite)
{
    d = new struct GDriveLiteFileUploaderData;

    d->uploadFileName = fileName;
    d->gDriveLite = gDriveLite;
    this->setTitle( QString("Uploading '%1'..").arg( QFileInfo(fileName).fileName() ) );

    connect(this, SIGNAL(started(GCF::AbstractJob*)), this, SIGNAL(uploadFileNameChanged()));
}

GDriveLiteFileUploader::~GDriveLiteFileUploader()
{
    delete d;
}

void GDriveLiteFileUploader::setUploadFolderId(const QString &folderId)
{
    if(folderId.isEmpty() || folderId == d->uploadFolderId || this->isUploading())
        return;

    d->uploadFolderId = folderId;
}

QString GDriveLiteFileUploader::uploadFolderId() const
{
    return d->uploadFolderId;
}

QString GDriveLiteFileUploader::uploadFileName() const
{
    return d->uploadFileName;
}

GCF::GDriveContent::Item GDriveLiteFileUploader::uploadedItem() const
{
    return d->uplodedItem;
}

GCF::Result GDriveLiteFileUploader::startJob()
{
    if(QThread::currentThread() != this->thread())
        return GCF::Result(false, QString(), "This function cannot be called from a thread");

    if(!d->gDriveLite->isAuthenticated())
        return GCF::Result(false, QString(), "User not activated to begin the upload");

    if(d->uploadFileName.isEmpty() || !QFileInfo(d->uploadFileName).exists())
        return GCF::Result(false, QString(), "Specifified file does not exist");

    QFile file(d->uploadFileName);
    bool readable = file.open(QFile::ReadOnly);
    file.close();
    if(!readable)
    {
        QString fName = d->uploadFileName;
        d->uploadFileName.clear();
        return GCF::Result(false, QString(),
                           QString("Cannot open '%1' for reading contents.").arg(fName));
    }

    if(d->uploadFolderId.isEmpty())
        return this->beginUpload();

    // First fetch item information of the folder in the cloud into which
    // the file needs to be uploaded
    GCF::Result result = this->fetchUploadFolderItem();

    if(!result.isSuccess())
        return result;

    // Now validate the item to which needs to be uploaded and continue.
    if( !(d->folderItem.isValid()) || !d->folderItem.isFolder())
        return GCF::Result(false, QString(), "Requested folder-id is not valid");

    return this->beginUpload();
}

GCF::Result GDriveLiteFileUploader::cancelJob()
{
    return abortUpload("Upload cancelled by user");
}

QString GDriveLiteFileUploader::uploadMimeType(const QString &fileName) const
{
    static QMap<QString,QString> extMimeTypeMap;
    if(extMimeTypeMap.isEmpty())
    {
        extMimeTypeMap["doc"] = "application/msword";
        extMimeTypeMap["docx"] = "application/msword";
        extMimeTypeMap["xls"] = "application/vnd.ms-excel";
        extMimeTypeMap["xlsx"] = "application/vnd.ms-excel";
        extMimeTypeMap["ppt"] = "application/vnd.ms-powerpoint";
        extMimeTypeMap["pptx"] = "application/vnd.ms-powerpoint";
        extMimeTypeMap["pdf"] = "application/pdf";
        extMimeTypeMap["exe"] = "application/x-msdos-program";
        extMimeTypeMap["rar"] = "application/rar";
        extMimeTypeMap["png"] = "image/png";
        extMimeTypeMap["bmp"] = "image/bmp";
        extMimeTypeMap["jpg"] = "image/jpeg";
        extMimeTypeMap["jpeg"] = "image/jpeg";
        extMimeTypeMap["jpe"] = "image/jpeg";
        extMimeTypeMap["rtf"] = "application/rtf";
        extMimeTypeMap["tar"] = "application/x-tar";
        extMimeTypeMap["zip"] = "application/zip";
        extMimeTypeMap["gif"] = "image/gif";
        extMimeTypeMap["wav"] = "application/x-wav";
        extMimeTypeMap["tif"] = "image/tif";
        extMimeTypeMap["tiff"] = "image/tif";
        extMimeTypeMap["txt"] = "text/plain";
        extMimeTypeMap["h"] = "text/plain";
        extMimeTypeMap["c"] = "text/plain";
        extMimeTypeMap["cpp"] = "text/plain";
        extMimeTypeMap["php"] = "text/plain";
        extMimeTypeMap["js"] = "text/plain";
        extMimeTypeMap["py"] = "text/plain";
        extMimeTypeMap["java"] = "text/plain";
        extMimeTypeMap["mpeg"] = "video/mpeg";
        extMimeTypeMap["mpg"] = "video/mpeg";
        extMimeTypeMap["mpe"] = "video/mpeg";
        extMimeTypeMap["qt"] = "video/quicktime";
        extMimeTypeMap["mov"] = "video/quicktime";
        extMimeTypeMap["avi"] = "video/x-msvideo";
        extMimeTypeMap["mp4"] = "video/mpeg";
        extMimeTypeMap["mp3"] = "audio/mpeg";
    }

    QString extension = QFileInfo(fileName).suffix();
    QString mimeType = extMimeTypeMap.value(extension, "application/octet-stream");
    return mimeType;
}

GCF::Result GDriveLiteFileUploader::fetchUploadFolderItem()
{
    d->folderItem = d->gDriveLite->contentModel()->item(d->uploadFolderId);

    if(d->folderItem.isValid())
    {
        if(d->folderItem.isFolder())
            return true;

        return GCF::Result(false, QString(),
                           QString("Provided upload folder id is not of a folder"));
    }

    GDriveLiteAPI::Files::GetRequest *api = new GDriveLiteAPI::Files::GetRequest(d->gDriveLite, this);
    api->setFileId(d->uploadFolderId);
    api->execute();
    GCF::SignalSpy spy(api, SIGNAL(done(QVariant,GCF::Result)));
    if(!spy.wait(10000))
        return GCF::Result(false, QString(),
                           QString("Could not fetch information about the folder item to which needs to be uploaded"));

    QVariantList args = spy.takeLast();
    GCF::Result sigResult = args.last().value<GCF::Result>();
    if(sigResult.isSuccess() == false)
        return GCF::Result(false, QString(), "Invalid item information received from Google Drive");

    QVariantMap info = args.first().toMap();
    d->folderItem = GCF::GDriveContent::Item(info);
    if(d->folderItem.isValid() && d->folderItem.id() == d->uploadFolderId && d->folderItem.isFolder())
        return true;

    d->folderItem = GCF::GDriveContent::Item();
    return GCF::Result(false, QString(), "Invalid item information received from Google Drive");
}

GCF::Result GDriveLiteFileUploader::beginUpload()
{
    QString mimeType = uploadMimeType(d->uploadFileName);

    QString authString = QString("Bearer %1").arg(d->gDriveLite->accessToken());

    QFileInfo fi(d->uploadFileName);
    QVariantMap uploadMetaDataMap;
    uploadMetaDataMap["title"] = fi.fileName();
    QVariantMap parentInfo;
    parentInfo["id"] = d->uploadFolderId;
    uploadMetaDataMap["parents"] = QVariantList() << parentInfo;

    QByteArray uploadMetaData = Json().serialize(uploadMetaDataMap);

    QNetworkRequest request(QUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable"));
    request.setRawHeader("Authorization", authString.toLatin1());
    request.setRawHeader("Content-Type", (QString("application/json; charset=UTF-8").toLatin1()));
    request.setRawHeader("X-Upload-Content-Type", mimeType.toLatin1());
    request.setRawHeader("X-Upload-Content-Length", (QString::number(fi.size()).toLatin1()));

    d->networkReply = d->gDriveLite->networkAccessManager()->post(request, uploadMetaData);
    if(!d->networkReply)
        return GCF::Result(false, QString(), "Couldnt send upload request");

    d->networkReply->setParent(this);

    connect(d->networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onMetaDataUploadError()));
    connect(d->networkReply, SIGNAL(finished()), this, SLOT(onMetaDataUploadFinished()));

    this->setProgress(0, "Uploading meta information about the file");
    emit uploadProgressChanged(this->progress());

    return true;
}

void GDriveLiteFileUploader::onMetaDataUploadError()
{
    if(d->networkReply.isNull())
        return;

    QString error = d->networkReply.data()->errorString();
    this->abortUpload(error);
}

void GDriveLiteFileUploader::onMetaDataUploadFinished()
{
    if(d->networkReply.isNull())
        return;

    QString uploadSessionURI = d->networkReply.data()->rawHeader("Location");
    d->networkReply->deleteLater();

    QFile *file = new QFile(d->uploadFileName);
    qint64 size = file->size();
    QString mimeType = uploadMimeType(d->uploadFileName);

    if(!file->open(QIODevice::ReadOnly))
    {
        abortUpload( QString("Unable to open the file %1").arg(d->uploadFileName));
        return;
    }

    QNetworkRequest request(uploadSessionURI);
    QString authString = QString("Bearer %1").arg(d->gDriveLite->accessToken());
    request.setRawHeader("Authorization", authString.toLatin1());
    request.setRawHeader("Content-Type", mimeType.toLatin1());
    request.setRawHeader("Content-Length", (QString::number(size)).toLatin1());
    d->networkReply = d->gDriveLite->networkAccessManager()->post(request, file);
    file->setParent(d->networkReply);

    connect(d->networkReply.data(), SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(onUploadProgress(qint64,qint64)));
    connect(d->networkReply.data(), SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onError()));
    connect(d->networkReply.data(), SIGNAL(finished()), this, SLOT(onUploadFinished()));
}

void GDriveLiteFileUploader::onUploadProgress(qint64 bytes, qint64 total)
{
    if(total <= 0)
        return;

    int percent = int((double(bytes)/double(total))*100.0);
    if(this->progress() == percent)
        return;

    QString text = QString("Uploaded %1 of %2 bytes").arg(bytes).arg(total);
    this->setProgress(percent, text);
    emit uploadProgressChanged(this->progress());
}

void GDriveLiteFileUploader::onError()
{
    QString error = d->networkReply->errorString();
    this->setProgress(0, error);
    this->abortUpload(error);
}

void GDriveLiteFileUploader::onUploadFinished()
{
    if(this->isComplete())
        return;

    this->setProgress(100, "Upload complete");

    QNetworkReply *reply = d->networkReply.data();
    QByteArray bytes = reply->readAll();
    QVariant json = Json().parse(bytes);
    QVariantMap response = json.toMap();

    if(response.contains("error"))
    {
        QVariantMap error = response.value("error").toMap();
        QString message = error.value("message").toString();
        this->abortUpload(message);
    }
    else
    {
        d->uplodedItem = GCF::GDriveContent::Item(response);
        GDriveLiteContentModel *contentModel = (GDriveLiteContentModel*)(d->gDriveLite->contentModel());
        contentModel->addItem(d->uploadFolderId, d->uplodedItem);
        this->clearError();
        emit uploadProgressChanged(this->progress());
        this->done();
        d->networkReply->deleteLater();
    }
}

GCF::Result GDriveLiteFileUploader::abortUpload(const QString &errMsg)
{
    if(!d->networkReply.isNull())
    {
        disconnect(d->networkReply, 0, this, 0);
        emit uploadProgressChanged(this->progress());
        this->abort(errMsg);
        d->networkReply->deleteLater();
        return true;
    }

    return GCF::Result(false, QString(), "There was no upload to abort");
}
