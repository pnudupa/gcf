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

#ifndef GDRIVELITEAPI_H
#define GDRIVELITEAPI_H

#include <GCF3/GCFGlobal>
#include <GCF3/SignalSpy>

#include <QUrl>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QPointer>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "GDriveLite.h"

namespace GDriveLiteAPI
{

class Request : public QObject
{
    Q_OBJECT

public:
    ~Request() { }

    void setAutoDelete(bool val) {
        if(m_autoDelete == val)
            return;
        if(val)
            connect(this, SIGNAL(done(QVariant,GCF::Result)), this, SLOT(deleteLater()), Qt::UniqueConnection);
        else
            disconnect(this, SIGNAL(done(QVariant,GCF::Result)), this, SLOT(deleteLater()));
    }

    bool isAutoDelete() const { return m_autoDelete; }

    GDriveLite *gDriveLite() const { return m_gDriveLite; }

    GCF::Result blockingExecute(int timeout=25000) {
        GCF::Result result = this->execute();
        if(result.isSuccess() == false)
            return result;
        return this->wait(timeout);
    }

    virtual GCF::Result execute() = 0;
    bool isActive() const { return m_networkReply.data() && m_networkReply->isRunning(); }

    GCF::Result wait(int timeout=5000) {
        GCF::SignalSpy spy(this, SIGNAL(done(QVariant,GCF::Result)));
        if( spy.wait(timeout) ) {
            QVariant data = spy.last().first();
            GCF::Result result = spy.last().last().value<GCF::Result>();
            if(result.isSuccess())
                return GCF::Result(true, QString(), QString(), data);
            return result;
        }
        return GCF::Result(false, QString(), "Timeout");
    }

signals:
    void done(const QVariant &val, const GCF::Result &result);

protected:
    Request(GDriveLite *gDriveLite, QObject *parent=0)
        : QObject(parent), m_gDriveLite(gDriveLite), m_autoDelete(false) { }

    bool httpPost(const QNetworkRequest &request, const QByteArray &postData) {
        return this->httpRequest(POST, request, postData);
    }

    bool httpGet(const QNetworkRequest &request) {
        return this->httpRequest(GET, request);
    }

    bool httpDelete(const QNetworkRequest &request) {
        return this->httpRequest(DELETE, request);
    }

    bool httpPut(const QNetworkRequest &request, const QByteArray &bytes=QByteArray()) {
        return this->httpRequest(PUT, request, bytes);
    }

    enum RequestType
    {
        POST,
        GET,
        DELETE,
        PUT
    };

    bool httpRequest(RequestType type, const QNetworkRequest &request, const QByteArray &bytes=QByteArray()) {
        if(this->isActive())
            return false;

        QNetworkReply *reply = 0;

        switch(type) {
        case POST: reply = m_gDriveLite->networkAccessManager()->post(request, bytes); break;
        case GET: reply = m_gDriveLite->networkAccessManager()->get(request); break;
        case DELETE: reply = m_gDriveLite->networkAccessManager()->deleteResource(request); break;
        case PUT: reply = m_gDriveLite->networkAccessManager()->put(request, bytes); break;
        break;
        }

        if(reply) {
            m_networkReply = reply;
            connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
            return true;
        }

        return false;
    }

    virtual void response(const QVariant &json) = 0;

    QNetworkRequest createNetworkRequest(const QUrl &url, const QVariantMap &query=QVariantMap()) const;

private slots:
    void onFinished();

private:
    GDriveLite *m_gDriveLite;
    QPointer<QNetworkReply> m_networkReply;
    bool m_autoDelete;
};

class AboutUserRequest : public Request
{
public:
    AboutUserRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~AboutUserRequest() { }

    GCF::Result execute();
    void response(const QVariant &json);
};

namespace Files
{

class ListRequest : public Request
{
public:
    ListRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent), m_maxResults(-1) { }
    ~ListRequest() { }

    void setPageToken(const QString &token) { m_pageToken = token; }
    QString pageToken() const { return m_pageToken; }

    void setMaxResults(int maxResults) { m_maxResults = maxResults; }
    int maxResults() const { return m_maxResults; }

    void setQuery(const QString &query) { m_query = query; }
    QString query() const { return m_query; }

