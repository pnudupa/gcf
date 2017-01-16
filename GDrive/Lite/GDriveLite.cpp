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

#include "GDriveLite.h"
#include "GDriveLite_p.h"
#include "Json.h"
#include "GDriveLiteAPI.h"
#include "GDriveLiteContentModel.h"
#include "GDriveLiteFileUploader.h"
#include "GDriveLiteContentSharingModel.h"
#include "GDriveLiteFileUploaderListModel.h"
#include "GDriveLiteFileDownloaderListModel.h"

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include <GCF3/Log>
#include <GCF3/MapToObject>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

static QString GDriveLiteRedirectURI("urn:ietf:wg:oauth:2.0:oob");

struct GDriveLiteData
{
    GDriveLiteData() : authenticated(false),
        contentModel(0), downloadersModel(0) { }

    QString clientID;
    QString clientSecret;

    bool authenticated;
    QString accessToken;
    QString refreshToken;
    QString errorMessage;
    QBasicTimer refreshTokenTimer;

    QNetworkAccessManager nam;

    GCF::GDriveContent::AuthenticatedUser authenticatedUser;
    GDriveLiteContentModel *contentModel;
    GDriveLiteFileUploaderListModel *uploadersModel;
    GDriveLiteFileDownloaderListModel *downloadersModel;
    GCF::MapToObject<QString> sharingModelMap;
};

GDriveLite::GDriveLite()
{
    d = new GDriveLiteData;
}

GDriveLite::~GDriveLite()
{
    delete d;
}

GCF::Version GDriveLite::version() const
{
    return GCF::Version(3, 0, 0);
}

void GDriveLite::setClientID(const QString &val)
{
    if(d->clientID.isEmpty())
        d->clientID = val;
    else
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, QString(), "ClientID can be set only once!");
}

QString GDriveLite::clientID() const
{
    return d->clientID;
}

void GDriveLite::setClientSecret(const QString &val)
{
    if(d->clientSecret.isEmpty())
        d->clientSecret = val;
    else
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, QString(), "Client-Secret can be set only once!");
}

QString GDriveLite::clientSecret() const
{
    return d->clientSecret;
}

QNetworkAccessManager *GDriveLite::networkAccessManager() const
{
    return &d->nam;
}

GCF::IGDriveLiteAuthenticator *GDriveLite::authenticator() const
{
    GDriveLite *that = const_cast<GDriveLite*>(this);
    return (GCF::IGDriveLiteAuthenticator*)(that);
}

GCF::GDriveContent::AuthenticatedUser GDriveLite::authenticatedUser() const
{
    return d->authenticatedUser;
}

GCF::IGDriveLiteContentModel *GDriveLite::contentModel() const
{
    return d->contentModel;
}

GCF::IGDriveLiteFileDownloader *GDriveLite::createFileDownloader(const QString &id)
{
    if(d->downloadersModel)
        return d->downloadersModel->createDownloader(id);

    return 0;
}

GCF::IGDriveLiteFileDownloaderListModel *GDriveLite::downloaders() const
{
    return d->downloadersModel;
}

GCF::IGDriveLiteFileUploaderListModel *GDriveLite::uploaders() const
{
    return d->uploadersModel;
}

GCF::IGDriveLiteFileUploader *GDriveLite::createFileUploader(const QString &localFileName)
{
    if(d->uploadersModel)
        return d->uploadersModel->createUploader(localFileName);

    return 0;
}

bool GDriveLite::isAuthenticated() const
{
    return d->authenticated;
}

QUrl GDriveLite::authenticationPageUrl() const
{
    if(d->clientID.isEmpty() || d->clientSecret.isEmpty())
        return QUrl();

    QUrl url("https://accounts.google.com/o/oauth2/auth?");
    const QString scope = "https://www.googleapis.com/auth/drive";

#if QT_VERSION >= 0x050000
    QUrlQuery query;
    query.addQueryItem("scope", scope);
    query.addQueryItem("redirect_uri", ::GDriveLiteRedirectURI);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", d->clientID);

    url.setQuery(query);
#else
    url.addQueryItem("scope", scope);
    url.addQueryItem("redirect_uri", ::GDriveLiteRedirectURI);
    url.addQueryItem("response_type", "code");
    url.addQueryItem("client_id", d->clientID);
#endif

    return url;
}

