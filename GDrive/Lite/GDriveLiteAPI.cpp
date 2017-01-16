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

#include "GDriveLiteAPI.h"
#include "Json.h"

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

QNetworkRequest GDriveLiteAPI::Request::createNetworkRequest(const QUrl &url, const QVariantMap &query) const
{
    QUrl urlCopy = url;

    if(query.count())
    {
        QVariantMap::const_iterator it = query.constBegin();
        QVariantMap::const_iterator end = query.constEnd();

#if QT_VERSION >= 0x050000
        QUrlQuery urlQuery;
        while(it != end)
        {
            if(it.value().isValid() && !it.value().isNull())
                urlQuery.addQueryItem(it.key(), it.value().toString());
            ++it;
        }

        urlCopy.setQuery(urlQuery);
#else
        while(it != end)
        {
            if(it.value().isValid() && !it.value().isNull())
                urlCopy.addQueryItem(it.key(), it.value().toString());
            ++it;
        }
#endif
    }

    QString authString = QString("Bearer %1").arg(m_gDriveLite->accessToken());

    QNetworkRequest request(urlCopy);
    request.setRawHeader("Authorization", authString.toLatin1());
    request.setRawHeader("Content-Type", "application/json");

    return request;
}

void GDriveLiteAPI::Request::onFinished()
{
    if(m_networkReply.data() && this->sender() == m_networkReply.data())
    {
        QNetworkReply *reply = m_networkReply.data();
        m_networkReply = 0;

        QByteArray bytes = reply->readAll();
        QVariant json = Json().parse(bytes);
        if(json.type() != QVariant::Map && bytes.size())
        {
            QVariantMap error;
            error["code"] = 404;
            error["message"] = json.toString();

            QVariantMap map;
            map["error"] = error;

            json = map;
        }

        QVariantMap response = json.toMap();

        if(response.contains("error"))
        {
            QVariantMap error = response.value("error").toMap();
            QString code = error.value("code").toString();
            QString message = error.value("message").toString();
            GCF::Result result(false, code, message);
            emit done(QVariant(), result);
            return;
        }

        this->response(json);

        reply->deleteLater();
    }
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::AboutUserRequest::execute()
{
    if(this->isActive())
        return GCF::Result(false, QString(), "Another execute() call is already active");

    QUrl url("https://www.googleapis.com/drive/v2/about");
    QNetworkRequest request = this->createNetworkRequest(url);
    if( !this->httpGet(request) )
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::AboutUserRequest::response(const QVariant &json)
{
    QVariantMap response = json.toMap();
    emit done(response, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Files::ListRequest::execute()
{
    if(this->isActive())
        return GCF::Result(false, QString(), "Another execute() call is already active");

    QUrl url("https://www.googleapis.com/drive/v2/files");

    QVariantMap query;
    if(m_maxResults > 0)
        query["maxResults"] = m_maxResults;
    if(!m_pageToken.isEmpty())
        query["pageToken"] = m_pageToken;
    if(!m_query.isEmpty())
        query["q"] = m_query;

    QNetworkRequest request = this->createNetworkRequest(url, query);
    bool success = this->httpGet(request);
    if(success)
        return true;

    return GCF::Result(false, QString(), "Could not send request");
}

void GDriveLiteAPI::Files::ListRequest::response(const QVariant &json)
{
    QVariantMap response = json.toMap();

    m_kind = response.value("kind").toString();
    m_etag = response.value("etag").toString();
    m_selfLink = response.value("selfLink").toString();
    m_nextPageToken = response.value("nextPageToken").toString();
    m_nextLink = response.value("nextLink").toString();
    m_items = response.value("items").toList();

    emit done(m_items, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Files::GetRequest::execute()
{
    if(this->isActive())
        return GCF::Result(false, QString(), "Another execute() call is already active");

    if(m_fileId.isEmpty())
        m_fileId = "root";

    QString urlString = QString("https://www.googleapis.com/drive/v2/files/%1").arg(m_fileId);
    QUrl url(urlString);
    QVariantMap query;

    if(m_updateViewedDate)
        query["updateViewedDate"] = true;

    QNetworkRequest request = this->createNetworkRequest(url, query);
    bool success = this->httpGet(request);
    if(success)
        return true;

    return GCF::Result(false, QString(), "Could not send request");
}

void GDriveLiteAPI::Files::GetRequest::response(const QVariant &json)
{
    emit done(json, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Files::MetaDataUpdateRequest::execute()
{
    if(m_fileId.isEmpty())
        return GCF::Result(false, QString(), "File-id must be specified");

    QVariantMap query;
    query["convert"] = m_convert;
    query["newRevision"] = m_newRevision;
    query["ocr"] = m_ocr;
    if(!m_ocrLanguage.isEmpty())
        query["ocrLanguage"] = m_ocrLanguage;
    query["pinned"] = m_pinned;
    query["setModifiedDate"] = m_setModifiedDate;
    if(!m_timedTextLanguage.isEmpty())
        query["timedTextLanguage"] = m_timedTextLanguage;
    if(!m_timedTextTrackName.isEmpty())
        query["timedTextTrackName"] = m_timedTextTrackName;
    query["updateViewedDate"] = m_updateViewedDate;
    query["useContentAsIndexableText"] = m_useContentAsIndexableText;

    QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1").arg(m_fileId);
    QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr), query);

    QVariantMap putDataMap;

    if(m_description.isValid())
        putDataMap["description"] = m_description;
    if(m_indexableText.isValid())
    {
        QVariantMap map;
        map["text"] = m_indexableText;
        putDataMap["indexableText"] = map;
    }

    QVariantMap labelsMap;
    if(m_hidden.isValid())
        labelsMap["hidden"] = m_hidden;
    if(m_restricted.isValid())
        labelsMap["restricted"] = m_restricted;
    if(m_starred.isValid())
        labelsMap["starred"] = m_starred;
    if(m_trashed.isValid())
        labelsMap["trashed"] = m_trashed;
    if(m_viewed.isValid())
        labelsMap["viewed"] = m_viewed;
    if(labelsMap.count())
        putDataMap["labels"] = labelsMap;

    if(m_lastViewedByMeDate.isValid())
        putDataMap["lastViewedByMeDate"] = m_lastViewedByMeDate;
    if(m_mimeType.isValid())
        putDataMap["mimeType"] = m_mimeType;
    if(m_modifiedDate.isValid())
        putDataMap["modifiedDate"] = m_modifiedDate;
    if(m_parentIds.isValid())
    {
        QVariantList parents;
        Q_FOREACH(QString parentId, m_parentIds.toStringList())
        {
            QVariantMap parent;
            parent["id"] = parentId;
            parents.append(parent);
        }
        putDataMap["parents"] = parents;
    }
    if(m_title.isValid())
        putDataMap["title"] = m_title;

    QByteArray putData = Json().serialize(putDataMap);
    if(!this->httpPut(request, putData))
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Files::MetaDataUpdateRequest::response(const QVariant &json)
{
    emit done(json, json.isValid());
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Files::MkdirRequest::execute()
{
    QString urlStr("https://www.googleapis.com/drive/v2/files");
    QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr));

    QVariantMap postDataMap;
    postDataMap["title"] = m_title;

    QVariantMap parent;
    parent["id"] = m_parentFolderId;
    postDataMap["parents"] = QVariantList() << parent;
    postDataMap["mimeType"] = "application/vnd.google-apps.folder";

    QByteArray postData = Json().serialize(postDataMap);
    if( !this->httpPost(request, postData) )
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Files::MkdirRequest::response(const QVariant &json)
{
    emit done(json, json.isValid());
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Children::ListRequest::execute()
{
    if(this->isActive())
        return GCF::Result(false, QString(), "Another execute() call is already active");

    if(m_folderId.isEmpty())
        m_folderId = "root";

    QString urlString = QString("https://www.googleapis.com/drive/v2/files/%1/children").arg(m_folderId);
    QUrl url(urlString);

    QVariantMap query;
    if(m_maxResults > 0)
        query["maxResults"] = m_maxResults;
    if(!m_pageToken.isEmpty())
        query["pageToken"] = m_pageToken;
    if(!m_query.isEmpty())
        query["q"] = m_query;

    QNetworkRequest request = this->createNetworkRequest(url, query);
    bool success = this->httpGet(request);
    if(success)
        return true;

    return GCF::Result(false, QString(), "Could not send request");
}

void GDriveLiteAPI::Children::ListRequest::response(const QVariant &json)
{
    QVariantMap response = json.toMap();

    m_kind = response.value("kind").toString();
    m_etag = response.value("etag").toString();
    m_selfLink = response.value("selfLink").toString();
    m_nextPageToken = response.value("nextPageToken").toString();
    m_nextLink = response.value("nextLink").toString();
    m_items = response.value("items").toList();

    emit done(m_items, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Permissions::ListRequest::execute()
{
    if(m_fileId.isEmpty())
        return GCF::Result(false, QString(), "Permissions cannot be fetched for an empty file-id");

    QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1/permissions")
            .arg(m_fileId);
    QUrl url(urlStr);
    QNetworkRequest request = this->createNetworkRequest(url);
    if( !this->httpGet(request) )
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Permissions::ListRequest::response(const QVariant &json)
{
    QVariantMap map = json.toMap();
    QVariantList items = map.value("items").toList();
    emit done(items, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Permissions::InsertRequest::execute()
{
#define CHECK(param) if(m_##param.isEmpty()) return GCF::Result(false, QString(), #param " not specified")

    CHECK(fileId);
    CHECK(role);
    CHECK(type);
    CHECK(value);

#undef CHECK

    QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1/permissions").arg(m_fileId);
    QVariantMap query;
    if(!m_emailMessage.isEmpty())
        query["emailMessage"] = m_emailMessage;
    query["sendNotificationEmails"] = m_sendNotificationEmails;

    QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr), query);

    // Prepare post-data packet
    QVariantMap postData;
    postData["role"] = m_role;
    postData["type"] = m_type;
    postData["value"] = m_value;

    if(m_additionalRoles.count())
    {
        QVariantList moreRoles;
        Q_FOREACH(QString role, m_additionalRoles)
            moreRoles << role;
        postData["additionalRoles"] = moreRoles;
    }

    postData["withLink"] = m_withLink;
    QByteArray postJsonData = Json().serialize(postData);

    // Send the request
    if(!this->httpPost(request, postJsonData))
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Permissions::InsertRequest::response(const QVariant &json)
{
    emit done(json, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Permissions::DeleteRequest::execute()
{
#define CHECK(param) if(m_##param.isEmpty()) return GCF::Result(false, QString(), #param " not specified")

    CHECK(fileId);
    CHECK(permissionId);

#undef CHECK

    QString url = QString("https://www.googleapis.com/drive/v2/files/%1/permissions/%2")
            .arg(m_fileId).arg(m_permissionId);
    QNetworkRequest request = this->createNetworkRequest(QUrl(url));
    if(!this->httpDelete(request))
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Permissions::DeleteRequest::response(const QVariant &json)
{
    emit done(json, true);
}

///////////////////////////////////////////////////////////////////////////////

GCF::Result GDriveLiteAPI::Permissions::UpdateRequest::execute()
{
#define CHECK(param) if(m_##param.isEmpty()) return GCF::Result(false, QString(), #param " not specified")

    CHECK(fileId);
    CHECK(permissionId);

#undef CHECK

    QString urlStr = QString("https://www.googleapis.com/drive/v2/files/%1/permissions/%2")
            .arg(m_fileId).arg(m_permissionId);
    QVariantMap query;
    query["transferOwnership"] = m_transferOwnership;

    QNetworkRequest request = this->createNetworkRequest(QUrl(urlStr), query);

    // Prepare post-data packet
    QVariantMap postData;
    if(!m_role.isEmpty())
        postData["role"] = m_role;
    if(m_additionalRoles.count())
    {
        QVariantList moreRoles;
        Q_FOREACH(QString role, m_additionalRoles)
            moreRoles << role;
        postData["additionalRoles"] = moreRoles;
    }

    QByteArray postJsonData = postData.count() ? Json().serialize(postData) : QByteArray();

    // Send the request
    if(!this->httpPost(request, postJsonData))
        return GCF::Result(false, QString(), "Could not send request");

    return true;
}

void GDriveLiteAPI::Permissions::UpdateRequest::response(const QVariant &json)
{
    emit done(json, true);
}