    // Results from the last call to execute.
    QString kind() const { return m_kind; }
    QString etag() const { return m_etag; }
    QString selfLink() const { return m_selfLink; }
    QString nextPageToken() const { return m_nextPageToken; }
    QString nextLink() const { return m_nextLink; }
    QVariantList items() const { return m_items; }

    // Request interface implementation
    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_pageToken;
    int m_maxResults;
    QString m_query; // unused for now

    // from the last result
    QString m_kind;
    QString m_etag;
    QString m_selfLink;
    QString m_nextPageToken;
    QString m_nextLink;
    QVariantList m_items;
};

class GetRequest : public Request
{
public:
    GetRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~GetRequest() { }

    void setFileId(const QString &id) { m_fileId = id; }
    QString fileId() const { return m_fileId; }

    void setUpdateViewedDate(bool val) { m_updateViewedDate = val; }
    bool isUpdateViewedDate() const { return m_updateViewedDate; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
    bool m_updateViewedDate;

};

class TrashRequest : public Request
{
public:
    TrashRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~TrashRequest() { }

    void setFileId(const QString &id) { m_fileId = id; }
    QString fileId() const { return m_fileId; }

    GCF::Result execute() {
        if(m_fileId.isEmpty())
            return GCF::Result(false, QString(), "File-id is not specified");

        QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1/trash").arg(m_fileId);
        QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr));
        if( !this->httpPost(request, QByteArray()) )
            return GCF::Result(false, QString(), "Could not send request");
        return true;
    }

    void response(const QVariant &json) {
        emit done(json, json.isValid());
    }

private:
    QString m_fileId;
};

class DeleteRequest : public Request
{
public:
    DeleteRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~DeleteRequest() { }

    void setFileId(const QString &id) { m_fileId = id; }
    QString fileId() const { return m_fileId; }

    GCF::Result execute() {
        if(m_fileId.isEmpty())
            return GCF::Result(false, QString(), "File-id is not specified");

        QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1").arg(m_fileId);
        QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr));
        if( !this->httpDelete(request) )
            return GCF::Result(false, QString(), "Could not send request");
        return true;
    }

    void response(const QVariant &json) {
        emit done(json, json.isNull() || !json.isValid());
    }

private:
    QString m_fileId;
};

class UntrashRequest : public Request
{
public:
    UntrashRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~UntrashRequest() { }

    void setFileId(const QString &id) { m_fileId = id; }
    QString fileId() const { return m_fileId; }

    GCF::Result execute() {
        if(m_fileId.isEmpty())
            return GCF::Result(false, QString(), "File-id is not specified");

        QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1/untrash").arg(m_fileId);
        QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr));
        if( !this->httpPost(request, QByteArray()) )
            return GCF::Result(false, QString(), "Could not send request");
        return true;
    }

    void response(const QVariant &json) {
        emit done(json, json.isValid());
    }

private:
    QString m_fileId;
};

