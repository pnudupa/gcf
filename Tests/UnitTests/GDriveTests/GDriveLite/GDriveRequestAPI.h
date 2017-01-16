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

#ifndef GDRIVEREQUESTAPI_H
#define GDRIVEREQUESTAPI_H

#include "Json.h"
#include <QtNetwork>
#include <GCF3/SignalSpy>
#include <GCF3/IGDriveLite>

class GDriveRequestAPI : public QObject
{
public:
    GDriveRequestAPI(GCF::IGDriveLite *gDriveLite, QObject *parent=0)
        : QObject(parent), m_gDriveLite(gDriveLite) { }

    void addQuery(const QString &key, const QVariant &value) {
        m_query[key] = value;
    }
    void clearQuery() { m_query.clear(); }

    void addData(const QString &key, const QVariant &value) {
        QVariant mapValue = m_postDataMap;
        this->addData(mapValue, key, value);
        m_postDataMap = mapValue.toMap();
    }
    void clearData() { m_postDataMap.clear(); }

    QVariantMap query() const { return m_query; }
    QVariantMap data() const { return m_postDataMap; }

    void clear() {
        this->clearQuery();
        this->clearData();
    }

    GCF::Result post(const QString &url) {
        return this->httpRequest(POST, url);
    }
    GCF::Result get(const QString &url) {
        return this->httpRequest(GET, url);
    }
    GCF::Result put(const QString &url) {
        return this->httpRequest(PUT, url);
    }
    GCF::Result remove(const QString &url) {
        return this->httpRequest(DELETE_RESOURCE, url);
    }

private:
    enum Type {
        POST,
        GET,
        PUT,
        DELETE_RESOURCE
    };

    GCF::Result httpRequest(int type, const QString &urlStr) {
        static QNetworkAccessManager nam;

        if(!m_gDriveLite || !m_gDriveLite->isAuthenticated())
            return GCF::Result(false, QString(), "Authentication information not found");

        // Send http request
        QNetworkRequest request = this->createNetworkRequest(urlStr);
        QByteArray data = Json().serialize(m_postDataMap);
        QNetworkReply *reply = 0;
        switch(type) {
        case POST: reply = nam.post(request, data); break;
        case GET: reply = nam.get(request); break;
        case PUT: reply = nam.put(request, data); break;
        case DELETE_RESOURCE: reply = nam.deleteResource(request); break;
        break;
        }

        if(!reply)
            return GCF::Result(false, QString(), "Could not send request");

        // Wait for response
        GCF::SignalSpy spy(reply, SIGNAL(finished()));
        if( !spy.wait(60000) )
            return GCF::Result(false, QString(), "Timeout");

        // Process response
        data = reply->readAll();
        delete reply;
        QVariant response = Json().parse(data);

        QVariantMap responseMap = response.toMap();
        if(responseMap.contains("error"))
        {
            QVariantMap errorMap = responseMap.value("error").toMap();
            QString code = errorMap.value("code").toString();
            QString message = errorMap.value("message").toString();
            return GCF::Result(false, code, message);
        }

        // Done
        return GCF::Result(true, QString(), QString(), response);
    }

    void addData(QVariant &mapValue, const QString &givenKey, const QVariant &givenValue) {
        QVariant value;
        QString key;

        if(givenKey.contains('.')) {
            key = givenKey.section('.', 0, 0);
            QString newKey = givenKey.section('.', 1);
            value = mapValue.toMap().value(key, QVariantMap());
            this->addData(value, newKey, givenValue);
        } else {
            value = givenValue;
            key = givenKey;
        }

        QVariantMap map = mapValue.toMap();
        map[key] = value;
        mapValue = map;
    }

    QNetworkRequest createNetworkRequest(const QString &urlStr) {
        QUrl url(urlStr);
        if(m_query.count()) {
            QVariantMap::const_iterator it = m_query.constBegin();
            QVariantMap::const_iterator end = m_query.constEnd();

    #if QT_VERSION >= 0x050000
            QUrlQuery urlQuery;
            while(it != end) {
                if(it.value().isValid() && !it.value().isNull())
                    urlQuery.addQueryItem(it.key(), it.value().toString());
                ++it;
            }
            url.setQuery(urlQuery);
    #else
            while(it != end) {
                if(it.value().isValid() && !it.value().isNull())
                    url.addQueryItem(it.key(), it.value().toString());
                ++it;
            }
    #endif
        }
        QNetworkRequest request(url);
        QString authString = QString("Bearer %1").arg(m_gDriveLite->accessToken());
        request.setRawHeader("Authorization", authString.toLatin1());
        request.setRawHeader("Content-Type", "application/json");
        return request;
    }

private:
    GCF::IGDriveLite *m_gDriveLite;
    QVariantMap m_query;
    QVariantMap m_postDataMap;
};

#endif // GDRIVEREQUESTAPI_H
