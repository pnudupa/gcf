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

#ifndef FIBERCONFIGURATIONSETTINGS_H
#define FIBERCONFIGURATIONSETTINGS_H

#include <QObject>
#include <QSettings>

struct FiberConfigurationSettingsData;
class FiberConfigurationSettings : public QObject
{
    Q_OBJECT

public:
    static FiberConfigurationSettings *instance();
    ~FiberConfigurationSettings();

    void setConfigurationFile(const QString& fileName);
    QString configurationFile() const;

    const QSettings &settings() const;

protected slots:
    void settingsFileChanged();

protected:
    FiberConfigurationSettings(QObject *parent = 0);

signals:
    void settingsChanged();

private:
    FiberConfigurationSettingsData *d;
};

#define gFiberConfig FiberConfigurationSettings::instance()
#define gFiberSettings FiberConfigurationSettings::instance()->settings()

#endif // FIBERCONFIGURATIONSETTINGS_H