GCF::Result GDriveLite::authenticateFromPageTitle(const QString &title, QObject *notifyObject, const char *notifySlot)
{
    if(d->authenticated)
        return GCF::Result(false, QString(), "Already authenticated");

    if(d->clientID.isEmpty() || d->clientSecret.isEmpty())
        return GCF::Result(false, QString(), "ClientID and/or Client-Secret has not been set");

    if(title.isEmpty())
        return GCF::Result(false, QString(), "Title is empty");

    if(!title.startsWith("Success code="))
        return GCF::Result(false, QString(), "Title does not contain a sucess code");

    QString successCode = title;
    successCode.remove(0, 13); // Success code=
    if(successCode.isEmpty())
        return GCF::Result(false, QString(), "Title does not contain a success code");

    if(notifyObject && notifySlot)
    {
        QByteArray slot = QMetaObject::normalizedSignature(notifySlot+1);
        if( notifyObject->metaObject()->indexOfMethod(slot) < 0 )
            return GCF::Result(false, QString(), "Notification slot doesnt exist in the notification object");

        if( !QMetaObject::checkConnectArgs(notifySlot, "f(GCF::IGDriveLiteAuthenticator*)") )
            return GCF::Result(false, QString(), "Incompatible notification slot signature. "
                               "Notification slot should only accept GCF::IGDriveLiteAuthenticator* "
                               "or no arguments at all.");
    }

    d->authenticatedUser = GCF::GDriveContent::AuthenticatedUser();

    GDriveLiteAuthenticator *authenticator = new GDriveLiteAuthenticator(successCode, this);
    if(notifyObject && notifySlot)
        connect(authenticator, SIGNAL(done(GCF::IGDriveLiteAuthenticator*)), notifyObject, notifySlot);
    connect(authenticator, SIGNAL(done(GCF::IGDriveLiteAuthenticator*)), this, SLOT(updateAuthenticatedUser()));
    authenticator->requestAuthentication();

    return true;
}

GCF::Result GDriveLite::refreshAuthentication()
{
    if(!d->authenticated || d->accessToken.isEmpty() || d->refreshToken.isEmpty())
        return GCF::Result(false, QString(), "There is no authentication to refresh");

    if( this->findChild<GDriveLiteAuthenticationRefresh*>() )
        return GCF::Result(false, QString(), "Another refresh is underway");

    GDriveLiteAuthenticationRefresh *authRefresh = new GDriveLiteAuthenticationRefresh(this);
    authRefresh->refreshAuthentication();
    return true;
}

QString GDriveLite::accessToken() const
{
    return d->accessToken;
}

QString GDriveLite::refreshToken() const
{
    return d->refreshToken;
}

QString GDriveLite::errorMessage() const
{
    return d->errorMessage;
}

GCF::IGDriveLiteContentSharingModel *GDriveLite::createSharingModel(const QString &itemId) const
{
    QObject *object = d->sharingModelMap.value(itemId);
    if(object)
        return qobject_cast<GDriveLiteContentSharingModel*>(object);

    GDriveLite *that = const_cast<GDriveLite*>(this);
    GDriveLiteContentSharingModel *sharingModel = new GDriveLiteContentSharingModel(itemId, that);
    d->sharingModelMap.insert(itemId, sharingModel);
    return sharingModel;
}

GCF::Result GDriveLite::trash(const QString &itemId)
{
    GDriveLiteAPI::Files::TrashRequest *api = new GDriveLiteAPI::Files::TrashRequest(this, this);
    api->setFileId(itemId);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        GCF::GDriveContent::Item trashItem(result.data().toMap());
        d->contentModel->removeItem(itemId);
        d->contentModel->addItem("trash", trashItem);
        return true;
    }

    return result;
}

GCF::Result GDriveLite::deletePermanently(const QString &itemId)
{
    GDriveLiteAPI::Files::DeleteRequest *api = new GDriveLiteAPI::Files::DeleteRequest(this, this);
    api->setFileId(itemId);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        d->contentModel->removeItem(itemId);
        return true;
    }

    return result;
}

