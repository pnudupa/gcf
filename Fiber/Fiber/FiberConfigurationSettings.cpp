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

#include "FiberConfigurationSettings.h"

#include <QMutex>
#include <QMutexLocker>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDebug>

#include <GCF3/Log>

class FiberConfigurationSettings2 : public FiberConfigurationSettings
{
public:
    FiberConfigurationSettings2() { }
    ~FiberConfigurationSettings2() { }
};

Q_GLOBAL_STATIC(FiberConfigurationSettings2, GlobalFiberConfigurationSettings)
FiberConfigurationSettings *FiberConfigurationSettings::instance()
{
    return GlobalFiberConfigurationSettings();
}

struct FiberConfigurationSettingsData
{
    FiberConfigurationSettingsData()
        : settings(0)
    { }

    QSettings *settings;
    QMutex settingsMutex;

    QString fileName;
    QFileSystemWatcher fileWatcher;
};

FiberConfigurationSettings::FiberConfigurationSettings(QObject *parent) :
    QObject(parent)
{
    d = new FiberConfigurationSettingsData;
}

FiberConfigurationSettings::~FiberConfigurationSettings()
{
    delete d;
    d = 0;
}

void FiberConfigurationSettings::setConfigurationFile(const QString &fileName)
{
    GCF::LogMessageBranch branch(GCF_DEFAULT_LOG_CONTEXT);

    if(!d->fileName.isEmpty())
        return;

    if(!QFile(fileName).exists())
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Configuration file not found !", "Missing " + fileName);
        qDebug() << "Configuration file not found !";
        return;
    }

    d->fileName = fileName;
    d->fileWatcher.addPath(d->fileName);
    connect(&d->fileWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(settingsFileChanged()), Qt::UniqueConnection);

    d->settings = new QSettings(d->fileName, QSettings::IniFormat, this);
    d->settings->sync();

    if(d->settings->status() != QSettings::NoError)
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Invalid configuration provided !", "Invalid format in " + fileName);
        qDebug() << "Invalid configuration provided !";
    }

    // Announce the availability of settings
    this->settingsFileChanged();
}

QString FiberConfigurationSettings::configurationFile() const
{
    return d->fileName;
}

const QSettings &FiberConfigurationSettings::settings() const
{
    QMutexLocker locker(&d->settingsMutex);

    return *d->settings;
}

void FiberConfigurationSettings::settingsFileChanged()
{
    QMutexLocker locker(&d->settingsMutex);

    d->settings->sync();
    emit settingsChanged();
}

