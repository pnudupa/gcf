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

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include "GuiCommon.h"
#include "../Core/Component.h"

class QMenu;
class QAction;
class QWidget;
class QToolBar;
class QMenuBar;
class QActionGroup;

namespace GCF
{

struct GuiComponentData;
class GCF_GUI_EXPORT GuiComponent : public GCF::Component
{
public:
    GuiComponent(QObject *parent=0);

protected:
    ~GuiComponent();
    void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e);
    void contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e);
    void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e);
    void contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e);
    void activateContentObjectEvent(GCF::ActivateContentObjectEvent *e);
    void deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e);

    // Loading and unloading specialized object types
    virtual QWidget *loadWidget(const QString &name, const QVariantMap &info);
    virtual bool unloadWidget(const QString &name, QWidget *widget, const QVariantMap &info);
    virtual QAction *loadAction(const QString &name, const QVariantMap &info);
    virtual bool unloadAction(const QString &name, QAction *action, const QVariantMap &info);
    virtual QActionGroup *loadActionGroup(const QString &name, const QVariantMap &info);
    virtual bool unloadActionGroup(const QString &name, QActionGroup *actionGroup, const QVariantMap &info);
    virtual QMenu *loadMenu(const QString &name, const QVariantMap &info);
    virtual bool unloadMenu(const QString &name, QMenu *menu, const QVariantMap &info);
    virtual QMenuBar *loadMenuBar(const QString &name, const QVariantMap &info);
    virtual bool unloadMenuBar(const QString &name, QMenuBar *menuBar, const QVariantMap &info);
    virtual QToolBar *loadToolBar(const QString &name, const QVariantMap &info);
    virtual bool unloadToolBar(const QString &name, QToolBar *toolBar, const QVariantMap &info);

    // Merging and unmerging of specialized types
    bool mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    bool unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool mergeWithWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeFromWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool mergeWithActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeFromActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool mergeWithMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeFromMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool mergeWithToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeFromToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool mergeWithMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool unmergeFromMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);

    // Activation and deactivation
    bool activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    bool deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool activateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);
    virtual bool deactivateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo);

private:
    GuiComponentData *d;
};

}

#endif // GUICOMPONENT_H