GCF::Result GDriveLite::restore(const QString &itemId)
{
    GDriveLiteAPI::Files::UntrashRequest *api = new GDriveLiteAPI::Files::UntrashRequest(this, this);
    api->setFileId(itemId);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        GCF::GDriveContent::Item restoredItem(result.data().toMap());
        d->contentModel->removeItem(itemId);

        QList<GCF::GDriveContent::ItemParent> parents = restoredItem.parents();
        Q_FOREACH(GCF::GDriveContent::ItemParent parent, parents)
            d->contentModel->addItem(parent.id(), restoredItem);

        return true;
    }

    return result;
}

GCF::Result GDriveLite::rename(const QString &itemId, const QString &newName)
{
    GDriveLiteAPI::Files::MetaDataUpdateRequest *api
            = new GDriveLiteAPI::Files::MetaDataUpdateRequest(this, this);
    api->setFileId(itemId);
    api->setTitle(newName);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        d->contentModel->changeItem(itemId, GCF::GDriveContent::Item(result.data().toMap()));
        return true;
    }

    return result;
}

GCF::Result GDriveLite::describe(const QString &itemId, const QString &description)
{
    GDriveLiteAPI::Files::MetaDataUpdateRequest *api
            = new GDriveLiteAPI::Files::MetaDataUpdateRequest(this, this);
    api->setFileId(itemId);
    api->setDescription(description);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        d->contentModel->changeItem(itemId, GCF::GDriveContent::Item(result.data().toMap()));
        return true;
    }

    return result;
}

GCF::Result GDriveLite::move(const QString &itemId, const QString &folderId)
{
    GDriveLiteAPI::Files::MetaDataUpdateRequest *api
            = new GDriveLiteAPI::Files::MetaDataUpdateRequest(this, this);
    api->setFileId(itemId);

    QString resolvedFolderId = folderId;
    if(resolvedFolderId.isEmpty())
        resolvedFolderId = d->authenticatedUser.rootFolderId();
    api->setParentIds( QStringList() << resolvedFolderId );
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        d->contentModel->removeItem(itemId);
        d->contentModel->addItem(resolvedFolderId, GCF::GDriveContent::Item(result.data().toMap()));
        return true;
    }

    return result;
}

GCF::Result GDriveLite::star(const QString &itemId)
{
    GDriveLiteAPI::Files::MetaDataUpdateRequest *api
            = new GDriveLiteAPI::Files::MetaDataUpdateRequest(this, this);
    api->setFileId(itemId);
    api->setStarred(true);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        d->contentModel->changeItem(itemId, item);
        d->contentModel->addItem("starred", item);
        return true;
    }

    return result;
}

GCF::Result GDriveLite::unstar(const QString &itemId)
{
    GDriveLiteAPI::Files::MetaDataUpdateRequest *api
            = new GDriveLiteAPI::Files::MetaDataUpdateRequest(this, this);
    api->setFileId(itemId);
    api->setStarred(false);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        d->contentModel->changeItem(itemId, item);
        d->contentModel->removeItem(itemId, "starred");
        return true;
    }

    return result;
}

GCF::Result GDriveLite::mkdir(const QString &folderName, const QString &parentId)
{
    GDriveLiteAPI::Files::MkdirRequest *api
            = new GDriveLiteAPI::Files::MkdirRequest(this, this);
    api->setParentFolderId(parentId);
    api->setTitle(folderName);
    GCF::Result result = api->blockingExecute();
    delete api;

    if(result.isSuccess())
        d->contentModel->addItem(parentId, GCF::GDriveContent::Item(result.data().toMap()));

    return result;
}

void GDriveLite::initializeEvent(GCF::InitializeEvent *e)
{
    if(e->isPreInitialize())
    {
        if(d->clientID.isEmpty())
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, QString(),
                                        "ClientID is not set before initializing GDriveLite component");
            return;
        }

        if(d->clientSecret.isEmpty())
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, QString(),
                                        "Client-Secret is not set before initializing GDriveLite component");
            return;
        }

        d->contentModel = new GDriveLiteContentModel(this);
        d->downloadersModel = new GDriveLiteFileDownloaderListModel(this);
        d->uploadersModel = new GDriveLiteFileUploaderListModel(this);
    }
    else if(e->isPostInitialize())
    {
        this->addContentObject("ContentModel", d->contentModel);
        this->addContentObject("DownloadersModel", d->downloadersModel);
        this->addContentObject("UploadersModel", d->uploadersModel);
    }
}