class MetaDataUpdateRequest : public Request
{
public:
    MetaDataUpdateRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent), m_convert(false), m_newRevision(true),
          m_ocr(false), m_pinned(false), m_setModifiedDate(false),
          m_updateViewedDate(true), m_useContentAsIndexableText(false) { }
    ~MetaDataUpdateRequest() { }

    void setFileId(const QString &val) { m_fileId = val; }
    void setConvert(bool val) { m_convert = val; }
    void setNewRevision(bool val) { m_newRevision = val; }
    void setOcr(bool val) { m_ocr = val; }
    void setOcrLanguage(const QString &val) { m_ocrLanguage = val; }
    void setPinned(bool val) { m_pinned = val; }
    void setSetModifiedDate(bool val) { m_setModifiedDate = val; }
    void setTimedTextLanguage(const QString &val) { m_timedTextLanguage = val; }
    void setTimedTextTrackName(const QString &val) { m_timedTextTrackName = val; }
    void setUpdateViewedDate(bool val) { m_updateViewedDate = val; }
    void setUseContentAsIndexableText(bool val) { m_useContentAsIndexableText = val; }
    void setDescription(const QString &val) { m_description = val; }
    void setIndexableText(const QString &val) { m_indexableText = val; }
    void setHidden(bool val) { m_hidden = val; }
    void setRestricted(bool val) { m_restricted = val; }
    void setStarred(bool val) { m_starred = val; }
    void setTrashed(bool val) { m_trashed = val; }
    void setViewed(bool val) { m_viewed = val; }
    void setLastViewedByMeDate(const QDateTime &val) { m_lastViewedByMeDate = val; }
    void setMimeType(const QString &val) { m_mimeType = val; }
    void setModifiedDate(const QDateTime &val) { m_modifiedDate = val; }
    void setParentIds(const QStringList &val) { m_parentIds = val; }
    void setTitle(const QString &val) { m_title = val; }

    QString fileId() const { return m_fileId; }
    bool convert() const { return m_convert; }
    bool newRevision() const { return m_newRevision; }
    bool ocr() const { return m_ocr; }
    QString ocrLanguage() const { return m_ocrLanguage; }
    bool pinned() const { return m_pinned; }
    bool setModifiedDate() const { return m_setModifiedDate; }
    QString timedTextLanguage() const { return m_timedTextLanguage; }
    QString timedTextTrackName() const { return m_timedTextTrackName; }
    bool updateViewedDate() const { return m_updateViewedDate; }
    bool useContentAsIndexableText() const { return m_useContentAsIndexableText; }
    QString description() const { return m_description.toString(); }
    QString indexableText() const { return m_indexableText.toString(); }
    bool hidden() const { return m_hidden.toBool(); }
    bool restricted() const { return m_restricted.toBool(); }
    bool starred() const { return m_starred.toBool(); }
    bool trashed() const { return m_trashed.toBool(); }
    bool viewed() const { return m_viewed.toBool(); }
    QDateTime lastViewedByMeDate() const { return m_lastViewedByMeDate.toDateTime(); }
    QString mimeType() const { return m_mimeType.toString(); }
    QDateTime modifiedDate() const { return m_modifiedDate.toDateTime(); }
    QStringList parentIds() const { return m_parentIds.toStringList(); }
    QString title() const { return m_title.toString(); }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
    bool m_convert;
    bool m_newRevision;
    bool m_ocr;
    QString m_ocrLanguage;
    bool m_pinned;
    bool m_setModifiedDate;
    QString m_timedTextLanguage;
    QString m_timedTextTrackName;
    bool m_updateViewedDate;
    bool m_useContentAsIndexableText;

    // In post data
    QVariant m_description;
    QVariant m_indexableText; // indexableText.text
    QVariant m_hidden; // labels.hidden
    QVariant m_restricted; // labels.restricted
    QVariant m_starred; // labels.starred
    QVariant m_trashed; // labels.trashed
    QVariant m_viewed; // labels.viewed
    QVariant m_lastViewedByMeDate;
    QVariant m_mimeType;
    QVariant m_modifiedDate;
    QVariant m_parentIds;
    QVariant m_title;
};

class MkdirRequest : public Request
{
public:
    MkdirRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~MkdirRequest() { }

    void setParentFolderId(const QString &id) { m_parentFolderId = id; }
    QString parentFolderId() const { return m_parentFolderId; }

    void setTitle(const QString &title) { m_title = title; }
    QString title() const { return m_title; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_parentFolderId;
    QString m_title;
};

}

namespace Children
{

class ListRequest : public Request
{
public:
    ListRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent), m_maxResults(-1) { }
    ~ListRequest() { }

    void setFolderId(const QString &id) { m_folderId = id; }
    QString folderId() const { return m_folderId; }

    void setMaxResults(int mr) { m_maxResults = mr; }
    int maxResults() const { return m_maxResults; }

    void setPageToken(const QString &pt) { m_pageToken = pt; }
    QString pageToken() const { return m_pageToken; }

    void setQuery(const QString &q) { m_query = q; }
    QString query() const { return m_query; }

    // Results from the last call to execute.
    QString kind() const { return m_kind; }
    QString etag() const { return m_etag; }
    QString selfLink() const { return m_selfLink; }
    QString nextPageToken() const { return m_nextPageToken; }
    QString nextLink() const { return m_nextLink; }
    QVariantList items() const { return m_items; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_folderId;
    int m_maxResults;
    QString m_pageToken;
    QString m_query;

    // from the last result
    QString m_kind;
    QString m_etag;
    QString m_selfLink;
    QString m_nextPageToken;
    QString m_nextLink;
    QVariantList m_items;
};

}

