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

#ifndef TESTGUICOMPONENT_H
#define TESTGUICOMPONENT_H

#include <GCF3/GuiComponent>

#include <QAction>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

struct LoadEvent
{
    LoadEvent() : Object(0) { }
    QString Name;
    QVariantMap Info;
    QObject *Object;
};
typedef LoadEvent UnloadEvent;

struct MergeEvent
{
    MergeEvent() : Parent(0), Child(0) { }
    MergeEvent(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
        : Parent(parent), Child(child), ParentInfo(parentInfo), ChildInfo(childInfo) { }

    QObject *Parent;
    QObject *Child;
    QVariantMap ParentInfo;
    QVariantMap ChildInfo;
};
typedef MergeEvent UnmergeEvent;

class TestGuiComponent : public GCF::GuiComponent
{
public:
    TestGuiComponent(QObject *parent=0) : GCF::GuiComponent(parent) {
        this->LoadWidgetEvents = 0;
        this->UnloadWidgetEvents = 0;
        this->LoadActionEvents = 0;
        this->UnloadActionEvents = 0;
        this->LoadActionGroupEvents = 0;
        this->UnloadActionGroupEvents = 0;
        this->LoadMenuEvents = 0;
        this->UnloadMenuEvents = 0;
        this->LoadMenuBarEvents = 0;
        this->UnloadMenuBarEvents = 0;
        this->LoadToolBarEvents = 0;
        this->UnloadToolBarEvents = 0;
        this->MergeEvents = 0;
        this->UnmergeEvents = 0;
        this->MergeWidgetEvents = 0;
        this->UnmergeWidgetEvents = 0;
        this->MergeWithActionGroupEvents = 0;
        this->UnmergeFromActionGroupEvents = 0;
        this->MergeWithMenuEvents = 0;
        this->UnmergeFromMenuEvents = 0;
        this->MergeWithToolBarEvents = 0;
        this->UnmergeFromToolBarEvents = 0;
        this->MergeWithMenuBarEvents = 0;
        this->UnmergeFromMenuBarEvents = 0;
        this->ActivateObjectEvents = 0;
        this->DeactivateObjectEvents = 0;
        this->ActivateWidgetEvents = 0;
        this->DeactivateWidgetEvents = 0;
    }
    ~TestGuiComponent() { }

    QString name() const { return "TestGuiComponent"; }

    QString ContentFile;
    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(this->ContentFile);
    }

    QObject *loadObject(const QString &, const QVariantMap &info) {
        QString type = info.value("type").toString();
        if(type == "layout:vertical")
            return new QVBoxLayout;
        if(type == "layout:horizontal")
            return new QHBoxLayout;
        if(type == "layout:grid")
            return new QGridLayout;
        if(type == "layout:form")
            return new QFormLayout;
        return 0;
    }

    QList<LoadEvent>* LoadWidgetEvents;
    QList<UnloadEvent>* UnloadWidgetEvents;
    virtual QWidget *loadWidget(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = GCF::GuiComponent::loadWidget(name, info);
        if(this->LoadWidgetEvents)
            this->LoadWidgetEvents->append(evt);
        return (QWidget*)(evt.Object);
    }

    virtual bool unloadWidget(const QString &name, QWidget *widget, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = widget;
        evt.Info = info;
        if(this->UnloadWidgetEvents)
            this->UnloadWidgetEvents->append(evt);
        return GCF::GuiComponent::unloadWidget(name, widget, info);
    }

    QList<LoadEvent>* LoadActionEvents;
    QList<UnloadEvent>* UnloadActionEvents;
    virtual QAction *loadAction(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = GCF::GuiComponent::loadAction(name, info);
        if(this->LoadActionEvents)
            this->LoadActionEvents->append(evt);
        return (QAction*)(evt.Object);
    }

    virtual bool unloadAction(const QString &name, QAction *action, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = action;
        evt.Info = info;
        if(this->UnloadActionEvents)
            this->UnloadActionEvents->append(evt);
        return GCF::GuiComponent::unloadAction(name, action, info);
    }

    QList<LoadEvent> *LoadActionGroupEvents;
    QList<UnloadEvent> *UnloadActionGroupEvents;
    virtual QActionGroup *loadActionGroup(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = GCF::GuiComponent::loadActionGroup(name, info);
        if(this->LoadActionGroupEvents)
            this->LoadActionGroupEvents->append(evt);
        return (QActionGroup*)(evt.Object);
    }

    virtual bool unloadActionGroup(const QString &name, QActionGroup *actionGroup, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = actionGroup;
        evt.Info = info;
        if(this->UnloadActionGroupEvents)
            this->UnloadActionGroupEvents->append(evt);
        return GCF::GuiComponent::unloadActionGroup(name, actionGroup, info);
    }

    QList<LoadEvent> *LoadMenuEvents;
    QList<UnloadEvent> *UnloadMenuEvents;
    virtual QMenu *loadMenu(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = (QObject*)GCF::GuiComponent::loadMenu(name, info);
        if(this->LoadMenuEvents)
            this->LoadMenuEvents->append(evt);
        return (QMenu*)(evt.Object);
    }

    virtual bool unloadMenu(const QString &name, QMenu *menu, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = (QObject*)menu;
        evt.Info = info;
        if(this->UnloadMenuEvents)
            this->UnloadMenuEvents->append(evt);
        return GCF::GuiComponent::unloadMenu(name, menu, info);
    }

    QList<LoadEvent> *LoadMenuBarEvents;
    QList<UnloadEvent> *UnloadMenuBarEvents;
    virtual QMenuBar *loadMenuBar(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = (QObject*)GCF::GuiComponent::loadMenuBar(name, info);
        if(this->LoadMenuBarEvents)
            this->LoadMenuBarEvents->append(evt);
        return (QMenuBar*)(evt.Object);
    }

