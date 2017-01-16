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

#include "GuiComponent.h"

#include <QDebug>

#include <QMenu>
#include <QLayout>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QGroupBox>
#include <QTextEdit>
#include <QMainWindow>
#include <QFormLayout>
#include <QActionGroup>
#include <QTableWidget>
#include <QWidgetAction>
#include <QStackedWidget>
#include <QPlainTextEdit>

/**
\class GCF::GuiComponent GuiComponent.h <GCF3/GuiComponent>
\brief Extends \ref GCF::Component to ease creation of GUI components
\ingroup gcf_gui


This class offers GUI (or widgets) friendly methods to make creation of GUI components
simple and straight forward. It is not mandatory for you to implement from this class
if you are writing GUI components. You can direclty subclass \ref GCF::Component.
However, inheriting from this class makes your code easier to comprehend and manage.
Plus you can leverage on all the GUI freebies that this class offers.

This class reimplements
\li \ref GCF::Component::contentObjectLoadEvent() - to identify special kinds of objects
like \c action, \c widget and calls specialized virtual functions for creation of
such objects.

\li \ref GCF::Component::contentObjectUnloadEvent() - to call specialized unload methods
based on object-type.

\li \ref GCF::Component::contentObjectMergeEvent() - to call specialized merge methods
based on the types of objects that are being merged.

\li \ref GCF::Component::contentObjectUnmergeEvent() - to call specialized unmerge methods
based on the types of objects that are being unmerged.

\li \ref GCF::Component::activateContentObjectEvent() - to call specialized activation methods
based on object type.

\li \ref GCF::Component::deactivateContentObjectEvent() - to call specialized deactivation methods
based on object type.

All of the specialized virtual methods called by the above event handlers have default implementations.
This means that the specialized virtual methods already know how to create one or more object types
and make your GUI component functional.

For instance, consider the content-file below
\code
<content>

    <!-- Create the top-level main-window -->
    <object name="window" type="widget" class="QMainWindow">
        <property>
            <key>windowTitle</key>
            <value>GCF::GuiComponent demo</value>
        </property>
    </object>

    <!-- Create text editor into the workspace of the mainwindow -->
    <object name="textEditor" type="widget" class="QTextEdit" parent="window">
        <property>
            <key>acceptRichText</key>
            <value>false</value>
        </property>
    </object>

    <!-- Create menu-bar -->
    <object name="menuBar" type="menuBar" />

    <!-- Create top-level menus -->
    <object name="fileMenu" type="menu" title="File" parent="menuBar" />
    <object name="editMenu" type="menu" title="Edit" parent="menuBar" />
    <object name="helpMenu" type="menu" title="Help" parent="menuBar" />

    <!-- Create actions -->
    <object name="fileNew" type="action" text="New" parent="fileMenu" />
    <object name="fileOpen" type="action" text="Open" parent="fileMenu" />
    <object name="fileSave" type="action" text="Save" parent="fileMenu" />
    <object name="fileSaveAs" type="action" text="Save As" parent="fileMenu" />
    <object name="fileClose" type="action" text="Close" parent="fileMenu" />
    <object name="fileExit" type="action" text="Exit"  parent="fileMenu" >
        <connection>
            <sender>triggered()</sender>
            <receiver>window::close()</receiver>
        </connection>
    </object>

    <object name="editCut" type="action" text="Cut" parent="editMenu">
        <connection>
            <sender>triggered()</sender>
            <receiver>textEditor::cut()</receiver>
        </connection>
    </object>
    <object name="editCopy" type="action" text="Copy" parent="editMenu">
        <connection>
            <sender>triggered()</sender>
            <receiver>textEditor::copy()</receiver>
        </connection>
    </object>
    <object name="editPaste" type="action" text="Paste" parent="editMenu">
        <connection>
            <sender>triggered()</sender>
            <receiver>textEditor::paste()</receiver>
        </connection>
    </object>

    <object name="helpAbout" type="action" text="About" parent="helpMenu" />

</content>
\endcode

Now when the above content-file is set for a GuiComponent (by reimplementing its
\ref GCF::Component::contentLoadEvent() and calling GCF::ContentLoadEvent::setContentFile()
to set the file where the above XML is stored); GCF constructs a main-window with file, edit
and help menus. It then creates a text-editor on the workspace area of the main-window. The
output would look as follows

\image html gui-component.png

*/

namespace GCF
{
struct GuiComponentData
{
    QWidget *createWidgetInstance(const QString &className);
};
}

/**
Constructor. Creates an instance of this class and inserts it as a child
under \c parent.

\note It is recommended that subclasses of \ref GCF::GuiComponent declare their
constructor as public and destructor as protected. This will ensure that the
components are created on the heap always.
 */
GCF::GuiComponent::GuiComponent(QObject *parent)
    : GCF::Component(parent)
{
    d = new GuiComponentData;
}

/**
Destructor.

\note It is recommended that subclasses of \ref GCF::GuiComponent declare their
constructor as public and destructor as protected. This will ensure that the
components are created on the heap always.
 */
