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

#ifndef SIMPLECOMPONENT_H
#define SIMPLECOMPONENT_H

#include <GCF3/Component>

class SimpleComponent : public GCF::Component
{
public:
    static int Counter;
    static QList<QEvent*> Events;

    SimpleComponent(QObject *parent=0) : GCF::Component(parent) { ++Counter; }
    ~SimpleComponent() { --Counter; }

    QString name() const { return "SimpleComponent"; }
    QList<QEvent*> events() const { return Events; }
    void clear() { qDeleteAll(Events); Events.clear(); }

protected:
    void finalizeEvent(GCF::FinalizeEvent *e) {
        GCF::Component::finalizeEvent(e);
        Events.append( new GCF::FinalizeEvent(e->isPreFinalize() ? -1 : 1) );
    }

    void initializeEvent(GCF::InitializeEvent *e) {
        GCF::Component::initializeEvent(e);
        Events.append( new GCF::InitializeEvent(e->isPreInitialize() ? -1 : 1) );
    }

    void activationEvent(GCF::ActivationEvent *e) {
        GCF::Component::activationEvent(e);
        Events.append( new GCF::ActivationEvent(e->isPreActivation() ? -1 : 1) );
    }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        GCF::Component::contentLoadEvent(e);
        Events.append( new GCF::ContentLoadEvent(e->contentFile(), e->isPreContentLoad() ? -1 : 1) );
    }

    void deactivationEvent(GCF::DeactivationEvent *e) {
        GCF::Component::deactivationEvent(e);
        Events.append( new GCF::DeactivationEvent(e->isPreDeactivation() ? -1 : 1) );
    }

    void settingsLoadEvent(GCF::SettingsLoadEvent *e) {
        GCF::Component::settingsLoadEvent(e);
        Events.append( new GCF::SettingsLoadEvent(e->settingsFile(), e->isPreSettingsLoad() ? -1 : 1) );
    }

    void contentUnloadEvent(GCF::ContentUnloadEvent *e) {
        GCF::Component::contentUnloadEvent(e);
        Events.append( new GCF::ContentUnloadEvent(e->isPreContentUnload() ? -1 : 1) );
    }

    void settingsUnloadEvent(GCF::SettingsUnloadEvent *e) {
        GCF::Component::settingsUnloadEvent(e);
        Events.append( new GCF::SettingsUnloadEvent(e->isPreSettingsUnload() ? -1 : 1) );
    }

    void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e) {
        GCF::Component::contentObjectLoadEvent(e);
        GCF::ContentObjectLoadEvent *newe = new GCF::ContentObjectLoadEvent(e->objectName(), e->info());
        newe->setObject(e->object());
        Events.append(newe);
    }

    void contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e) {
        GCF::Component::contentObjectMergeEvent(e);
        Events.append( new GCF::ContentObjectMergeEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e) {
        GCF::Component::contentObjectUnloadEvent(e);
        Events.append( new GCF::ContentObjectUnloadEvent(e->objectName(), e->object(), e->info()) );
    }

    void contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e) {
        GCF::Component::contentObjectUnmergeEvent(e);
        Events.append( new GCF::ContentObjectUnmergeEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void activateContentObjectEvent(GCF::ActivateContentObjectEvent *e) {
        GCF::Component::activateContentObjectEvent(e);
        Events.append( new GCF::ActivateContentObjectEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }

    void deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e) {
        GCF::Component::deactivateContentObjectEvent(e);
        Events.append( new GCF::DeactivateContentObjectEvent(e->parent(), e->child(), e->parentInfo(), e->childInfo()) );
    }
};

#endif // SIMPLECOMPONENT_H