void GDriveLite::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == d->refreshTokenTimer.timerId())
    {
        d->refreshTokenTimer.stop();
        this->refreshAuthentication();
    }
}

void GDriveLite::updateAuthenticatedUser()
{
    GDriveLiteAPI::AboutUserRequest *api = new GDriveLiteAPI::AboutUserRequest(this, this);
    connect(api, SIGNAL(done(QVariant,GCF::Result)), this,
            SLOT(authenticatedUserRequestDone(QVariant,GCF::Result)));
    api->setAutoDelete(true);
    api->execute();
}

void GDriveLite::authenticatedUserRequestDone(const QVariant &json, const GCF::Result &result)
{
    if(result)
    {
        d->authenticatedUser = GCF::GDriveContent::AuthenticatedUser(json.toMap());
        emit authenticatedUserChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

void GDriveLiteAuthenticator::requestAuthentication()
{
    QByteArray postData;
    postData = "client_id=";
    postData += m_gDriveLite->clientID();
    postData += "&client_secret=";
    postData += m_gDriveLite->clientSecret();
    postData += "&code=";
    postData += m_successCode;
    postData += "&grant_type=authorization_code";
    postData += "&redirect_uri=";
    postData += ::GDriveLiteRedirectURI;

    QNetworkRequest request;
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setUrl(QUrl("https://accounts.google.com/o/oauth2/token"));

    QNetworkReply *reply = m_gDriveLite->networkAccessManager()->post(request, postData);
    connect(reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
}

void GDriveLiteAuthenticator::networkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
    if(!reply)
        return; // Wrong number. Wait for our reply to finish!

    QVariantMap json = Json().parse(reply->readAll()).toMap();
    if(json.isEmpty())
    {
        m_gDriveLite->d->errorMessage = "no response received from the server";
        emit done( (GCF::IGDriveLiteAuthenticator*)m_gDriveLite );
        return;
    }

    if(json.contains("error"))
    {
        m_gDriveLite->d->errorMessage = json.value("error").toString();
        emit done( (GCF::IGDriveLiteAuthenticator*)m_gDriveLite );
        return;
    }

    m_gDriveLite->d->accessToken = json.value("access_token").toString();
    m_gDriveLite->d->refreshToken = json.value("refresh_token").toString();
    m_gDriveLite->d->authenticated = !m_gDriveLite->d->accessToken.isEmpty();
    emit m_gDriveLite->authenticationChanged();

    int expiresIn = json.value("expires_in").toInt();
    int refreshTimeout = int((double(expiresIn) * 0.75)*1000);
    m_gDriveLite->d->refreshTokenTimer.stop();
    m_gDriveLite->d->refreshTokenTimer.start(refreshTimeout, m_gDriveLite);

    emit done( (GCF::IGDriveLiteAuthenticator*)m_gDriveLite );
}

///////////////////////////////////////////////////////////////////////////////

void GDriveLiteAuthenticationRefresh::refreshAuthentication()
{
    QByteArray postData;
    postData = "client_id=";
    postData += m_gDriveLite->clientID();
    postData += "&client_secret=";
    postData += m_gDriveLite->clientSecret();
    postData += "&refresh_token=";
    postData += m_gDriveLite->refreshToken();
    postData += "&grant_type=refresh_token";

    QNetworkRequest request(QUrl("https://accounts.google.com/o/oauth2/token"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_gDriveLite->networkAccessManager()->post(request, postData);
    connect(reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
}

void GDriveLiteAuthenticationRefresh::networkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
    if(!reply)
        return; // Wrong number. Wait for our reply to finish!

    QVariantMap json = Json().parse(reply->readAll()).toMap();
    if(json.isEmpty() || json.contains("error"))
        return; // We can ignore now and schedule a refresh for later.

    m_gDriveLite->d->accessToken = json.value("access_token").toString();
    emit m_gDriveLite->authenticationChanged();

    int expiresIn = json.value("expires_in").toInt();
    int refreshTimeout = int((double(expiresIn) * 0.75)*1000);
    m_gDriveLite->d->refreshTokenTimer.stop();
    m_gDriveLite->d->refreshTokenTimer.start(refreshTimeout, m_gDriveLite);

    this->deleteLater();
}

GCF_EXPORT_COMPONENT(GDriveLite)


