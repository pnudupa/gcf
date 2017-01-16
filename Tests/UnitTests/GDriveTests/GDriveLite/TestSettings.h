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

#ifndef TESTSETTINGS_H
#define TESTSETTINGS_H

#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QString>
#include <QVariant>
#include <QFile>

namespace GDriveTests
{

inline bool loadSettings(QVariantMap &settings)
{
    settings.clear();

    QStringList keys = QProcessEnvironment::systemEnvironment().keys();
    Q_FOREACH(QString key, keys)
    {
        if(key.startsWith("GDRIVE_TEST_"))
        {
            QString value = QProcessEnvironment::systemEnvironment().value(key);
            key.remove(0, 12);
            key = key.toLower();
            settings[key] = value;
        }
    }

    static QStringList expectedKeys = QStringList() << "client_id" << "client_secret"
                                                << "login" << "password"; // << "content" << "upload_folder_id" << "upload_location";
    keys = settings.keys();
    Q_FOREACH(QString expectedKey, expectedKeys)
    {
        if(!keys.contains(expectedKey))
        {
            QString key = expectedKey.toUpper();
            qDebug("Please set GDRIVE_TEST_%s environment variable", qPrintable(key));
            settings.clear();
            return false;
        }
    }

    return true;
}

}

#endif // TESTSETTINGS_H
