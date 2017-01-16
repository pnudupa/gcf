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

#ifndef GDRIVELITE_H
#define GDRIVELITE_H

#include <GCF3/Component>

#include "IGDriveLite.h"
#include "IGDriveLiteAuthenticator.h"

class QNetworkAccessManager;
class GDriveLiteAuthenticator;
class GDriveLiteAuthenticationRefresh;

struct GDriveLiteData;
class GDriveLite : public GCF::IGDriveLite,
                   public GCF::IGDriveLiteAuthenticator
{
    Q_OBJECT
    Q_INTERFACES(GCF::IGDriveLite)

public:
    GDriveLite();
    GCF::Version version() const;

    Q_PROPERTY(QString clientID READ clientID WRITE setClientID NOTIFY clientIDChanged)
    void setClientID(const QString &val);
    QString clientID() const;
    Q_SIGNAL void clientIDChanged(const QString &clientID);

    Q_PROPERTY(QString clientSecret READ clientSecret WRITE setClientSecret NOTIFY clientSecretChanged)
    void setClientSecret(const QString &val);
    QString clientSecret() const;
    Q_SIGNAL void clientSecretChanged(const QString &clientSecret);

    QNetworkAccessManager *networkAccessManager() const;

    // GCF::IGDriveLite implementation
    GCF::IGDriveLiteAuthenticator *authenticator() const;
    GCF::GDriveContent::AuthenticatedUser authenticatedUser() const;
    GCF::IGDriveLiteContentModel *contentModel() const;
    GCF::IGDriveLiteFileDownloader *createFileDownloader(const QString &id);
    GCF::IGDriveLiteFileUploader *createFileUploader(const QString &localFileName);
    GCF::IGDriveLiteFileDownloaderListModel *downloaders() const;
    GCF::IGDriveLiteFileUploaderListModel *uploaders() const;

    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticationChanged)
    bool isAuthenticated() const;
    QString accessToken() const;
    QString refreshToken() const;
    QString errorMessage() const;
    Q_SIGNAL void authenticationChanged();
    Q_SIGNAL void authenticatedUserChanged();

    GCF::IGDriveLiteContentSharingModel *createSharingModel(const QString &itemId) const;
    GCF::Result trash(const QString &itemId);
    GCF::Result deletePermanently(const QString &itemId);
    GCF::Result restore(const QString &itemId);
    GCF::Result rename(const QString &itemId, const QString &newName);
    GCF::Result describe(const QString &itemId, const QString &description);
    GCF::Result move(const QString &itemId, const QString &folderId);
    GCF::Result star(const QString &itemId);
    GCF::Result unstar(const QString &itemId);
    GCF::Result mkdir(const QString &folderName, const QString &parentId=QString());

    // GCF::IGDriveLiteAuthenticator implementation
    QUrl authenticationPageUrl() const;
    GCF::Result authenticateFromPageTitle(const QString &title, QObject *notifyObject, const char *notifySlot);
    GCF::Result refreshAuthentication();

protected:
    ~GDriveLite();
    void initializeEvent(GCF::InitializeEvent *e);
    void timerEvent(QTimerEvent *e);
    Q_SLOT void updateAuthenticatedUser();
    Q_SLOT void authenticatedUserRequestDone(const QVariant &json, const GCF::Result &result);

private:
    friend class GDriveLiteAuthenticator;
    friend class GDriveLiteAuthenticationRefresh;
    GDriveLiteData *d;
};

#endif // GDRIVELITE_H
