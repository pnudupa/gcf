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

#ifndef GDRIVECONTENT_H
#define GDRIVECONTENT_H

#include <QVariant>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>
#include <QVariantList>
#include <GCF3/GCFGlobal>

namespace GCF
{

namespace GDriveContent
{

inline QVariant MapValue(const QVariantMap &map, const QString &path) {
    if(path.isEmpty() || map.isEmpty())
        return QVariant();

    QStringList comps = path.split('.', QString::SkipEmptyParts);
    QVariantMap map2 = map;
    QVariant retVal;
    while(comps.count()) {
        QString comp = comps.takeFirst();
        retVal = map2.value(comp);
        if(comps.count()) {
            map2 = retVal.toMap();
            retVal = QVariant();
        }
    }

    return retVal;
}

#define FN3(Type, Name, Path) \
    Type Name() const { return GCF::GDriveContent::MapValue(m_map, Path).value<Type>(); }
#define FN2(Type, Name) FN3(Type, Name, #Name)

#define BeginType(Name) \
class Name \
{ \
    QVariantMap m_map; \
public: \
    Name() { } \
    Name(const QVariantMap &map) : m_map(map) { } \
    Name(const Name &other) : m_map(other.m_map) { } \
    ~Name() { } \
    bool isValid() const { return !m_map.isEmpty(); } \
    QVariantMap data() const { return m_map; } \
    bool operator == (const Name &other) const { return m_map == other.m_map; } \
    Name &operator = (const Name &other) { m_map = other.m_map; return *this; } \
    FN2(QString, kind)

#define EndType(Name) };

BeginType(Permission)
    FN2(QString, etag)
    FN2(QString, id)
    FN2(QString, selfLink)
    FN2(QString, name)
    FN2(QString, emailAddress)
    FN2(QString, domain)
    FN2(QStringList, additionalRoles)
    FN2(QString, role)
    FN2(QString, type)
    FN2(QString, value)
    FN2(QString, authKey)
    FN2(QString, withLink)
    FN2(QString, photoLink)
EndType(Permission)

BeginType(User)
    FN2(QString, displayName)
    FN3(QString, picture, "picture.url")
    FN2(bool, isAuthenticatedUser)
    FN2(QString, permissionId)
    QString id() const { return this->permissionId(); }
EndType(User)

BeginType(Properties)
    FN2(QString, etag)
    FN2(QString, selfLink)
    FN2(QString, key)
    FN2(QString, visibility)
    FN2(QString, value)
EndType(Properties)

BeginType(ImageMediaMetaData)
    FN2(int, width)
    FN2(int, height)
    FN2(int, rotation)
    FN3(double, latitude, "location.latitude")
    FN3(double, longitude, "location.longitude")
    FN3(double, altitude, "location.altitude")
    FN2(QString, date)
    FN2(QString, cameraMake)
    FN2(QString, cameraModel)
    FN2(double, exposureTime)
    FN2(double, aperture)
    FN3(bool, isFlashUsed, "flashUsed")
    FN2(double, focalLength)
    FN2(int, isoSpeed)
    FN2(QString, meteringMode)
    FN2(QString, sensor)
    FN2(QString, exposureMode)
    FN2(QString, colorSpace)
    FN2(QString, whiteBalance)
    FN2(double, exposureBias)
    FN2(double, maxApertureValue)
    FN2(int, subjectDistance)
    FN2(QString, lens)
EndType(ImageMediaMetaData)

BeginType(ItemParent)
    FN2(QString, id)
    FN2(QString, selfLink)
    FN2(QString, parentLink)
    FN2(bool, isRoot)
EndType(ItemParent)

BeginType(Item)
    FN2(QString, id)
    FN2(QString, etag)
    FN2(QString, selfLink)
    FN2(QString, webContentLink)
    FN2(QString, webViewLink)
    FN2(QString, alternateLink)
    FN2(QString, embedLink)
    FN2(QVariantMap, openWithLinks)
    FN2(QString, defaultOpenWithLink)
    FN2(QString, iconLink)
    FN2(QString, thumbnailLink)
    FN2(QString, title)
    FN2(QString, mimeType)
    FN2(QString, description)
    FN3(bool, isStarred, "labels.starred")
    FN3(bool, isHidden, "labels.hidden")
    FN3(bool, isTrashed, "labels.trashed")
    FN3(bool, isRestricted, "labels.restricted")
    FN3(bool, isViewed, "labels.viewed")
    FN2(QDateTime, createdDate)
    FN2(QDateTime, modifiedDate)
    FN2(QDateTime, modifiedByMeDate)
    FN2(QDateTime, lastViewedByMeDate)
    FN2(QDateTime, sharedWithMeDate)
    FN2(QString, downloadUrl)
    FN2(QVariantMap, exportLinks)
    FN3(QString, indexableText, "indexableText.text")
    FN2(QString, originalFileName)
    FN2(QString, fileExtension)
    FN2(QString, md5Checksum)
    FN2(int, fileSize)
    FN2(int, quotaBytesUsed)
    FN2(QStringList, ownerNames)
    FN2(QString, lastModifyingUserName)
    FN3(bool, isEditable, "editable")
    FN3(bool, isCopyable, "copyable")
    FN3(bool, canWritersShare, "writersCanShare")
    FN3(bool, isShared, "shared")
    FN3(bool, isExplicitlyTrashed, "explicitlyTrashed")
    FN3(bool, isAppDataContents, "appDataContents")
    FN2(QString, headRevisionId)

