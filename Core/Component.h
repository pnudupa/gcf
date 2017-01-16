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

#ifndef COMPONENT_H
#define COMPONENT_H

#include "GCFGlobal.h"
#include "Version.h"
#include "Application.h"

#include <QEvent>
#include <QObject>

class QSettings;

namespace GCF
{

class Application;
class FinalizeEvent;
class InitializeEvent;
class ActivationEvent;
class ContentLoadEvent;
class SettingsLoadEvent;
class DeactivationEvent;
class ContentUnloadEvent;
class SettingsUnloadEvent;
class ContentObjectLoadEvent;
class ContentObjectMergeEvent;
class ContentObjectUnloadEvent;
class ContentObjectUnmergeEvent;
class ActivateContentObjectEvent;
class DeactivateContentObjectEvent;

struct ApplicationServicesData;

struct ComponentData;
class GCF_EXPORT Component : public QObject
{
    Q_OBJECT

public:
    Component(QObject *parent = 0);

    // Information about the vendor that gave this component
    virtual QString name() const; // Returns component class-name by default
    virtual QString organization() const; // Returns qApp->organizationName() by default
    virtual GCF::Version version() const; // Returns 1.0.0 by default
    virtual QString buildTimestamp() const; // Returns __TIMESTAMP__ by default

    // Component settings for public access
    const QSettings *settings() const;

    // Checking state of the component
    bool isLoaded() const;
    bool isActive() const;

    // Shortcut methods
    void load();
    void unload();
    void activate();
    void deactivate();

    // Adding custom content to the component
    void addContentObject(const QString &name, QObject *object, const QVariantMap &info=QVariantMap());
    void addContentObject(QObject *object, const QVariantMap &info=QVariantMap()) {
        // assuming that the object has a name, otherwise we generate a name
        this->addContentObject(object->objectName(), object, info);
    }
    void removeContentObject(QObject *object);
    void removeContentObject(const QString &name);

protected:
    ~Component(); // We only want the Application class to be able to delete the component
    bool event(QEvent *e);
    virtual void finalizeEvent(GCF::FinalizeEvent *e);
    virtual void initializeEvent(GCF::InitializeEvent *e);
    virtual void activationEvent(GCF::ActivationEvent *e);
    virtual void contentLoadEvent(GCF::ContentLoadEvent *e);
    virtual void deactivationEvent(GCF::DeactivationEvent *e);
    virtual void settingsLoadEvent(GCF::SettingsLoadEvent *e);
    virtual void contentUnloadEvent(GCF::ContentUnloadEvent *e);
    virtual void settingsUnloadEvent(GCF::SettingsUnloadEvent *e);
    virtual void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e);
    virtual void contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e);
    virtual void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e);
    virtual void contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e);
    virtual void activateContentObjectEvent(GCF::ActivateContentObjectEvent *e);
    virtual void deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e);

    virtual QObject *loadObject(const QString &name, const QVariantMap &info);
    virtual bool unloadObject(const QString &name, QObject *object, const QVariantMap &info);
    virtual bool mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);

private:
    void setSettings(QSettings *settings);

private:
    friend class Application;
    friend struct ApplicationServicesData;
    ComponentData *d;
};

// Component Event Classes
class GCF_EXPORT InitializeEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    InitializeEvent(int initType=0);
    ~InitializeEvent() { }

    bool isPreInitialize() const { return m_initType < 0; }
    bool isPostInitialize() const { return m_initType > 0; }

private:
    qint8 m_initType; // PRE (-1), POST (+1)
};

class GCF_EXPORT SettingsLoadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    SettingsLoadEvent(const QString &settingsFile=QString(), int settingsLoadType=0);
    ~SettingsLoadEvent() { }

    bool isPreSettingsLoad() const { return m_settingsLoadType < 0; }
    bool isPostSettingsLoad() const { return m_settingsLoadType > 0; }

    void setSettingsFile(const QString &fileName) { m_settingsFile = fileName; }
    QString settingsFile() const { return m_settingsFile; }

private:
    qint8 m_settingsLoadType;
    QString m_settingsFile;
};

class GCF_EXPORT ContentLoadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentLoadEvent(const QString &contentFile=QString(), int contentLoadType=0);
    ~ContentLoadEvent() { }

    bool isPreContentLoad() const { return m_contentLoadType < 0; }
    bool isPostContentLoad() const { return m_contentLoadType > 0; }

    void setContentFile(const QString &fileName) { m_contentFile = fileName; }
    QString contentFile() const { return m_contentFile; }

private:
    int m_contentLoadType;
    QString m_contentFile;
};

class GCF_EXPORT ActivationEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ActivationEvent(int actType=0);
    ~ActivationEvent() { }

    bool isPreActivation() const { return m_activationType < 0; }
    bool isPostActivation() const { return m_activationType > 0; }

private:
    int m_activationType;
};

class GCF_EXPORT DeactivationEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    DeactivationEvent(int deactType=0);
    ~DeactivationEvent() { }

    bool isPreDeactivation() const { return m_deactivationType < 0; }
    bool isPostDeactivation() const { return m_deactivationType > 0; }

private:
    int m_deactivationType;
};

