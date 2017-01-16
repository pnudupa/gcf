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

#ifndef GENERICCOMPONENT_H
#define GENERICCOMPONENT_H

#include <GCF3/Component>

#include "Object.h"

#include <QMap>
#include <QFileInfo>

struct LoadEvent
{
    LoadEvent() : Object(0) { }
    LoadEvent(const QString &name, QObject *ptr, const QVariantMap &info)
        : Name(name), Object(ptr), Info(info) { }

    QString Name;
    QObject *Object;
    QVariantMap Info;
};
typedef LoadEvent UnloadEvent;

struct MergeEvent
{
    MergeEvent() : Parent(0), Child(0) { }
    MergeEvent(QObject *parent, QObject *child,
               const QVariantMap &parentInfo,
               const QVariantMap &childInfo)
        : Parent(parent), Child(child),
          ParentInfo(parentInfo), ChildInfo(childInfo) { }

    QObject *Parent;
    QObject *Child;
    QVariantMap ParentInfo;
    QVariantMap ChildInfo;
};
typedef MergeEvent UnmergeEvent;
typedef MergeEvent ActivateEvent;
typedef MergeEvent DeactivateEvent;

class GenericComponent : public GCF::Component
{
public:
    GenericComponent(QObject *parent=0)
        : GCF::Component(parent) {
        this->LoadObjectEvents = 0;
        this->UnloadObjectEvents = 0;
        this->MergeEvents = 0;
        this->UnmergeEvents = 0;
        this->ActivateEvents = 0;
        this->DeactivateEvents = 0;
    }

    ~GenericComponent() { this->clear(); }

    QSettings *writableSettings() const {
        return const_cast<QSettings*>(this->settings());
    }

    QString name() const {
        if(m_contentFile.isEmpty())
            return "GenericComponent";
        return QFileInfo(m_contentFile).baseName();
    }

    QList<QEvent*> events() const { return m_events; }
    void clear() { qDeleteAll(m_events); m_events.clear(); }

    void setContentFile(const QString &file) { m_contentFile = file; }
    QString contentFile() const { return m_contentFile; }

    void setSettingsFile(const QString &file) { m_settingsFile = file; }
    QString settingsFile() const { return m_settingsFile; }

    void finalizeEvent(GCF::FinalizeEvent *e) {
        GCF::Component::finalizeEvent(e);
        m_events.append( new GCF::FinalizeEvent(e->isPreFinalize() ? -1 : 1) );
    }

    void initializeEvent(GCF::InitializeEvent *e) {
        GCF::Component::initializeEvent(e);
        m_events.append( new GCF::InitializeEvent(e->isPreInitialize() ? -1 : 1) );
    }

    void activationEvent(GCF::ActivationEvent *e) {
        GCF::Component::activationEvent(e);
        m_events.append( new GCF::ActivationEvent(e->isPreActivation() ? -1 : 1) );
    }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(m_contentFile);
        else if(e->isPostContentLoad())
            m_contentFile = e->contentFile();
        GCF::Component::contentLoadEvent(e);
        m_events.append( new GCF::ContentLoadEvent(e->contentFile(), e->isPreContentLoad() ? -1 : 1) );
    }

    void deactivationEvent(GCF::DeactivationEvent *e) {
        GCF::Component::deactivationEvent(e);
        m_events.append( new GCF::DeactivationEvent(e->isPreDeactivation() ? -1 : 1) );
    }

    void settingsLoadEvent(GCF::SettingsLoadEvent *e) {
        if(e->isPreSettingsLoad())
            e->setSettingsFile(m_settingsFile);
        else if(e->isPostSettingsLoad())
            m_settingsFile = e->settingsFile();
        GCF::Component::settingsLoadEvent(e);
        m_events.append( new GCF::SettingsLoadEvent(e->settingsFile(), e->isPreSettingsLoad() ? -1 : 1) );
    }

    void contentUnloadEvent(GCF::ContentUnloadEvent *e) {
        GCF::Component::contentUnloadEvent(e);
        m_events.append( new GCF::ContentUnloadEvent(e->isPreContentUnload() ? -1 : 1) );
    }

    void settingsUnloadEvent(GCF::SettingsUnloadEvent *e) {
        GCF::Component::settingsUnloadEvent(e);
        m_events.append( new GCF::SettingsUnloadEvent(e->isPreSettingsUnload() ? -1 : 1) );
    }

    void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e) {
        GCF::Component::contentObjectLoadEvent(e);
        GCF::ContentObjectLoadEvent *newe = new GCF::ContentObjectLoadEvent(e->objectName(), e->info());
        newe->setObject(e->object());
        m_events.append(newe);
    }

    void contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e) {
        GCF::Component::contentObjectMergeEvent(e);
        m_events.append( new GCF::ContentObjectMergeEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e) {
        GCF::Component::contentObjectUnloadEvent(e);
        m_events.append( new GCF::ContentObjectUnloadEvent(e->objectName(), e->object(), e->info()) );
    }

    void contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e) {
        GCF::Component::contentObjectUnmergeEvent(e);
        m_events.append( new GCF::ContentObjectUnmergeEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void activateContentObjectEvent(GCF::ActivateContentObjectEvent *e) {
        GCF::Component::activateContentObjectEvent(e);
        m_events.append( new GCF::ActivateContentObjectEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e) {
        GCF::Component::deactivateContentObjectEvent(e);
        m_events.append( new GCF::DeactivateContentObjectEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    QList<LoadEvent> *LoadObjectEvents;
    QList<UnloadEvent> *UnloadObjectEvents;
    QObject *loadObject(const QString &name, const QVariantMap &info) {
        Object *retObj = new Object(this);
        if(this->LoadObjectEvents)
            this->LoadObjectEvents->append(LoadEvent(name, retObj, info));
        return retObj;
    }

    bool unloadObject(const QString &name, QObject *object, const QVariantMap &info) {
        if(this->UnloadObjectEvents != 0)
            this->UnloadObjectEvents->append( UnloadEvent(name, object, info) );
        return GCF::Component::unloadObject(name, object, info);
    }

    QList<MergeEvent> *MergeEvents;
    QList<UnmergeEvent> *UnmergeEvents;
    bool mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->MergeEvents != 0)
            this->MergeEvents->append( MergeEvent(parent, child, parentInfo, childInfo) );
        return GCF::Component::mergeObject(parent, child, parentInfo, childInfo);
    }

    bool unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeEvents != 0)
            this->UnmergeEvents->append( UnmergeEvent(parent, child, parentInfo, childInfo) );
        return GCF::Component::unmergeObject(parent, child, parentInfo, childInfo);
    }

    QList<ActivateEvent> *ActivateEvents;
    QList<DeactivateEvent> *DeactivateEvents;
    bool activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->ActivateEvents != 0)
            this->ActivateEvents->append( ActivateEvent(parent, child, parentInfo,childInfo) );
        return GCF::Component::activateObject(parent, child, parentInfo, childInfo);;
    }

    bool deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->DeactivateEvents != 0)
            this->DeactivateEvents->append( DeactivateEvent(parent,child, parentInfo,childInfo) );
        return GCF::Component::deactivateObject(parent, child, parentInfo, childInfo);
    }

private:
    QString m_contentFile;
    QString m_settingsFile;
    QList<QEvent*> m_events;
};

#endif // GENERICCOMPONENT_H