namespace Resource
{

class IconRequest : public QObject
{
    Q_OBJECT

public:
    IconRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : QObject(parent), m_gDriveLite(gDriveLite) { }
    ~IconRequest() { }

    void setIconUrl(const QString &url) { m_iconUrl = url; }
    QString iconUrl() const { return m_iconUrl; }

    GCF::Result execute() {
        if(m_iconUrl.isEmpty())
            return GCF::Result(false, QString(), "No icon url specified");
        QNetworkRequest request(m_iconUrl);
        QNetworkReply *reply = m_gDriveLite->networkAccessManager()->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));

        return true;
    }

signals:
    void done(const QString &url, const QIcon &icon);

private slots:
    void replyFinished() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
        if(!reply)
            return;

        QImage image;
        image.loadFromData(reply->readAll(), "png");
        QPixmap pm = QPixmap::fromImage(image);
        // QIcon icon( QPixmap::fromImage(image) );
        QIcon icon;
        icon.addPixmap(pm); // default size
        icon.addPixmap(pm.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon.addPixmap(pm.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon.addPixmap(pm.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon.addPixmap(pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon.addPixmap(pm.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        emit done(m_iconUrl, icon);

        this->deleteLater();
    }

private:
    QString m_iconUrl;
    GDriveLite *m_gDriveLite;
};

}

namespace Permissions
{

class ListRequest : public Request
{
public:
    ListRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~ListRequest() { }

    void setFileId(const QString &id) { m_fileId = id; }
    QString fileId() const { return m_fileId; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
};

class InsertRequest : public Request
{
public:
    InsertRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent), m_sendNotificationEmails(false),
          m_type("user"), m_withLink(true) { }
    ~InsertRequest() { }

    void setFileId(const QString &val) { m_fileId = val; }
    QString fileId() const { return m_fileId; }

    void setEMailMessage(const QString &val) { m_emailMessage = val; }
    QString emailMessage() const { return m_emailMessage; }

    void setSendNotificationEMails(bool val) { m_sendNotificationEmails = val; }
    bool isSendNotificationEMails() const { return m_sendNotificationEmails; }

    void setRole(const QString &val) { m_role = val; }
    QString role() const { return m_role; }

    void setValue(const QString &val) { m_value = val; }
    QString value() const { return m_value; }

    void setAdditionalRoles(const QStringList &val) { m_additionalRoles = val; }
    QStringList additionalRoles() const { return m_additionalRoles; }

    void setWithLink(bool val) { m_withLink = val; }
    bool isWithLink() const { return m_withLink; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
    QString m_emailMessage;
    bool m_sendNotificationEmails;
    QString m_role;
    QString m_type;
    QString m_value;
    QStringList m_additionalRoles;
    bool m_withLink;
};

class DeleteRequest : public Request
{
public:
    DeleteRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent) { }
    ~DeleteRequest() { }

    void setFileId(const QString &val) { m_fileId = val; }
    QString fileId() const { return m_fileId; }

    void setPermissionId(const QString &val) { m_permissionId = val; }
    QString permissionId() const { return m_permissionId; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
    QString m_permissionId;
};

class UpdateRequest : public Request
{
public:
    UpdateRequest(GDriveLite *gDriveLite, QObject *parent=0)
        : Request(gDriveLite, parent), m_transferOwnership(false) { }
    ~UpdateRequest() { }

    void setFileId(const QString &val) { m_fileId = val; }
    QString fileId() const { return m_fileId; }

    void setPermissionId(const QString &val) { m_permissionId = val; }
    QString permissionId() const { return m_permissionId; }

    void setTransferOwnership(bool val) { m_transferOwnership = val; }
    bool istransferOwnership() const { return m_transferOwnership; }

    void setAdditionalRoles(const QStringList &val) { m_additionalRoles = val; }
    QStringList additionalRoles() const { return m_additionalRoles; }

    void setRole(const QString &val) { m_role = val; }
    QString role() const { return m_role; }

    GCF::Result execute();
    void response(const QVariant &json);

private:
    QString m_fileId;
    QString m_permissionId;
    bool m_transferOwnership;
    QStringList m_additionalRoles;
    QString m_role;
};

}

}

#endif // GDRIVELITEAPI_H