class GCF_EXPORT SettingsUnloadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    SettingsUnloadEvent(int settingsUnloadType=0);
    ~SettingsUnloadEvent() { }

    bool isPreSettingsUnload() const { return m_settingsUnloadType < 0; }
    bool isPostSettingsUnload() const { return m_settingsUnloadType > 0; }

private:
    qint8 m_settingsUnloadType;
};

class GCF_EXPORT ContentUnloadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentUnloadEvent(int contentUnloadType=0);
    ~ContentUnloadEvent() { }

    bool isPreContentUnload() const { return m_contentUnloadType < 0; }
    bool isPostContentUnload() const { return m_contentUnloadType > 0; }

private:
    int m_contentUnloadType;
};

class GCF_EXPORT FinalizeEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    FinalizeEvent(int finType=0);
    ~FinalizeEvent() { }

    bool isPreFinalize() const { return m_finalizeType < 0; }
    bool isPostFinalize() const { return m_finalizeType > 0; }

private:
    int m_finalizeType;
};

class GCF_EXPORT ContentObjectLoadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentObjectLoadEvent(const QString &objName, const QVariantMap &info);
    ~ContentObjectLoadEvent() { }

    QString objectName() const { return m_objectName; }
    QVariantMap info() const { return m_info; }

    void setObject(QObject *object) { m_object = object; }
    QObject *object() const { return m_object; }

private:
    QString m_objectName;
    QVariantMap m_info;
    QObject *m_object;
};

class GCF_EXPORT ContentObjectMergeEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentObjectMergeEvent(QObject *parent, QObject *child,
                            const QVariantMap &parentInfo,
                            const QVariantMap &childInfo);
    ~ContentObjectMergeEvent() { }

    QObject *parent() const { return m_parent; }
    QObject *child() const { return m_child; }
    QVariantMap parentInfo() const { return m_parentInfo; }
    QVariantMap childInfo() const { return m_childInfo; }

private:
    QObject *m_parent;
    QObject *m_child;
    QVariantMap m_parentInfo;
    QVariantMap m_childInfo;
};

class GCF_EXPORT ActivateContentObjectEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ActivateContentObjectEvent(QObject *parent, QObject *child,
                            const QVariantMap &parentInfo,
                            const QVariantMap &childInfo);
    ~ActivateContentObjectEvent() { }

    QObject *parent() const { return m_parent; }
    QObject *child() const { return m_child; }
    QVariantMap parentInfo() const { return m_parentInfo; }
    QVariantMap childInfo() const { return m_childInfo; }

private:
    QObject *m_parent;
    QObject *m_child;
    QVariantMap m_parentInfo;
    QVariantMap m_childInfo;
};

class GCF_EXPORT DeactivateContentObjectEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    DeactivateContentObjectEvent(QObject *parent, QObject *child,
                            const QVariantMap &parentInfo,
                            const QVariantMap &childInfo);
    ~DeactivateContentObjectEvent() { }

    QObject *parent() const { return m_parent; }
    QObject *child() const { return m_child; }
    QVariantMap parentInfo() const { return m_parentInfo; }
    QVariantMap childInfo() const { return m_childInfo; }

private:
    QObject *m_parent;
    QObject *m_child;
    QVariantMap m_parentInfo;
    QVariantMap m_childInfo;
};

class GCF_EXPORT ContentObjectUnloadEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentObjectUnloadEvent(const QString &objName, QObject *object, const QVariantMap &info);
    ~ContentObjectUnloadEvent() { }

    QString objectName() const { return m_objectName; }
    QVariantMap info() const { return m_info; }
    QObject *object() const { return m_object; }

private:
    QString m_objectName;
    QVariantMap m_info;
    QObject *m_object;
};

class GCF_EXPORT ContentObjectUnmergeEvent : public QEvent
{
public:
    static const QEvent::Type Kind;

    ContentObjectUnmergeEvent(QObject *parent, QObject *child,
                            const QVariantMap &parentInfo,
                            const QVariantMap &childInfo);
    ~ContentObjectUnmergeEvent() { }

    QObject *parent() const { return m_parent; }
    QObject *child() const { return m_child; }
    QVariantMap parentInfo() const { return m_parentInfo; }
    QVariantMap childInfo() const { return m_childInfo; }

private:
    QObject *m_parent;
    QObject *m_child;
    QVariantMap m_parentInfo;
    QVariantMap m_childInfo;
};

namespace ComponentEvent
{
QString GCF_EXPORT eventName(QEvent::Type eventType);
}

}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::Component*)

#define GCF_EXPORT_COMPONENT(ComponentClass) \
extern "C" Q_DECL_EXPORT const char *GCF3ComponentBuildVersion(); \
extern "C" Q_DECL_EXPORT GCF::Component *GCF3CreateComponentInstance(); \
const char *GCF3ComponentBuildVersion() { \
    static char versionStr[20]; \
    qsnprintf(versionStr, 20, "%hu.%hu.%hu", GCF_VERSION_MAJOR, \
              GCF_VERSION_MINOR, GCF_VERSION_REVISION); \
    return versionStr; \
} \
GCF::Component *GCF3CreateComponentInstance() { \
    ComponentClass *inst = new ComponentClass; \
    return inst; \
}

#endif // COMPONENT_H