    virtual bool unloadMenuBar(const QString &name, QMenuBar *menuBar, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = (QObject*)menuBar;
        evt.Info = info;
        if(this->UnloadMenuBarEvents)
            this->UnloadMenuBarEvents->append(evt);
        return GCF::GuiComponent::unloadMenuBar(name, menuBar, info);
    }

    QList<LoadEvent> *LoadToolBarEvents;
    QList<UnloadEvent> *UnloadToolBarEvents;
    virtual QToolBar *loadToolBar(const QString &name, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Info = info;
        evt.Object = (QObject*)GCF::GuiComponent::loadToolBar(name, info);
        if(this->LoadToolBarEvents)
            this->LoadToolBarEvents->append(evt);
        return (QToolBar*)(evt.Object);
    }

    virtual bool unloadToolBar(const QString &name, QToolBar *menuBar, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.Object = (QObject*)menuBar;
        evt.Info = info;
        if(this->UnloadToolBarEvents)
            this->UnloadToolBarEvents->append(evt);
        return GCF::GuiComponent::unloadToolBar(name, menuBar, info);
    }

    QList<MergeEvent> *MergeEvents;
    QList<UnmergeEvent> *UnmergeEvents;
    bool mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeObject(parent, child, parentInfo, childInfo);
        if(this->MergeEvents)
            this->MergeEvents->append( MergeEvent(parent,child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeEvents)
            this->UnmergeEvents->append( UnmergeEvent(parent,child,parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeObject(parent, child, parentInfo, childInfo);
    }

    QList<MergeEvent> *MergeWidgetEvents;
    QList<UnmergeEvent> *UnmergeWidgetEvents;
    bool mergeWithWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeWithWidget(parent, child,
                                                          parentInfo, childInfo);
        if(this->MergeWidgetEvents)
            this->MergeWidgetEvents->append( MergeEvent(parent,child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeFromWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeWidgetEvents)
            this->UnmergeWidgetEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeFromWidget(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *MergeWithActionGroupEvents;
    QList<UnmergeEvent> *UnmergeFromActionGroupEvents;
    bool mergeWithActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeWithActionGroup(parent, child,
                                                          parentInfo, childInfo);
        if(this->MergeWithActionGroupEvents)
            this->MergeWithActionGroupEvents->append( MergeEvent(parent,child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeFromActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeFromActionGroupEvents)
            this->UnmergeFromActionGroupEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeFromActionGroup(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *MergeWithMenuEvents;
    QList<UnmergeEvent> *UnmergeFromMenuEvents;
    bool mergeWithMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeWithMenu(parent, child,
                                                          parentInfo, childInfo);
        if(this->MergeWithMenuEvents)
            this->MergeWithMenuEvents->append( MergeEvent(parent,child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeFromMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeFromMenuEvents)
            this->UnmergeFromMenuEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeFromMenu(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *MergeWithToolBarEvents;
    QList<UnmergeEvent> *UnmergeFromToolBarEvents;
    bool mergeWithToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeWithToolBar(parent, child,
                                                          parentInfo, childInfo);
        if(this->MergeWithToolBarEvents)
            this->MergeWithToolBarEvents->append( MergeEvent(parent,child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeFromToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeFromToolBarEvents)
            this->UnmergeFromToolBarEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeFromToolBar(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *MergeWithMenuBarEvents;
    QList<UnmergeEvent> *UnmergeFromMenuBarEvents;
    bool mergeWithMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::mergeWithMenuBar(parent, child,
                                                          parentInfo, childInfo);
        if(this->MergeWithMenuBarEvents)
            this->MergeWithMenuBarEvents->append( MergeEvent(qobject_cast<QObject*>(parent),
                                                             child,parentInfo,childInfo) );
        return success;
    }

    bool unmergeFromMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->UnmergeFromMenuBarEvents)
            this->UnmergeFromMenuBarEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::unmergeFromMenuBar(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *ActivateObjectEvents;
    QList<UnmergeEvent> *DeactivateObjectEvents;
    bool activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::activateObject(parent, child,
                                                          parentInfo, childInfo);
        if(this->ActivateObjectEvents)
            this->ActivateObjectEvents->append( MergeEvent(parent,
                                                             child,parentInfo,childInfo) );
        return success;
    }

    bool deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->DeactivateObjectEvents)
            this->DeactivateObjectEvents->append( UnmergeEvent(parent,child,
                                                            parentInfo,childInfo) );
        return GCF::GuiComponent::deactivateObject(parent, child,
                                                    parentInfo, childInfo);
    }

    QList<MergeEvent> *ActivateWidgetEvents;
    QList<UnmergeEvent> *DeactivateWidgetEvents;
    bool activateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        bool success = GCF::GuiComponent::activateWidget(parent, child,
                                                          parentInfo, childInfo);
        if(this->ActivateWidgetEvents)
            this->ActivateWidgetEvents->append( MergeEvent(qobject_cast<QObject*>(parent),
                                                           qobject_cast<QObject*>(child),
                                                           parentInfo,childInfo) );
        return success;
    }

    bool deactivateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        if(this->DeactivateWidgetEvents)
            this->DeactivateWidgetEvents->append( UnmergeEvent(qobject_cast<QObject*>(parent),
                                                               qobject_cast<QObject*>(child),
                                                               parentInfo,childInfo) );
        return GCF::GuiComponent::deactivateWidget(parent, child,
                                                    parentInfo, childInfo);
    }
};

#endif // TESTGUICOMPONENT_H
