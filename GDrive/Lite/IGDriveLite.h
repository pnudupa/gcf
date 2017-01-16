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

#ifndef IGDRIVELITE_H
#define IGDRIVELITE_H

#include <GCF3/GCFGlobal>
#include <GCF3/Component>

#include "GDriveContent.h"
#include "IGDriveLiteFileUploader.h"
#include "IGDriveLiteContentModel.h"
#include "IGDriveLiteAuthenticator.h"
#include "IGDriveLiteFileDownloader.h"
#include "IGDriveLiteContentSharingModel.h"
#include "IGDriveLiteFileUploaderListModel.h"
#include "IGDriveLiteFileDownloaderListModel.h"

namespace GCF
{

class IGDriveLite : public GCF::Component
{
public:
    virtual GCF::IGDriveLiteAuthenticator *authenticator() const = 0;

    virtual bool isAuthenticated() const = 0;
    virtual GCF::GDriveContent::AuthenticatedUser authenticatedUser() const = 0;
    virtual QString accessToken() const = 0;
    virtual QString refreshToken() const = 0;
    virtual QString errorMessage() const = 0;

    virtual GCF::IGDriveLiteContentModel *contentModel() const = 0;

    virtual GCF::IGDriveLiteFileDownloader *createFileDownloader(const QString &id) = 0;
    GCF::IGDriveLiteFileDownloader *createFileDownloader(const GCF::GDriveContent::Item &item) {
        return this->createFileDownloader(item.id());
    }
    virtual GCF::IGDriveLiteFileDownloaderListModel *downloaders() const = 0;

    virtual GCF::IGDriveLiteFileUploader *createFileUploader(const QString &localFileName) = 0;
    virtual GCF::IGDriveLiteFileUploaderListModel *uploaders() const = 0;

    Q_SIGNAL virtual void authenticationChanged() = 0;
    Q_SIGNAL virtual void authenticatedUserChanged() = 0;

    virtual GCF::IGDriveLiteContentSharingModel *createSharingModel(const QString &itemId) const = 0;
    virtual GCF::Result trash(const QString &itemId) = 0;
    virtual GCF::Result deletePermanently(const QString &itemId) = 0;
    virtual GCF::Result restore(const QString &itemId) = 0;

    // Meta-data updates
    virtual GCF::Result rename(const QString &itemId, const QString &newName) = 0;
    virtual GCF::Result describe(const QString &itemId, const QString &description) = 0;
    virtual GCF::Result move(const QString &itemId, const QString &folderId=QString()) = 0;
    virtual GCF::Result star(const QString &itemId) = 0;
    virtual GCF::Result unstar(const QString &itemId) = 0;
    virtual GCF::Result mkdir(const QString &folderName, const QString &parentId=QString()) = 0;
};

}

Q_DECLARE_INTERFACE(GCF::IGDriveLite, "com.vcreatelogic.IGDriveLite/3.0.0")

#endif // IGDRIVELITE_H