GCF::GuiComponent::~GuiComponent()
{
    delete d;
}

/**
This event handler is implemented to look for a \c type key in the information map
(\ref GCF::ContentObjectLoadEvent::info()) provided by the event-object. Depending on the
type a specialized load function is called.

\li \c widget - \ref loadWidget()
\li \c action - \ref loadAction()
\li \c menu - \ref loadMenu()
\li \c actiongroup - \ref loadActionGroup()
\li \c toolbar - \ref loadToolBar()
\li \c menubar - \ref loadMenuBar()

For all other values of \c type, the base class implementation of the event handler is called,
which in-turn calls the \ref GCF::Component::loadObject() method.

\note the value of type is case-insensitive
*/
void GCF::GuiComponent::contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e)
{
    QString type = e->info().value("type").toString().toLower();

    if(type == "widget")
        e->setObject( this->loadWidget(e->objectName(), e->info()) );
    else if(type == "action")
        e->setObject( this->loadAction(e->objectName(), e->info()) );
    else if(type == "menu")
        e->setObject( this->loadMenu(e->objectName(), e->info()) );
    else if(type == "actiongroup")
        e->setObject( this->loadActionGroup(e->objectName(), e->info()) );
    else if(type == "toolbar")
        e->setObject( this->loadToolBar(e->objectName(), e->info()) );
    else if( type == "menubar" )
        e->setObject( this->loadMenuBar(e->objectName(), e->info()) );
    else
        GCF::Component::contentObjectLoadEvent(e);
}