    QList<ItemParent> parents() const {
        QVariantList list = m_map.value("parents").toList();
        QList<ItemParent> retList;
        Q_FOREACH(QVariant item, list)
            retList << ItemParent(item.toMap());
        return retList;
    }

    Permission userPermissions() const {
        QVariantMap map = m_map.value("userPermission").toMap();
        return Permission(map);
    }

    QList<User> owners() const {
        QVariantList list = m_map.value("owners").toList();
        QList<User> retList;
        Q_FOREACH(QVariant item, list)
            retList << User(item.toMap());
        return retList;
    }

    User lastModifyingUser() const {
        QVariantMap map = m_map.value("lastModifyingUser").toMap();
        return User(map);
    }

    QList<Properties> properties() const {
        QVariantList list = m_map.value("properties").toList();
        QList<Properties> retList;
        Q_FOREACH(QVariant item, list)
            retList << Properties(item.toMap());
        return retList;
    }

    ImageMediaMetaData imageMediaMetaData() const {
        QVariantMap map = m_map.value("imageMediaMetaData").toMap();
        return ImageMediaMetaData(map);
    }

    bool isOwnedByAuthenticatedUser() const {
        QList<User> owners = this->owners();
        for(int i=0; i<owners.count(); i++)
            if(owners.at(i).isAuthenticatedUser())
                return true;
        return false;
    }

    bool isFile() const { return !this->isFolder(); }
    bool isFolder() const { return this->mimeType() == "application/vnd.google-apps.folder"; }

EndType(Item)

BeginType(AuthenticatedUser)
    FN2(QString, etag)
    FN2(QString, selfLink)
    FN2(QString, name)
    FN2(int, quotaBytesTotal)
    FN2(int, quotaBytesUsed)
    FN2(int, quotaBytesUsedAggregate)
    FN2(int, quotaBytesUsedInThash)
    FN2(int, largestChangeId)
    FN2(int, remainingChangeIds)
    FN2(QString, rootFolderId)
    FN2(QString, domainSharingPolicy)
    FN2(QString, permissionId)
    User user() const {
        QVariantMap map = m_map.value("user").toMap();
        return User(map);
    }
    QString id() const { return this->permissionId(); }
EndType(AuthenticatedUser)

BeginType(Child)
    FN2(QString, id)
    FN2(QString, selfLink)
    FN2(QString, childLink)
EndType(Child)

}

#undef FN2
#undef FN3
#undef BeginType
#undef EndType

}

#endif // GDRIVECONTENT_H