/**
This event handler is implemented to look for the \c type key in parent information map,
which is accessible via \ref GCF::ContentObjectMergeEvent::parentInfo() method.
Depending on the parent type; a specialized merge function is called.

\li parent type \c widget - \ref mergeWithWidget(). Note if the child is not a widget, then
\ref GCF::Component::contentObjectMergeEvent() is called.
\li parent type \c actiongroup - \ref mergeWithActionGroup(). Note if the child is not a
\c QAction, then \ref GCF::Component::contentObjectMergeEvent() is called.
\li parent type \c menu - \ref mergeWithMenu()
\li parent type \c toolbar - \ref mergeWithToolBar()
\li parent type \c menubar - \ref mergeWithMenuBar()

For all other-values of \c type, the base class implementation of the event handler is called,
which in-turn calls the \ref GCF::Component::mergeObject() method.

\note the value of type is case-insensitive
*/
void GCF::GuiComponent::contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e)
{
    QString parentType = e->parentInfo().value("type").toString().toLower();

    if(parentType == "widget")
    {
        if(e->child()->isWidgetType())
            this->mergeWithWidget((QWidget*)e->parent(), (QWidget*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::contentObjectMergeEvent(e);
    }
    else if(parentType == "actiongroup")
    {
        if(qobject_cast<QAction*>(e->child()))
            this->mergeWithActionGroup((QActionGroup*)e->parent(), (QAction*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::contentObjectMergeEvent(e);
    }
    else if(parentType == "menu")
        this->mergeWithMenu((QMenu*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else if(parentType == "toolbar")
        this->mergeWithToolBar((QToolBar*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else if(parentType == "menubar")
        this->mergeWithMenuBar((QMenuBar*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else
        GCF::Component::contentObjectMergeEvent(e);
}

/**
This event handler is implemented to call a specialized unload function based on the \c type key
in the information map obtained by calling \ref GCF::ContentObjectUnloadEvent::info() on the event
object. Depending on the value of \c type, a specialized unload function is called.

\li \c widget - \ref unloadWidget()
\li \c action - \ref unloadAction()
\li \c menu - \ref unloadMenu()
\li \c actiongroup - \ref unloadActionGroup()
\li \c toolbar - \ref unloadToolBar()
\li \c menubar - \ref unloadMenuBar()

For all other-values of \c type, the base class implementation of the event handler is called,
which in-turn calls the \ref GCF::Component::unloadObject() method.

\note the value of type is case-insensitive
 */
void GCF::GuiComponent::contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e)
{
    QString type = e->info().value("type").toString().toLower();

    if(type == "widget")
        e->setAccepted( this->unloadWidget(e->objectName(), (QWidget*)e->object(), e->info()) );
    else if(type == "action")
        e->setAccepted( this->unloadAction(e->objectName(), (QAction*)e->object(), e->info()) );
    else if(type == "menu")
        e->setAccepted( this->unloadMenu(e->objectName(), (QMenu*)e->object(), e->info()) );
    else if(type == "actiongroup")
        e->setAccepted( this->unloadActionGroup(e->objectName(), (QActionGroup*)e->object(), e->info()) );
    else if(type == "toolbar")
        e->setAccepted( this->unloadToolBar(e->objectName(), (QToolBar*)e->object(), e->info()) );
    else if(type == "menubar")
        e->setAccepted( this->unloadMenuBar(e->objectName(), (QMenuBar*)e->object(), e->info()));
    else
        GCF::Component::contentObjectUnloadEvent(e);
}

/**
This event handler is implemented to look for the \c type key in parent information map,
which is accessible via \ref GCF::ContentObjectUnmergeEvent::parentInfo() method.
Depending on the parent type; a specialized merge function is called.

\li parent type \c widget - \ref unmergeWithWidget(). Note if the child is not a widget, then
\ref GCF::Component::contentObjectUnmergeEvent() is called.
\li parent type \c actiongroup - \ref unmergeWithActionGroup(). Note if the child is not a
\c QAction, then \ref GCF::Component::contentObjectUnmergeEvent() is called.
\li parent type \c menu - \ref unmergeWithMenu()
\li parent type \c toolbar - \ref unmergeWithToolBar()
\li parent type \c menubar - \ref unmergeWithMenuBar()

For all other-values of \c type, the base class implementation of the event handler is called,
which in-turn calls the \ref GCF::Component::unmergeObject() method.

\note the value of type is case-insensitive
 */
void GCF::GuiComponent::contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e)
{
    QString parentType = e->parentInfo().value("type").toString().toLower();

    if(parentType == "widget")
    {
        if(e->child()->isWidgetType())
            this->unmergeFromWidget((QWidget*)e->parent(), (QWidget*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::contentObjectUnmergeEvent(e);
    }
    else if(parentType == "actiongroup")
    {
        if(qobject_cast<QAction*>(e->child()))
            this->unmergeFromActionGroup((QActionGroup*)e->parent(), (QAction*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::contentObjectUnmergeEvent(e);
    }
    else if(parentType == "menu")
        this->unmergeFromMenu((QMenu*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else if(parentType == "toolbar")
        this->unmergeFromToolBar((QToolBar*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else if(parentType == "menubar")
        this->unmergeFromMenuBar((QMenuBar*)e->parent(), e->child(), e->parentInfo(), e->childInfo());
    else
        GCF::Component::contentObjectUnmergeEvent(e);
}

/**
This event handler is implemented to look for the \c type key in parent information map,
which is accessible via \ref GCF::ActivateContentObjectEvent::parentInfo() method.

If the \c type is \c widget, then the \ref activateWidget() method is called. Otherwise
the base-class implementation of this event handler is called, which in turn calls
\ref GCF::Component::activateObject()

\note the value of type is case-insensitive
 */
void GCF::GuiComponent::activateContentObjectEvent(GCF::ActivateContentObjectEvent *e)
{
    QString parentType = e->parentInfo().value("type").toString().toLower();

    if(parentType == "widget")
    {
        if(e->child()->isWidgetType())
            this->activateWidget((QWidget*)e->parent(), (QWidget*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::activateContentObjectEvent(e);
    }
    else
        GCF::Component::activateContentObjectEvent(e);
}

/**
This event handler is implemented to look for the \c type key in parent information map,
which is accessible via \ref GCF::DeactivateContentObjectEvent::parentInfo() method.

If the \c type is \c widget, then the \ref deactivateWidget() method is called. Otherwise
the base-class implementation of this event handler is called, which in turn calls
\ref GCF::Component::deactivateObject()

\note the value of type is case-insensitive
 */
void GCF::GuiComponent::deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e)
{
    QString parentType = e->parentInfo().value("type").toString().toLower();

    if(parentType == "widget")
    {
        if(e->child()->isWidgetType())
            this->deactivateWidget((QWidget*)e->parent(), (QWidget*)e->child(), e->parentInfo(), e->childInfo());
        else
            GCF::Component::deactivateContentObjectEvent(e);
    }
    else
        GCF::Component::deactivateContentObjectEvent(e);
}

/**
This virtual function may be implemented to return pointer to a widget based on \c name and
\c info parameters. The default implementation of this function does the following

\li Extracts value of the \c class key in \c info and creates an instance of the corresponding widget
class. Currently widgets for classes \c QMainWindow, \c QTabWidget, \c QStackedWidget, \c QGroupBox,
\c QTableWidget, \c QPlainTextEdit and \c QTextEdit are created. For all other class-names a \c QWidget
instance is created. In a future release we will make it possible to create appropriate instance of
any Qt widget.

\li Extracts the value of \c layout key in \c info and installs the corresponding layout on the widget.
Accepted values of \c layout key are \c vertical, \c horizontal, \c grid and \c form.

\li Extracts the value of \c geometry key in \c info and splits the value across comma (,). The split
list is considered to contain \c x, \c y, \c width and \c height values. This will then be set as
the geoemtry of the created widget.

\param name name of the widget to be loaded. This name would be set as the \c objectName
of the returned \c QWidget.
\param info information map, that provides some hints about the kind of widget that needs
to be loaded.
\return Pointer to a widget that got loaded.

\note The function always returns a valid \c QWidget pointer.

\note If you reimplement this function, please remember to call the default (this) implementation at
the end.
 */
QWidget *GCF::GuiComponent::loadWidget(const QString &name, const QVariantMap &info)
{
    QString wClass = info.value("class").toString();

    QWidget *widget = d->createWidgetInstance(wClass);
    widget->setObjectName(name);
    widget->setWindowTitle( gAppService->translate(info.value("windowtitle").toString()) );

    QLayout *layout = widget->layout();
    QString layoutType = info.value("layout", "vertical").toString();
    if(layout == 0 && !layoutType.isEmpty())
    {
        if(layoutType == "vertical")
            layout = new QVBoxLayout;
        else if(layoutType == "horizontal")
            layout = new QHBoxLayout;
        else if(layoutType == "grid")
            layout = new QGridLayout;
        else if(layoutType == "form")
            layout = new QFormLayout;
        widget->setLayout(layout);
    }

    QString geo = info.value("geometry").toString();
    if(!geo.isEmpty())
    {
        QStringList fields = geo.split(",", QString::SkipEmptyParts);
        int x = fields.count() >= 1 ? fields.at(0).toInt() : 0;
        int y = fields.count() >= 2 ? fields.at(1).toInt() : 0;
        int width = fields.count() >= 3 ? fields.at(2).toInt() : widget->sizeHint().width();
        int height = fields.count() >= 4 ? fields.at(3).toInt() : widget->sizeHint().height();
        widget->setGeometry(x, y, width, height);
    }

    return widget;
}

/**
This virtual function can be implemented to unload a widget. The default implementation
ignores \c name and \c info, and simply deletes the \c widget.

\param name name of the widge that needs to be unloaded
\param widget pointer to the \c QWidget that needs to be unloaded
\param info information key=value map associated with the widget that needs to be unloaded
\return true upon successful unloading of the widget, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadWidget(const QString &name, QWidget *widget, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete widget;
    return true;
}

/**
This virtual function can be reimplemented to load a \c QAction object based on \c name and \c info.
The default implementation creates a new \c QAction instance and sets its \c text and \c tooltip values
by reading from the \c text and \c tooltip keys from \c info.

\param name name of the action to load
\param info information key=value map of the action that needs to be loaded
\return pointer to a \c QAction instance

\note the default implementation always returns a valid \c QAction pointer.
*/
QAction *GCF::GuiComponent::loadAction(const QString &name, const QVariantMap &info)
{
    QAction *action = new QAction(this);
    action->setObjectName(name);
    action->setText( gAppService->translate( info.value("text").toString() ) );
    action->setToolTip( gAppService->translate( info.value("tooltip").toString() ) );
    return action;
}

/**
This virtual function can be implemented to unload an action. The default implementation
ignores \c name and \c info, and simply deletes the \c action.

\param name name of the action that needs to be unloaded
\param action pointer to the \c QAction that needs to be unloaded
\param info information key=value map associated with the action that needs to be unloaded
\return true upon successful unloading of the action, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadAction(const QString &name, QAction *action, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete action;
    return true;
}

/**
This virtual function can be reimplemented to load a \c QActionGroup object based on \c name and \c info.
The default implementation creates a new \c QActionGroup instance and sets its \c exclusive property
by reading the \c exclusive key from \c info

\param name name of the action group to load
\param info information key=value map of the action group that needs to be loaded
\return pointer to a \c QActionGroup instance

\note the default implementation always returns a valid \c QActionGroup pointer.
*/
QActionGroup *GCF::GuiComponent::loadActionGroup(const QString &name, const QVariantMap &info)
{
    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->setObjectName(name);
    actionGroup->setExclusive( info.value("exclusive").toBool() );
    return actionGroup;
}

/**
This virtual function can be implemented to unload an action group. The default implementation
ignores \c name and \c info, and simply deletes the \c actionGroup.

\param name name of the action group that needs to be unloaded
\param action pointer to the \c QActionGroup that needs to be unloaded
\param info information key=value map associated with the action group that needs to be unloaded
\return true upon successful unloading of the action group, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadActionGroup(const QString &name, QActionGroup *actionGroup, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete actionGroup;
    return true;
}

/**
This virtual function can be reimplemented to load a \c QMenu object based on \c name and \c info.
The default implementation creates a new \c QMenu instance and sets its \c text and \c tooltip values
by reading from the \c text and \c tooltip keys from \c info.

\param name name of the menu to load
\param info information key=value map of the menu that needs to be loaded
\return pointer to a \c QMenu instance

\note the default implementation always returns a valid \c QMenu pointer.
*/
QMenu *GCF::GuiComponent::loadMenu(const QString &name, const QVariantMap &info)
{
    QMenu *menu = new QMenu;
    menu->setObjectName(name);
    menu->setTitle( gAppService->translate( info.value("title").toString() ) );
    menu->setToolTip( gAppService->translate( info.value("tooltip").toString() ) );
    return menu;
}

/**
This virtual function can be implemented to unload a menu. The default implementation
ignores \c name and \c info, and simply deletes the \c menu.

\param name name of the menu that needs to be unloaded
\param action pointer to the \c QMenu that needs to be unloaded
\param info information key=value map associated with the menu that needs to be unloaded
\return true upon successful unloading of the menu, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadMenu(const QString &name, QMenu *menu, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete menu;
    return true;
}

/**
This virtual function can be reimplemented to load a \c QMenuBar object based on \c name and \c info.
The default implementation ignores \c name and \c info and creates a new \c QMenuBar instance.

\param name name of the menu-bar to load
\param info information key=value map of the menu-bar that needs to be loaded
\return pointer to a \c QMenuBar instance

\note the default implementation always returns a valid \c QMenuBar pointer.
*/
QMenuBar *GCF::GuiComponent::loadMenuBar(const QString &name, const QVariantMap &info)
{
    QMenuBar *menuBar = new QMenuBar;
    menuBar->setObjectName(name);
    Q_UNUSED(info);
    return menuBar;
}

/**
This virtual function can be implemented to unload a menu-bar. The default implementation
ignores \c name and \c info, and simply deletes the \c menuBar.

\param name name of the menu-bar that needs to be unloaded
\param action pointer to the \c QMenuBar that needs to be unloaded
\param info information key=value map associated with the menu-bar that needs to be unloaded
\return true upon successful unloading of the menu-bar, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadMenuBar(const QString &name, QMenuBar *menuBar, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete menuBar;
    return true;
}

/**
This virtual function can be reimplemented to load a \c QToolBar object based on \c name and \c info.
The default implementation creates a new \c QToolBar instance and sets it \c title property
by reading the \c title key from \c info

\param name name of the tool-bar to load
\param info information key=value map of the tool-bar that needs to be loaded
\return pointer to a \c QToolBar instance

\note the default implementation always returns a valid \c QToolBar pointer.
*/
QToolBar *GCF::GuiComponent::loadToolBar(const QString &name, const QVariantMap &info)
{
    QString title = gAppService->translate( info.value("title").toString() );
    QToolBar *toolBar = new QToolBar(title);
    toolBar->setObjectName(name);
    toolBar->setToolTip( gAppService->translate( info.value("tooltip").toString() ) );
    return toolBar;
}

/**
This virtual function can be implemented to unload a tool-bar. The default implementation
ignores \c name and \c info, and simply deletes the \c toolBar.

\param name name of the tool-bar that needs to be unloaded
\param action pointer to the \c QToolBar that needs to be unloaded
\param info information key=value map associated with the tool-bar that needs to be unloaded
\return true upon successful unloading of the tool-bar, false otherwise.

\note the default implementation always returns true
 */
bool GCF::GuiComponent::unloadToolBar(const QString &name, QToolBar *toolBar, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete toolBar;
    return true;
}

/**
This virtual function is reimplemented from \ref GCF::Component to handle the case where \c child
is a layout and \c parent is a widget. For all other cases the base class implementation is called.

In the case where \c child is a layout and \c parent is a widget, the layout is set on the widget
deleting any previous layout.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be merged (or parented) into \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
 */
bool GCF::GuiComponent::mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    /*
     * If parent is a QWidget and child is a QLayout, then we will install the layout.
     */
    if(parent->isWidgetType() && qobject_cast<QLayout*>(child))
    {
        QWidget *parentWidget = (QWidget*)parent;
        QLayout *layout = (QLayout*)child;
        if(parentWidget->layout())
        {
            GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                          QString("Widget %1(%2) already has a layout. Replacing it with %3(%4)")
                                          .arg(parent->metaObject()->className())
                                          .arg(parent->objectName())
                                          .arg(child->metaObject()->className())
                                          .arg(child->objectName()));
            delete parentWidget->layout();
        }

        parentWidget->setLayout(layout);
        return true;
    }

    return GCF::Component::mergeObject(parent, child, parentInfo, childInfo);
}

/**
This function is reimplemented from \ref GCF::Component to handle the case where \c child is a layout
and \c parent is a widget. In such a case the \c child layout will be deleted and the \parent widget
will be left with no layout. For all other cases the base class implementation is called.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be unmerged (or parented) into \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
 */
bool GCF::GuiComponent::unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    /*
     * If parent is a QWidget and child is a QLayout, then we will uninstall the layout.
     */
    if(parent->isWidgetType() && qobject_cast<QLayout*>(child))
    {
        QWidget *parentWidget = (QWidget*)parent;
        delete parentWidget->layout();
        return true;
    }

    return GCF::Component::unmergeObject(parent, child, parentInfo, childInfo);
}

/**
This virtual function can be reimplemented to customize the way in which a \c child widget is parented
into the \c parent widget. The default implementation makes use of the following rules while performing
the merge.

\li if \c child is a \c QMenuBar, then it is set as a menu-widget in the \c parent's widget.

\li if \c parent is a \c QTabWidget, then the \c child is added as a tab; with the value of \c title
from the \c childInfo as tab-title.

\li if \c parent is a \c QStackedWidget, then the \c child is added as a widget to it.

\li if \c parent is a \c QMainWindow, then the \c child is added as a widget to the central-widget. If
no such central widget exist; then a default central widget with vertical layout is created. The \c child
is then added to the newly created central widget.

\li if the \c parent is a \c QWidget and has a layout, then the \c child is added to the layout. Value
of the \c layoutposition key in \c childInfo can provide hints about the insert-index. If \c parent
has a form layout, then the label for \c child will be picked up from \c labeltext key of \c childInfo.

\li if \c parent does not have any layout associated with it; then \c child is reparented to \c parent.
Position of the \c child within \c parent will be detemrined by the \c geometry key of \c childInfo.

\param parent pointer to the parent \c QWidget into which the child should be merged
\param child pointer to the child \c QWidget that needs merging
\param parentInfo pointer to the information key=value map associated with the \c parent
\param childInfo pointer to the information key=value map associated with the \c child
\return true on success, false otherwise. (The default implementation always returns true)
 */
bool GCF::GuiComponent::mergeWithWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    /*
     * If the parent is a QWidget and child is a menu-bar; then we will
     * install the child as a menu-bar of the widget's layout. We will create
     * a simple vertical layout - if none exists!
     */
    QMenuBar *childMenuBar = qobject_cast<QMenuBar*>(child);
    if(childMenuBar)
    {
        QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parent);
        if(mainWindow)
        {
            mainWindow->setMenuBar(childMenuBar);
            return true;
        }

        if(parent->layout())
            parent->layout()->setMenuBar(childMenuBar);
        else
        {
            GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                       QString("Creating a vertical layout for %1(%2) and setting %3(%4) as its menu-bar")
                                       .arg(parent->metaObject()->className())
                                       .arg(parent->objectName())
                                       .arg(child->metaObject()->className())
                                       .arg(child->objectName()));

            parent->setLayout(new QVBoxLayout);
            parent->layout()->setMenuBar(childMenuBar);
        }

        return true;
    }

    /*
     * If the parent is a tab-widget, then we can simply add the child widget
     * as a new tab.
     */
    QTabWidget *tabWidget = qobject_cast<QTabWidget*>(parent);
    if(tabWidget)
    {
        QString tabText = childInfo.value("tabtext").toString();
        if(tabText.isEmpty())
            tabText = childInfo.value("windowtitle").toString();
        if(tabText.isEmpty())
            tabText = child->windowTitle();
        if(tabText.isEmpty())
            tabText = QString("Tab %1").arg(tabWidget->count()+1);

        tabWidget->addTab(child, gAppService->translate(tabText));
        return true;
    }

    /*
     * If the parent is a stacked widget, then we simply add the child widget
     * as a stacked-content.
     */
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(parent);
    if(stackedWidget)
    {
        stackedWidget->addWidget(child);
        return true;
    }

    /*
     * If the parent is a main-window, then we work with the workspace only
     */
    // FIXME: #pragma message("Test merge with QMainWindow workspace")
    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parent);
    if(mainWindow)
    {
        if(!mainWindow->centralWidget())
        {
            mainWindow->setCentralWidget(new QWidget);
            mainWindow->centralWidget()->setLayout(new QVBoxLayout);
        }

        parent = mainWindow->centralWidget();
    }

    /*
     * At this point, we will simply add child to the parent. The only additional
     * value we can provide at this point is to add the child into the layout of the parent
     */
    if(parent->layout())
    {
        QString layoutPos = childInfo.value("layoutposition").toString();
        QStringList fields = layoutPos.split(",", QString::SkipEmptyParts);
        if(fields.isEmpty())
            fields.append("-1");

        if( qobject_cast<QBoxLayout*>(parent->layout()) )
        {
            int stretch = qMax( fields.count() >= 2 ? fields.at(1).toInt() : -1, 0 );
            QBoxLayout *boxLayout = (QBoxLayout*)parent->layout();
            boxLayout->insertWidget( fields.first().toInt(), child, stretch );
        }
        else if( qobject_cast<QGridLayout*>(parent->layout()) )
        {
            QGridLayout *gridLayout = (QGridLayout*)parent->layout();
            int row = qMax( fields.at(0).toInt(), 0 );
            int col = qMax( fields.count() >= 2 ? fields.at(1).toInt() : -1, 0 );
            int rowSpan = qMax( fields.count() >= 3 ? fields.at(2).toInt() : -1, 1 );
            int colSpan = qMax( fields.count() >= 4 ? fields.at(3).toInt() : -1, 1 );
            gridLayout->addWidget(child, row, col, rowSpan, colSpan);
        }
        else if( qobject_cast<QFormLayout*>(parent->layout()) )
        {
            QString label = childInfo.value("labeltext").toString();
            if(label.isEmpty())
                label = childInfo.value("text").toString();
            if(label.isEmpty())
                label = childInfo.value("windowtitle").toString();
            if(label.isEmpty())
                label = child->windowTitle();
            if(label.isEmpty())
                label = "untitled";

            QFormLayout *formLayout = (QFormLayout*)parent->layout();
            formLayout->insertRow(fields.at(0).toInt(), label, child);
        }
        else
            parent->layout()->addWidget(child);

        child->setVisible(childInfo.value("visible", true).toBool());

        return true;
    }

    child->setParent(parent);
    child->setVisible(childInfo.value("visible", true).toBool());

    return true;
}

/**
Thi function can be implemented to customize the way in which \c child is unmerged (or unparented) from
\c parent. The default implementation removes \c child from the \c parent.

\param parent pointer to the parent \c QWidget from which the child should be unmerged
\param child pointer to the child \c QWidget that needs unmerging
\param parentInfo pointer to the information key=value map associated with the \c parent
\param childInfo pointer to the information key=value map associated with the \c child
\return true on success, false otherwise. (The default implementation always returns true)
 */
bool GCF::GuiComponent::unmergeFromWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    if(parent->layout())
        parent->layout()->removeWidget(child);

    if(!qobject_cast<QMenuBar*>(child))
        child->setParent(0);

    return true;
}

/**
This function can be implemented to customize the way in which \c child action is added to the \c parent action-group.
The default implementation simply adds the action to the action group.

\param parent pointer to the \c QActionGroup into which an action needs to be added
\param child pointer to the \c QAction that needs to be added
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true on success, false otherwise. (The default implementation returns true always)
 */
bool GCF::GuiComponent::mergeWithActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    parent->addAction( (QAction*)child );
    return true;
}

/**
This function can be implemented to customize the way in which \c child action is removed from the
\c parent action-group. The default implementation simply removes the action from the action group.

\param parent pointer to the \c QActionGroup from which an action needs to be removed
\param child pointer to the \c QAction that needs to be removed
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true on success, false otherwise. (The default implementation returns true always)
 */
bool GCF::GuiComponent::unmergeFromActionGroup(QActionGroup *parent, QAction *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if( qobject_cast<QAction*>(child) )
    {
        parent->removeAction( (QAction*)child );
        return true;
    }

    return false;
}

/**
This function can be implemented to customize the way in which \c child is added to the \c parent menu.
The default implementation
\li adds \c child as an action to \c parent, if \c child is a \c QAction
\li adds \c child as sub-menu to \c parent, if \c child is a \c QMenu
\li adds \c child as widget-action to \c parent, if \c child is a \c QWidget

\param parent pointer to the \c QMenu into which the child needs to be added
\param child pointer to the \c QObject that needs to be added
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QAction, \c QMenu or \c QWidget. False otherwise.
 */
bool GCF::GuiComponent::mergeWithMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if( qobject_cast<QAction*>(child) )
    {
        parent->addAction( (QAction*)child );
        return true;
    }

    if( qobject_cast<QMenu*>(child) )
    {
        parent->addMenu( (QMenu*)child );
        return true;
    }

    if( child->isWidgetType() )
    {
        QWidgetAction *widgetAction = new QWidgetAction(parent);
        widgetAction->setDefaultWidget( (QWidget*)child );
        parent->addAction(widgetAction);
        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to add %1(%2) to menu %3")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function can be implemented to customize the way in which \c child is removed from the \c parent menu.
The default implementation
\li removes \c child as an action from \c parent, if \c child is a \c QAction
\li removes \c child as sub-menu from \c parent, if \c child is a \c QMenu
\li removes \c child as widget-action from \c parent, if \c child is a \c QWidget

\param parent pointer to the \c QMenu from which the child needs to be removed
\param child pointer to the \c QObject that needs to be removed
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QAction, \c QMenu or \c QWidget. False otherwise.
 */
bool GCF::GuiComponent::unmergeFromMenu(QMenu *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if( qobject_cast<QAction*>(child) )
    {
        parent->removeAction( (QAction*)child );
        return true;
    }

    if( qobject_cast<QMenu*>(child) )
    {
        parent->removeAction( ((QMenu*)child)->menuAction() );
        return true;
    }

    if( child->isWidgetType() )
    {
        QList<QWidgetAction*> widgetActions = parent->findChildren<QWidgetAction*>();
        Q_FOREACH(QWidgetAction *act, widgetActions)
        {
            if(act->defaultWidget() == (QWidget*)child )
            {
                parent->removeAction(act);
                delete act;
                break;
            }
        }

        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to remove %1(%2) from menu %3")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function can be implemented to customize the way in which \c child is added to the \c parent toolbar.
The default implementation
\li adds \c child as an action to \c parent, if \c child is a \c QAction
\li adds \c child as an menu to \c parent, if \c child is a \c QMenu

\param parent pointer to the \c QToolBar into which the child needs to be added
\param child pointer to the \c QObject that needs to be added
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QAction or \c QMenu. False otherwise.
 */
bool GCF::GuiComponent::mergeWithToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if( qobject_cast<QAction*>(child) )
    {
        parent->addAction( (QAction*)child );
        return true;
    }

    if( qobject_cast<QMenu*>(child) )
    {
        parent->addAction( ((QMenu*)child)->menuAction() );
        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to add %1(%2) to toolbar %3")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function can be implemented to customize the way in which \c child is removed from the \c parent toolbar.
The default implementation
\li removes \c child as an action from \c parent, if \c child is a \c QAction
\li removes \c child as a menu from \c parent, if \c child is a \c QMenu

\param parent pointer to the \c QToolBar from which the child needs to be added
\param child pointer to the \c QObject that needs to be added
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QAction or \c QMenu. False otherwise.
 */
bool GCF::GuiComponent::unmergeFromToolBar(QToolBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    if( qobject_cast<QAction*>(child) )
    {
        parent->removeAction( (QAction*)child );
        return true;
    }

    if( qobject_cast<QMenu*>(child) )
    {
        parent->removeAction( ((QMenu*)child)->menuAction() );
        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to remove %1(%2) from toolbar %3")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function can be implemented to customize the way in which \c child is added to the \c parent menu-bar.
The default implementation adds \c child as a menu to \c parent, if \c child is an instance of \c QMenu.

\param parent pointer to the \c QMenuBar into which the child needs to be added
\param child pointer to the \c QObject that needs to be added
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QMenu. False otherwise.
 */
bool GCF::GuiComponent::mergeWithMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    QMenu *childMenu = qobject_cast<QMenu*>(child);
    if(childMenu)
    {
        parent->addMenu(childMenu);
        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to insert %1(%2) into QMenuBar(%3)")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function can be implemented to customize the way in which \c child is removed from the \c parent menu-bar.
The default implementation removes \c child as a menu from \c parent, if \c child is an instance of \c QMenu.

\param parent pointer to the \c QMenuBar from which the child needs to be removed
\param child pointer to the \c QObject that needs to be removed
\param parentInfo information key=value map associated with \c parent
\param childInfo information key=value map associated with \c child
\return true if \c child was a \c QMenu. False otherwise.
 */
bool GCF::GuiComponent::unmergeFromMenuBar(QMenuBar *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);

    QMenu *childMenu = qobject_cast<QMenu*>(child);
    if(childMenu)
    {
        parent->removeAction(childMenu->menuAction());
        return true;
    }

    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                  QString("Attempting to remove %1(%2) from QMenuBar(%3)")
                                  .arg(child->metaObject()->className())
                                  .arg(child->objectName())
                                  .arg(parent->objectName()));
    return false;
}

/**
This function is reimplemented from \ref GCF::Component to set the \c enabled property of \c child
as true, if \c child is an instance of \c QAction. The base class implementation is called for
all other cases.
 */
bool GCF::GuiComponent::activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    if( qobject_cast<QAction*>(child) )
    {
        child->setProperty("enabled", true);
        return true;
    }

    return GCF::Component::activateObject(parent, child, parentInfo, childInfo);
}

/**
This function is reimplemented from \ref GCF::Component to set the \c enabled property of \c child
as false, if \c child is an instance of \c QAction. The base class implementation is called for
all other cases.
 */
bool GCF::GuiComponent::deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    if( qobject_cast<QAction*>(child) )
    {
        child->setProperty("enabled", false);
        return true;
    }

    return GCF::Component::deactivateObject(parent, child, parentInfo, childInfo);
}

/**
This function can be reimplemented to customize the way in which \c child is activated in \c parent,
based on information provided by \c parentInfo and \c childInfo. The default implementation sets
the \c enabled property of \c child as true. The default implementation always returns true.
 */
bool GCF::GuiComponent::activateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parent);
    child->setEnabled(true);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    return true;
}

/**
This function can be reimplemented to customize the way in which \c child is deactivated in \c parent,
based on information provided by \c parentInfo and \c childInfo. The default implementation sets
the \c enabled property of \c child as false. The default implementation always returns false.
 */
bool GCF::GuiComponent::deactivateWidget(QWidget *parent, QWidget *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_UNUSED(parent);
    child->setEnabled(false);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    return true;
}

QWidget *GCF::GuiComponentData::createWidgetInstance(const QString &className)
{
    // TODO: Update this function to include other important Qt widgets as well.
    static QStringList classNames = QStringList()
            << "QMainWindow"
            << "QTabWidget"
            << "QStackedWidget"
            << "QGroupBox"
            << "QTableWidget"
            << "QPlainTextEdit"
            << "QTextEdit";

    // FIXME: #pragma message("Test addition of QPlainTextEdit, QTextEdit")
    switch( classNames.indexOf(className) )
    {
    case 0: return new QMainWindow;
    case 1: return new QTabWidget;
    case 2: return new QStackedWidget;
    case 3: return new QGroupBox;
    case 4: return new QTableWidget;
    case 5: return new QPlainTextEdit;
    case 6: return new QTextEdit;
    default: break;
    }

    if(!className.isEmpty())
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                      QString("Dont know how to instantiate widget of class %1")
                                      .arg(className));

    return new QWidget;
}
