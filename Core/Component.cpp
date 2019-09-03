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

#include "Component.h"
#include "Log.h"
#include "ObjectTree.h"

#include <QSettings>

/**
\class GCF::Component Component.h <GCF3/Component>
\brief Base class for all GCF components
\ingroup gcf_core


This class is at the heart of GCF's component model. A component, in GCF, is an
entity that makes available one or more objects that can work in tandem with
objects offered by other components in the application. Components are generally
written for reusability. They can be written once and used in several different
application contexts.

The \ref GCF::Component class offers a unified way to represent components,
initialize (or load) and uninitialize (or unload) them in an application. Objects
offered by components are registered with the application's object-tree
( \ref GCF::ApplicationServices::objectTree() ). This way, any component in the
application can easily search for objects (and services) offered by any other
component in the application.

To create your own component, you can subclass from \ref GCF::Component and
implement your component's behavior. Example:

\code
class MyComponent : public GCF::Component
{
public:
    MyComponent(QObject *parent=0) : GCF::Component(parent) { }

protected:
    ~MyComponent() { }
};

GCF_EXPORT_COMPONENT(MyComponent);
\endcode

A couple of key points must be noticed from the code snippet above

\li Constructor must be public, destructor must be protected. This is to ensure
that the component can only be created on the heap and never on the stack.

\li \c GCF_EXPORT_COMPONENT macro can be used to export the component class
across shared-library boundaries. This should be done in a source file, and not
in a header file.

\section gcf_component_instantiation Instantiating components

A component is said to be instantiated if an instance of the component is created
in memory. An instantiated component is however not useful and not included into
the application. For that you will need to load the compoent. A loaded component
is initialized and its objects are included in the application. Read the following
sections to know more about this.

Components can be instantiated using any of the following means
\li By loading the component from a shared library
\li By creating an instance of a component class

To load a component from shared library, you can make use of the
\ref GCF::ApplicationServices::loadComponent() function. Example:

\code
#include <GCF3/Application>
#include <GCF3/Component>

int main(int argc, char **argv)
{
    GCF::Application a(argc, argv);

    GCF::Component *component = gAppService->loadComponent("GDrive/GDriveLite");
    if(component)
    {
        // The component "GDrive/GDriveLite" has been instantiated
        // and initialized
    }

    return a.exec();
}
\endcode

If you want to instantiate the component from a shared library and load it later (explicitly); then
you can use the \ref GCF::ApplicationServices::instantiateComponent() function. Example:

\code
#include <GCF3/Application>
#include <GCF3/Component>

int main(int argc, char **argv)
{
    GCF::Application a(argc, argv);

    GCF::Component *component = gAppService->instantiateComponent("GDrive/GDriveLite");
    if(component)
    {
        // The component "GDrive/GDriveLite" has been instantiated
        // but __not__ initialized.

        ...
        ...
        ...
        component->load();
    }

    return a.exec();
}
\endcode

If you have access to the constructor of any subclass of GCF::Component, then you can
instantiate the component using the \c new operator of C++ and then call the
\ref load() method on the component to load the component.

\section gcf_component_loading Component loading (or initialization)

One of the ways to load a component is by making use of the \ref load() method. Example:

\code
GCF::Component *comp = ....;
comp->load();

// By the time the load() function returns, the component is
// completely initialized and its content loaded.
\endcode

While a component is being loaded, it receives one or more events. These
events are handled by the \ref event() method. As a component developer you can
reimplement event handlers from this class and offer your own code. Following is
the order in which events are despatched to the component (upon load)

\htmlonly
<table border="0" cellpadding="5">
<tr><td>
\endhtmlonly

\li \b Pre-Initialization: Read documentation of \ref GCF::Component::initializeEvent()
for more information.

\li \b Pre-SettingsLoad: Read documentation of \ref GCF::Component::settingsLoadEvent()
for more information.

\li \b Post-SettingsLoad: Read documentation of \ref GCF::Component::settingsLoadEvent()
for more information.

\li \b Pre-ContentLoad: Read documentation of \ref GCF::Component::contentLoadEvent()
for more information.

\li For each object loaded by the content-file, a \b GCF::ContentObjectLoadEvent is sent.
Read documentation of \ref GCF::Component::contentObjectLoadEvent() for more information.
And if any of the loaded content-objects, request for merge with content-objects from other
components then \b GCF::ContentObjectMergeEvent event is sent. Read documentation of
\ref GCF::Component::contentObjectMergeEvent() for more information.

\li \b Post-ContentLoad: Read documentation of \ref GCF::Component::contentLoadEvent()
for more information.

\li \b Post-Initialization: Read documentation of \ref GCF::Component::initializeEvent()
for more information.

\li \b Pre-Activation: Read documentation of \ref GCF::Component::activationEvent() for
more information. After the pre-activation event is handled by the component, each and
every content object of the component is activated.

\li \b Post-Activation: Read documentation of \ref GCF::Component::activationEvent() for
more information.

Once the post-initialization event has been handled; the component is considered to have been
initialized.

\htmlonly
</td><td>
\endhtmlonly

\image html gcf-component-load-events.png

\htmlonly
</td></tr></table>
\endhtmlonly

\sa \ref gcf_component_model_5

\section gcf_component_unloading Component unloading (or uninitialization)

One of the ways to unload a component is to make use of the \ref unload() method. Example:

\code
GCF::Component *comp = ....;
comp->unload();

// By the time the unload() function returns, the component is
// completely unloaded and the component object is deleted.

comp = 0;
\endcode

When a component is being to unloading, it receives one or more events. These
events are handled by the \ref event() method. As a component developer you can
reimplement event handlers from this class and offer your own code. Following is
the order in which events are despatched to the component (upon unload)

\htmlonly
<table border="0" cellpadding="5">
<tr><td>
\endhtmlonly

\li \b Pre-Deactivation: Read \ref GCF::Component::deactivationEvent() for more information.
After pre-deactivation is handled, each and every content object is deactivated.

\li \b Post-Deactivation: Read \ref GCF::Component::deactivateObject() for more information.

\li \b Pre-Finalize: Read \ref GCF::Component::finalizeEvent() for more inforamtion.

\li \b Pre-ContentUnload: Read \ref GCF::Component::contentUnloadEvent() for more information.

\li For each content-object loaded during unload(), a \ref GCF::ContentObjectUnmergeEvent
event is first sent for unmerging the object ( \ref GCF::Component::contentObjectUnmergeEvent() )
and \c GCF::ContentObjectUnloadEvent is sent for unloading the object
( \ref GCF::Component::contentObjectUnloadEvent() )

\li \b Post-ContentUnload: Read \ref GCF::Component::contentUnloadEvent() for more information.

\li \b Pre-SettingsUnload: Read \ref GCF::Component::settingsUnloadEvent() for more information.

\li \b Post-SettingsUnload: Read \ref GCF::Component::settingsUnloadEvent() for more information.

\li \b Post-Finalize: Read \ref GCF::Component::finalizeEvent() for more information.

After the post-finalize event is handled, the component object is deleted.

\htmlonly
</td><td>
\endhtmlonly

\image html gcf-component-unload-events.png

\htmlonly
</td></tr></table>
\endhtmlonly

\sa \ref gcf_component_model_5_5

\section gcf_component_contentfile Content File

Each component can (optionally) provide a content file listing the content-objects offered by it.

A component specifies the content-file that it uses via the \ref GCF::ContentLoadEvent class when
its \ref GCF::Component::contentLoadEvent() is called. A typical implementation of this event handler
would be

\code
void ComponentClass::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
        e->setContentFile(....);
}
\endcode

A content file is written in XML. At the most basic level, a content file has a root \c content XML
element and one or more \c object XML elements under it. Example:

\code
<content>

    <object name="object1" key11="value1" key12="value2" key13="value3" />
    <object name="object2" key21="value1" key22="value2" key23="value3" />
    ...
    ...
    ...
    <object name="objectn" keyn1="value1" keyn2="value2" keyn3="value3" />

</content>
\endcode

Whenever an \c object element is parsed by GCF, it sends a \ref GCF::ContentObjectLoadEvent event
to the component. Whatever object is created and reported via the event is associated with that name.
Example:

\code
void ComponentClass::contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e)
{
    if(e->objectName() == "object1")
    {
        QObject *object = .....; // create the object
        e->setObject(object);
        return;
    }

    if(e->objectName() == "object2")
    {
        QObject *object = .....; // create the object
        e->setObject(object);
        return;
    }

    .....
    .....
    .....
    .....

    if(e->objectName() == "objectn")
    {
        QObject *object = .....; // create the object
        e->setObject(object);
        return;
    }
}
\endcode

You could also optionally do away with implementation of \c contentObjectLoadEvent() and simply
implement the \ref GCF::Component::loadObject() to behave like a factory method to create
\c QObject instances based on the name. This is because the default implementation of
\ref GCF::Component::contentObjectLoadEvent() calls the \ref GCF::Component::loadObject()

\code
QObject *ComponentClass::loadObject(const QString &name, const QVariantMap &info)
{
    if(name == "object1")
        return .....; // create the object

    if(name == "object2")
        return .....; // create the object

    .......
    .......
    .......
    .......

    if(name == "objectn")
        return .....; // create the object
}
\endcode

The \c info parameter passed to the \c loadObject() method or accessed via the
\ref GCF::ContentObjectLoadEvent::info() method on the event-object, contains all key=value
pairs provided in the \c object XML element.

You can also configure object properties and create signal/slot connections in the
content-xml file. Read \ref gcf_content_xml for more information.

\section gcf_component_activation Activation

Components can be activated by calling the \ref activate() method. When this method is called
GCF sends a \ref GCF::ActivationEvent event to the component, which can be handled by reimplementing
the \ref activationEvent() virtual function. The activation event is actually sent twice to the
component. GCF first sends a pre-activation event to the component. After this event is handled,
it sends a \ref GCF::ActivateContentObjectEvent event to the component against each of its content
objects. This event can be handled by reimplementing the \ref activateContentObjectEvent() function.
Finally, GCF sends a post-activtion event to the component.

By default components are activated at the end of \ref load().

\section gcf_component_deactivation Deactivation

Just as with \ref gcf_component_activation "component activation", a component can be deactivated
by calling the \ref deactivate() method. When this method is called, GCF sends a
\ref GCF::DeactivationEvent event to the component, which can be handled by reimplementing the
\ref deactiationEvent() virtual function. The deactivation is sent twice to the component.
GCF first sends a pre-deactivation event. After that it sends a \ref GCF::DeactivateContentObjectEvent
event to the component against each of its content objects. This event can be handled by reimplementing
the \ref deactivateContentObjectEvent() function. Finally, GCF sends a post-deactivtion event to the component.

By default components are deactivated at the start of \ref unload().

\section gcf_component_objectTree Application Object Tree

Objects loaded from the parsing of a content-file is registered with the application's object tree
accessible via \c gAppService->objectTree(). This part is explained in the following video

\htmlonly
<video width="500" controls>
    <source src="../../../Videos/Concepts/Component-Model.mp4" type="video/mp4">
</video>
<p>If you are unable to view the embedded video above, please click <a href="http://player.vimeo.com/video/77953239" target="_blank">here</a> to view it on the web.</p>
\endhtmlonly

There are more ways in which objects can be registered with the application and included in the
application object tree. Please read the section on \ref gcf_component_model_6 to know more about this.

*/

namespace GCF
{

struct ComponentData
{
    ComponentData() : settings(0), currentEvent(0) { }

    QSettings *settings;
    QEvent *currentEvent;
};

}

/**
 * Constructor for instantiating the component.
 *
 * \sa \ref gcf_component_instantiation
 */
GCF::Component::Component(QObject *parent) :
    QObject(parent)
{
    d = new ComponentData;
}

/**
 * Destructor.
 *
 * \note If you are subclassing from \ref GCF::Component, it is recommended that
 * you declare destructor as protected. This will ensure that your component can
 * only be created on the heap, never on the stack.
 */
GCF::Component::~Component()
{
    delete d;
}

/**
 * Reimplement this function to return a name for your component. By default
 * this function returns the name of the component-class.
 *
 * \note If you are reimplementing this function, return a string name that
 * confirms to class-naming conventions of C++. [No spaces, first letter should
 * be character, no special characters apart from _]
 */
QString GCF::Component::name() const
{
    QString retStr = QString::fromLatin1( this->metaObject()->className() );
    retStr = retStr.replace("::", "_");
    return retStr;
}

/**
 * Reimplement this function to return the name of your organization.
 * By default this function returns \ref qApp->organizationName()
 */
QString GCF::Component::organization() const
{
    return qApp->organizationName();
}

/**
 * Reimplement this function to return a version number for your component.
 * By default this function return \ref GCF::Version(1,0,0);
 */
GCF::Version GCF::Component::version() const
{
    return GCF::version();
}

/**
 * Reimplement this function to return a build-timestamp. This should be
 * the date and time of the last time the component was built. By default
 * this function returns \c \__TIMESTAMP\__
 */
QString GCF::Component::buildTimestamp() const
{
    return __TIMESTAMP__;
}

/**
 * Returns settings of this component. The function will return a NULL
 * pointer until a \b Post-SettingsLoad event (\ref GCF::SettingsLoadEvent)
 * is delivered. It will return a valid pointer to \c QSettings after that.
 *
 * \sa \ref gcf_component_loading
 */
const QSettings *GCF::Component::settings() const
{
    return d->settings;
}

/**
 * This function returns true if the component has been loaded completely.
 * False otherwise. A component is considered loaded if it has received
 * the \b Post-Initialize event (\ref GCF::InitializeEvent).
 *
 * \sa \ref gcf_component_loading
 */
bool GCF::Component::isLoaded() const
{
    return gAppService->isLoaded(this);
}

/**
 * Returns true if the component has been loaded and activated. False
 * otherwise.
 *
 * \sa \ref gcf_component_activation
 */
bool GCF::Component::isActive() const
{
    return gAppService->isActive(this);
}

/**
 * Loads the component.
 *
 * \note If the component has already been loaded, then this function
 * is a no-op.
 *
 * \sa \ref gcf_component_loading
 */
void GCF::Component::load()
{
    gAppService->loadComponent(this);
}

/**
 * Unloads the component. The component is deleted by the time it returns.
 *
 * \sa \ref gcf_component_unloading
 */
void GCF::Component::unload()
{
    gAppService->unloadComponent(this);
}

/**
 * Activates the component.
 *
 * \note If the component is already active, then this function is a no-op.
 *
 * \sa \ref gcf_component_activation, \ref isActive()
 */
void GCF::Component::activate()
{
    gAppService->activateComponent(this);
}

/**
 * Deactivates the component.
 *
 * \note If the component is already inactive, then this function is a no-op.
 *
 * \sa \ref gcf_component_deactivation, \ref isActive()
 */
void GCF::Component::deactivate()
{
    gAppService->deactivateComponent(this);
}

/**
 * This function allows you to programmatically add a content-object to the component's
 * object tree, which makes it possible for other components in the application to
 * search for and reference the added object.
 *
 * Read \ref gcf_component_objectTree for more information about object-trees.
 *
 * @param name a string name for the added object. Names should following C++
 * object-naming convention.
 * @param object pointer to the \c QObject that is being added. If NULL, then this
 * function is a no-op.
 * @param info optional key=value map that contains additional information about the
 * content object.
 *
 * \note This function cannot be called within \ref contentObjectLoadEvent() and \ref contentObjectUnloadEvent()
 */
void GCF::Component::addContentObject(const QString &name, QObject *object, const QVariantMap &info)
{
    if(object == 0)
        return;

    ObjectTreeNode *componentNode = gAppService->objectTree()->node(this);
    if(!componentNode)
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
            QString("Cannot find object tree for component %1 while exposing object \"%2\" type %3. "
                    "The component needs to have begun loading before content objects can be added.")
                .arg(this->name()).arg(name).arg(object->metaObject()->className()));
        return;
    }

    if(d->currentEvent &&
       (d->currentEvent->type() == GCF::ContentObjectLoadEvent::Kind ||
        d->currentEvent->type() == GCF::ContentObjectUnloadEvent::Kind) )
    {
        GCF::Log::instance()->warning( GCF_DEFAULT_LOG_CONTEXT,
                                       QString("Cannot expose object %1 (of type %2) while a "
                                      "ContentObjectLoadEvent or ContentObjectUnloadEvent "
                                      "is being handled.").arg(name).arg(object->metaObject()->className()));
        return;
    }

    QString objectName = name;
    if(objectName.isEmpty())
    {
        int counter = componentNode->children().count();
        objectName = QString("%1%2").arg(object->metaObject()->className()).arg(counter);
    }

    new ObjectTreeNode(componentNode, objectName, object, info);
}

/**
\fn void GCF::Component::addContentObject(QObject *object, const QVariantMap &info)

This function allows you to programmatically add a content-object to the component's
object tree, which makes it possible for other components in the application to
search for and reference the added object.

Read \ref gcf_component_objectTree for more information about object-trees.

@param object pointer to the \c QObject that is being added. The \c objectName() of the object
is considered to be the \c name of the object in the object tree.
@param info optional key=value map that contains additional information about the
content object.

\note This function cannot be called within \ref contentObjectLoadEvent() and \ref contentObjectUnloadEvent()
  */

/**
 * This function removes a content-object from the component's object tree.
 *
 * @param object pointer to the \c QObject that needs to be removed
 *
 * \note The \c QObject refererred to by \c object will not be deleted by this function.
 */
void GCF::Component::removeContentObject(QObject *object)
{
    if(!object || !this->isLoaded())
        return;

    ObjectTreeNode *node = gAppService->objectTree()->node(object);
    if(node)
        delete node;
}

/**
 * This function removes a content-object from the component's object tree.
 *
 * @param name name of the object that needs to be removed
 *
 * \note The \c QObject refererred to by \c name will not be deleted by this function.
 */
void GCF::Component::removeContentObject(const QString &name)
{
    if(name.isEmpty() || !this->isLoaded())
        return;

    ObjectTreeNode *componentNode = gAppService->objectTree()->node(this);
    if(componentNode)
    {
        QString path = QString("%1.%2").arg(componentNode->name()).arg(name);
        ObjectTreeNode *node = componentNode->node(path);
        if(node)
            delete node;
    }
}

/**
 * \internal
 */
bool GCF::Component::event(QEvent *e)
{
    d->currentEvent = e;

    /*
      Since GCF::xxxEvent::Kind are runtime const integers, they cannot be
      used in a switch/case expression. Hence the usage of ladder-if
      construct below.

      By runtime-const integers; I mean that the value of the integer
      is evaluated at run-time by calling function QEvent::registerEventType().
      This means that the values of GCF::xxxEvent::Kind are not known at
      compile time!

      switch/case requires that the case value be known at compile time!
      */
    if( e->type() == GCF::InitializeEvent::Kind )
    {
        e->setAccepted(true);
        this->initializeEvent( (GCF::InitializeEvent*)e );
    }
    else if( e->type() == GCF::SettingsLoadEvent::Kind )
    {
        e->setAccepted(true);
        this->settingsLoadEvent( (GCF::SettingsLoadEvent*)e );
    }
    else if( e->type() == GCF::SettingsUnloadEvent::Kind )
    {
        e->setAccepted(true);
        this->settingsUnloadEvent( (GCF::SettingsUnloadEvent*)e );
    }
    else if( e->type() == GCF::ContentLoadEvent::Kind )
    {
        e->setAccepted(true);
        this->contentLoadEvent( (GCF::ContentLoadEvent*)e );
    }
    else if( e->type() == GCF::ContentObjectMergeEvent::Kind )
    {
        e->setAccepted(true);
        this->contentObjectMergeEvent( (GCF::ContentObjectMergeEvent*)e );
    }
    else if( e->type() == GCF::FinalizeEvent::Kind )
    {
        e->setAccepted(true);
        this->finalizeEvent( (GCF::FinalizeEvent*)e );
    }
    else if( e->type() == GCF::ContentUnloadEvent::Kind )
    {
        e->setAccepted(true);
        this->contentUnloadEvent( (GCF::ContentUnloadEvent*)e );
    }
    else if( e->type() == GCF::ActivationEvent::Kind )
    {
        e->setAccepted(true);
        this->activationEvent( (GCF::ActivationEvent*)e );
    }
    else if( e->type() == GCF::DeactivationEvent::Kind )
    {
        e->setAccepted(true);
        this->deactivationEvent( (GCF::DeactivationEvent*)e );
    }
    else if( e->type() == GCF::ContentObjectLoadEvent::Kind )
    {
        this->contentObjectLoadEvent( (GCF::ContentObjectLoadEvent*)e );
        e->setAccepted( ((GCF::ContentObjectLoadEvent*)e)->object() != 0 );
    }
    else if(e->type() == GCF::ActivateContentObjectEvent::Kind)
    {
        e->setAccepted(true);
        this->activateContentObjectEvent( (GCF::ActivateContentObjectEvent*)e );
    }
    else if(e->type() == GCF::DeactivateContentObjectEvent::Kind)
    {
        e->setAccepted(true);
        this->deactivateContentObjectEvent( (GCF::DeactivateContentObjectEvent*)e );
    }
    else if(e->type() == GCF::ContentObjectUnloadEvent::Kind)
    {
        e->setAccepted(true);
        this->contentObjectUnloadEvent( (GCF::ContentObjectUnloadEvent*)e );
    }
    else if(e->type() == GCF::ContentObjectUnmergeEvent::Kind)
    {
        e->setAccepted(true);
        this->contentObjectUnmergeEvent( (GCF::ContentObjectUnmergeEvent*)e );
    }
    else
        QObject::event(e);

    d->currentEvent = 0;

    return e->isAccepted();
}

/**
\class GCF::InitializeEvent
\brief Contains parameters that describe an initialization event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::InitializeEvent::isPreInitialize() const
\return true if this class describes a pre-initialization event, false otherwise.
\sa \ref GCF::Component::initializeEvent()
*/

/**
\fn bool GCF::InitializeEvent::isPostInitialize() const
\return true if this class describes a post-initialization event, false otherwise.
\sa \ref GCF::Component::initializeEvent()
*/

/**
Reimplement this event handler to handle pre and post initialization events. The
default implementation does nothing.

@param e pointer to a \ref GCF::InitializeEvent object that describes the event.

This is the first event that gets delivered to a component during load.
Typically this function is reimplemented to handle
\li \c Pre-Initialzation time to initialize component's objects and variables (or
examine system environment) before the component's settings and content get loaded.
\li \c Post-Initialization time to do some work after the component is completely
loaded, but before it is activated.

Example:

\code
class MyComponent : public GCF::Component
{
protected:
    void initializeEvent(GCF::InitializeEvent *e) {
        if(e->isPreInitialize()) {
             // Pre-initialization work
        } else {
             // Post-initialization work
        }
    }
};
\endcode

\sa \ref gcf_component_loading
 */
void GCF::Component::initializeEvent(GCF::InitializeEvent*)
{
}

/**
\class GCF::SettingsLoadEvent
\brief Contains parameters that describe an settings load event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::SettingsLoadEvent::isPreSettingsLoad() const
\return true if this class describes a pre-settings load event, false otherwise.
\sa \ref GCF::Component::settingsLoadEvent()
*/

/**
\fn bool GCF::SettingsLoadEvent::isPostSettingsLoad() const
\return true if this class describes a post-settings load event, false otherwise.
\sa \ref GCF::Component::settingsLoadEvent()
*/

/**
\fn void GCF::SettingsLoadEvent::setSettingsFile(const QString &fileName)
Use this function to set the settings file for loading.

@param fileName name of the settings file to load.

\note the fileName is either absolute or relative to the location of the executable.
If no name is passed then the settings file is assumed to be a \c INI file in
component's name (\ref GCF::Component::name()) stored in the settings directory
(\ref GCF::settingsDirectory())

\note This function should only be used if \ref isPreSettingsLoad() returns true.

\sa \ref GCF::Component::settingsLoadEvent()
*/

/**
\fn QString GCF::SettingsLoadEvent::settingsFile() const

\return When \ref isPreSettingsLoad() returns true, the function returns name of
the settings file set via the \ref setSettingsFile() method. When \ref isPostSettingsLoad()
returns true, the function returns name of the settings file that was actually used
to load the settings.

\sa \ref GCF::Component::settingsLoadEvent()
*/

/**
Reimplement this event handler to handle pre and post settings-load events. The
default implementation does nothing.

@param e pointer to a \ref GCF::SettingsLoadEvent object that describes the event.

This is the second event that gets delivered to a component during load. Typically
this function is implemented to
\li Offer name of the settings file to load in \c Pre-SettingsLoad.
\li Process the loaded settings in \c Post-SettingsLoad

Example:

\code
class MyComponent : public GCF::Component
{
protected:
    void settingsLoadEvent(GCF::SettingsLoadEvent *e) {
        if(e->isPreSettingsLoad())
            e->setSettingsFile(....);
        else {
            const QSettings *settings = this->settings();
            // process loaded settings
        }
    }
};
\endcode

\sa \ref gcf_component_loading
 */
void GCF::Component::settingsLoadEvent(GCF::SettingsLoadEvent*)
{
}

/**
\class GCF::SettingsUnloadEvent
\brief Contains parameters that describe a settings-unload event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::SettingsUnloadEvent::isPreSettingsUnload() const
\return true if this class describes a pre-settings unload event, false otherwise.
\sa \ref GCF::Component::settingsUnloadEvent()
*/

/**
\fn bool GCF::SettingsUnloadEvent::isPostSettingsUnload() const
\return true if this class describes a post-settings unload event, false otherwise.
\sa \ref GCF::Component::settingsUnloadEvent()
*/

/**
Reimplement this event handler to handle pre and post settings-unload events. The
default implementation does nothing.

@param e pointer to a \ref GCF::SettingsUnloadEvent object that describes the event.

This event is delivered to the component during its \ref unload() process. Typically
this function is implemented to
\li Add/remove/alter keys and/or values in \ref settings() in \c Pre-SettingsUnload.
\li Release any memory occupied for handling component settings in \c Post-SettingsUnload

Example:

\code
class MyComponent : public GCF::Component
{
protected:
    void settingsUnloadEvent(GCF::SettingsUnloadEvent *e) {
        if(e->isPreSettingsUnload()) {
            QSettings *settings = const_cast<QSettings*>(this->settings());
            // add/remove/alter settings here
        } else {
            // release any memory used for handling configuration parameters
            // sourced from settings
        }
    }
};
\endcode

\sa \ref gcf_component_loading
*/
void GCF::Component::settingsUnloadEvent(GCF::SettingsUnloadEvent*)
{
}

/**
\class GCF::ContentLoadEvent
\brief Contains parameters that describe a content-load event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::ContentLoadEvent::isPreContentLoad() const
\return true if this class describes a pre-content load event, false otherwise.
\sa \ref GCF::Component::contentLoadEvent()
*/

/**
\fn bool GCF::ContentLoadEvent::isPostContentLoad() const
\return true if this class describes a post-content load event, false otherwise.
\sa \ref GCF::Component::contentLoadEvent()
*/

/**
\fn void GCF::ContentLoadEvent::setContentFile(const QString &fileName)
Use this function to set the content file for loading.

@param fileName name of the content file to load.

\note the fileName is either absolute or relative to the location of the executable.
If no name is passed then the content file is assumed to be a \c XML file in
component's name (\ref GCF::Component::name()) stored in the content directory
(\ref GCF::contentDirectory())

\note This function should only be used if \ref isPreContentLoad() returns true.

\sa \ref GCF::Component::contentLoadEvent()
*/

/**
\fn QString GCF::ContentLoadEvent::contentFile() const

\return When \ref isPreContentLoad() returns true, the function returns name of
the content file set via the \ref setContentFile() method. When \ref isPostContentLoad()
returns true, the function returns name of the content file that was actually used
to load the content.

\sa \ref GCF::Component::contentLoadEvent()
*/

/**
Reimplement this event handler to handle pre and post content-load events. The
default implementation does nothing.

@param e pointer to a \ref GCF::ContentLoadEvent object that describes the event.

This event is delivered to the component during its \ref load() process. Typically
this function is implemented to
\li Offer name of the content file to load during \b Pre-ContentLoad
\li Initialize loaded content objects during \b Post-ContentLoad

Example:

\code
class MyComponent : public GCF::Component
{
protected:
    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad()) {
            e->setContentFile(...);
        } else {
            // Initialize or do anything else with loaded content
        }
    }
};
\endcode

\sa \ref gcf_component_loading
*/
void GCF::Component::contentLoadEvent(GCF::ContentLoadEvent*)
{
}

/**
\class GCF::ContentObjectLoadEvent
\brief Contains parameters that describe a content-object-load event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn void GCF::ContentObjectLoadEvent::objectName() const

\return name of the object that needs loading. This would be equal to the
\c name attribute of the \c objext XML element against which this event
was generated.

\sa \ref GCF::Component::contentObjectLoadEvent()
*/

/**
\fn QVariantMap GCF::ContentObjectLoadEvent::info() const

\return a attribute=value map of all attributes from from the \c object XML
element against which this event was generated.

\sa \ref GCF::Component::contentObjectLoadEvent()
*/

/**
\fn void GCF::ContentObjectLoadEvent::setObject(QObject *object)

Through this function the event handler \ref GCF::Component::contentObjectLoadEvent() can
set the content object.

@param object pointer to a \c QObject that was loaded/created/mapped when this
event was generated.

\sa \ref GCF::Component::contentObjectLoadEvent()
*/

/**
\fn QObject *GCF::ContentObjectLoadEvent::object() const

\return pointer to the \c QObject that was set using the \ref setObject() method. If
no such object was set, then a NULL pointer is returned.

\sa \ref GCF::Component::contentObjectLoadEvent()
*/

/**
Reimplement this event handler to handle content object load events. This event handler is called
whenever an \c object XML element is encountered in the content file.

@param e pointer to a \ref GCF::ContentObjectLoadEvent object that describes the event.

The default implementation of this event-handler calls the \ref loadObject() method and
sets the object returned from it into \ref GCF::ContentObjectLoadEvent::setObject(). It
is recommended that you implement \ref loadObject() instead of this event handler for
loading content objects.

\code
class MyComponent : public GCF::Component
{
protected:
    void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e) {
        if(e->name() == ....)
            e->setObject(...);
        else if(e->name() == ....)
            e->setObject(...)
        // ...
        // ...
        // ...
        // ...
    }
};
\endcode

\sa \ref gcf_component_contentfile
*/
void GCF::Component::contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e)
{
    QObject *obj = this->loadObject(e->objectName(), e->info());
    e->setObject(obj);
}

/**
\class GCF::ContentObjectMergeEvent
\brief Contains parameters that describe a content-object-merge event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn QObject *GCF::ContentObjectMergeEvent::parent() const
\return pointer to a \c QObject belonging to the component that received this event.
Into this parent the \ref child() should be merged (or parented)

\sa \ref GCF::Component::contentObjectMergeEvent()
*/

/**
\fn QObject *GCF::ContentObjectMergeEvent::child() const

\return pointer to a \c QObject that needs merging (or reparenting) into \ref parent().

\sa \ref GCF::Component::contentObjectMergeEvent()
*/

/**
\fn QVariantMap GCF::ContentObjectMergeEvent::parentInfo() const

\return a key=value information map associated with \ref parent() object

\sa \ref GCF::Component::contentObjectMergeEvent()
*/

/**
\fn QVariantMap GCF::ContentObjectMergeEvent::childInfo() const

\return a key=value information map associated with \ref child() object

\sa \ref GCF::Component::contentObjectMergeEvent()
*/

/**
Reimplement this event handler to handle content-object merge events. This event
handler is called if a content-object that is currently being loaded by GCF needs
to be merged with a content-object belonging to this component.

@param e pointer to a \ref GCF::ContentObjectMergeEvent object that describes the event.

A merge happens when the \c parent attribute of \c object XML element (that caused
creation of a content-object) refers to a content object belonging to this component.

Example: Suppose that this component (by name MyComponent) is offering an object by
name 'container'. Also suppose that another component has a content XML file as
shown below

\code
<content>
    <object name="object" parent="MyComponent.container" />
</content>
\endcode

After the other component has finished handling \ref GCF::ContentObjectLoadEvent for \c object,
a \ref GCF::ContentObjectMergeEvent is sent to this component with
\li pointer to 'container'
\li pointer to 'object'
\li key=value information map of 'container'
\li key=value information map of 'object'

This event handler can then customize the way in which parenting (or merging) happens
for 'container' and 'object'. For example, in a real world scenario, implementations of this event
handler could add a child-widget into a tab of the parent-widget.

\note The default implementation of this event handler calls \ref mergeObject().
It is recommended that you implement \ref mergeObject() instead of this event handler.

\sa \ref gcf_component_loading, \ref gcf_component_contentfile
*/
void GCF::Component::contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e)
{
    bool success = this->mergeObject( e->parent(), e->child(), e->parentInfo(), e->childInfo() );
    e->setAccepted(success);
}

/**
\class GCF::ContentObjectUnloadEvent
\brief Contains parameters that describe a content-object-unload event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn QString GCF::Component::objectName() const

\return name of the object that is being unloaded

\sa \ref GCF::Component::contentObjectUnloadEvent()
*/

/**
\fn QVariantMap GCF::Component::info() const

\return key=value information map associated with the object that is about to be unloaded

\sa \ref GCF::Component::contentObjectUnloadEvent()
*/

/**
\fn QObject *GCF::Component::object() const

\return pointer to the \c QObject that needs unloading.

\note You can delete the object returned by this function in
\ref GCF::Component::contentObjectUnloadEvent(). Otherwise, GCF will delete the object after
the event is delivered.

\sa \ref GCF::Component::contentObjectUnloadEvent()
*/

/**
Reimplement this event handler to handle content object load events. This event handler is
called whenever a object XML element is encountered in the content file.

@param e pointer to a \ref GCF::ContentObjectUnloadEvent object that describes the event.

The default implementation of this event-handler calls the \ref unloadObject() method. It is
recommended that you implement \ref unloadObject() instead of this event handler for unloading
objects.

\code
class MyComponent : public GCF::Component
{
protected:
    void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e) {
        if(e->name() == ....) {
            // un-initialize the object
            // optionally delete the object
            delete e->object();
        } else if(e->name() == ....) {

        }

        // ...
        // ...
        // ...
        // ...
    }
};
\endcode

\note Content-object that is being unloaded will be automatically deleted by GCF, unless you reimplement
the event handler to explicitly delete the object here itself.

\sa \ref gcf_component_unloading
 */
void GCF::Component::contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e)
{
    bool success = this->unloadObject(e->objectName(), e->object(), e->info());
    e->setAccepted(success);
}

/**
\class GCF::ContentObjectUnmergeEvent
\brief Contains parameters that describe a content-object-unmerge event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn QObject *GCF::ContentObjectUnmergeEvent::parent() const
\return pointer to a \c QObject belonging to the component that received this event.
From this parent the \ref child() should be unmerged

\sa \ref GCF::Component::contentObjectUnmergeEvent()
*/

/**
\fn QObject *GCF::ContentObjectUnmergeEvent::child() const

\return pointer to a \c QObject that needs unmerging from \ref parent().

\sa \ref GCF::Component::contentObjectUnmergeEvent()
*/

/**
\fn QVariantMap GCF::ContentObjectUnmergeEvent::parentInfo() const

\return a key=value information map associated with \ref parent() object

\sa \ref GCF::Component::contentObjectUnmergeEvent()
*/

/**
\fn QVariantMap GCF::ContentObjectUnmergeEvent::childInfo() const

\return a key=value information map associated with \ref child() object

\sa \ref GCF::Component::contentObjectUnmergeEvent()
*/

/**
Reimplement this event handler to handle content object unmerge events. This event handler is
called if a content-object that is currently being unloaded by GCF needs to be unmerged from
a content-object belonging to this component.

@param e pointer to a \ref GCF::ContentObjectUnmergeEvent object that describes the event.

An unmerge happens when a component, whose object was merged with an object from this component,
is being unloaded. This event handler can customize the process of unmerging. The default implementation
calls \ref unmergeObject() function. It is recommended that you implement \ref unmergeObject()
instead of this event handler.

\sa \ref gcf_component_unloading
*/
void GCF::Component::contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e)
{
    bool success = this->unmergeObject( e->parent(), e->child(), e->parentInfo(), e->childInfo() );
    e->setAccepted(success);
}

/**
\class GCF::ContentUnloadEvent
\brief Contains parameters that describe a content-unload event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::ContentUnloadEvent::isPreContentUnload() const
\return true if this class describes a pre-content unload event, false otherwise.
\sa \ref GCF::Component::contentUnloadEvent()
*/

/**
\fn bool GCF::ContentUnloadEvent::isPostContentUnload() const
\return true if this class describes a post-content unload event, false otherwise.
\sa \ref GCF::Component::contentUnloadEvent()
*/

/**
Reimplement this event handler to handle pre and post content-unload events. The
default implementation does nothing.

@param e pointer to a \ref GCF::ContentUnloadEvent object that describes the event

This event is called during the \ref unload() process. Typically this function is implemented to

\li Prepare the component and its content-objects for unloading during \b Pre-ContentUnload
\li Release any additional memory occupied for managing the content of the component during
\b Post-ContentUnload

Example:

\code
class MyComponent : public GCF::Component
{
protected:
    void contentUnloadEvent(GCF::ContentUnloadEvent *e) {
        if(e->isPreContentLoad()) {
            // Prepare for unload
        } else {
            // Cleanup after unload
        }
    }
};
\endcode

\sa \ref gcf_component_unloading
*/
void GCF::Component::contentUnloadEvent(GCF::ContentUnloadEvent*)
{
}

/**
\class GCF::ActivationEvent
\brief Contains parameters that describe an activation event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;
\endhtmlonly
 */

/**
\fn bool GCF::ActivationEvent::isPreActivation() const
\return true if the class describes a pre-activation event. false otherwise.
*/

/**
\fn bool GCF::ActivationEvent::isPostActivation() const
\return true if the class describes a post-activation event. false otherwise.
*/

/**
Reimplement this event handler to handle pre and post activation events. The default implementation
does nothing.

@param e pointer to a \ref GCF::ActivationEvent object that describes this event.

This event gets delivered during \ref activate() process. The event handler can be reimplemented to
perform some tasks just before and just after activation.
 */
void GCF::Component::activationEvent(GCF::ActivationEvent*)
{
}

/**
\class GCF::DeactivationEvent
\brief Contains parameters that describe a deactivation event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;
\endhtmlonly
 */

/**
\fn bool GCF::DeactivationEvent::isPreDeactivation() const
\return true if the class describes a pre-deactivation event. false otherwise.
*/

/**
\fn bool GCF::DeactivationEvent::isPostDeactivation() const
\return true if the class describes a post-deactivation event. false otherwise.
*/

/**
Reimplement this event handler to handle pre and post deactivation events. The default implementation
does nothing.

@param e pointer to a \ref GCF::DeactivationEvent object that describes this event.

This event gets delivered during \ref deactivate() process. The event handler can be reimplemented to
perform some tasks just before and just after deactivation.
 */
void GCF::Component::deactivationEvent(GCF::DeactivationEvent*)
{
}

/**
\class GCF::ActivateContentObjectEvent
\brief Contains parameters that describe an content-object-activation event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;
\endhtmlonly
 */

/**
\fn QObject *GCF::ActivateContentObjectEvent::parent() const
\return pointer to a \c QObject belonging to the component that received this event.
The \ref child() within this parent needs to be activated.

\sa \ref GCF::Component::activateContentObjectEvent()
*/

/**
\fn QObject *GCF::ActivateContentObjectEvent::child() const

\return pointer to a \c QObject that needs activation

\sa \ref GCF::Component::activateContentObjectEvent()
*/

/**
\fn QVariantMap GCF::ActivateContentObjectEvent::parentInfo() const

\return a key=value information map associated with \ref parent() object

\sa \ref GCF::Component::activateContentObjectEvent()
*/

/**
\fn QVariantMap GCF::ActivateContentObjectEvent::childInfo() const

\return a key=value information map associated with \ref child() object

\sa \ref GCF::Component::activateContentObjectEvent()
*/

/**
Reimplement this handler to handle content-object activation. The event is sent as a part
of \ref activate() process. The default event handler calls \ref activateObject(). It
is recommended that you implement \ref activateObject() than handling this event.

\param e pointer to a \ref GCF::ActivateContentObjectEvent object that describes this event

\sa \ref gcf_component_activation
*/
void GCF::Component::activateContentObjectEvent(GCF::ActivateContentObjectEvent *e)
{
    bool success = this->activateObject(e->parent(), e->child(), e->parentInfo(), e->childInfo() );
    e->setAccepted(success);
}

/**
\class GCF::DeactivateContentObjectEvent
\brief Contains parameters that describe an content-object-deactivation event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;
\endhtmlonly
 */

/**
\fn QObject *GCF::DeactivateContentObjectEvent::parent() const
\return pointer to a \c QObject belonging to the component that received this event.
The \ref child() within this parent needs to be deactivated.

\sa \ref GCF::Component::deactivateContentObjectEvent()
*/

/**
\fn QObject *GCF::DeactivateContentObjectEvent::child() const

\return pointer to a \c QObject that needs deactivation

\sa \ref GCF::Component::deactivateContentObjectEvent()
*/

/**
\fn QVariantMap GCF::DeactivateContentObjectEvent::parentInfo() const

\return a key=value information map associated with \ref parent() object

\sa \ref GCF::Component::deactivateContentObjectEvent()
*/

/**
\fn QVariantMap GCF::DeactivateContentObjectEvent::childInfo() const

\return a key=value information map associated with \ref child() object

\sa \ref GCF::Component::deactivateContentObjectEvent()
*/

/**
Reimplement this handler to handle content-object deactivation. The event is sent as a part
of \ref deactivate() process. The default event handler calls \ref deactivateObject(). It
is recommended that you implement \ref deactivateObject() than handling this event.

\param e pointer to a \ref GCF::DeactivateContentObjectEvent object that describes this event

\sa \ref gcf_component_deactivation
*/
void GCF::Component::deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e)
{
    bool success = this->deactivateObject(e->parent(), e->child(), e->parentInfo(), e->childInfo() );
    e->setAccepted(success);
}

/**
\class GCF::FinalizeEvent
\brief Contains parameters that describe an finalization event
\ingroup gcf_core

\htmlonly
<pre>#include &lt;GCF3/Component&gt;</pre>
\endhtmlonly
*/

/**
\fn bool GCF::FinalizeEvent::isPreFinalize() const
\return true if this class describes a pre-finalization event, false otherwise.
\sa \ref GCF::Component::finalizeEvent()
*/

/**
\fn bool GCF::FinalizeEvent::isPostFinalize() const
\return true if this class describes a post-finalization event, false otherwise.
\sa \ref GCF::Component::finalizeEvent()
*/

/**
Reimplement this event handler to handle pre and post finalization events. This
function is called as a part of the \ref unload() process. The function can be
reimplemented to

\li Prepare the component for finalization during \b Pre-Finalize
\li Clean up memory and prepare for deletion during \b Post-Finalize

The default implementation does nothing.

@param e pointer to \ref GCF::FinalizeEvent object that describes this event
 */
void GCF::Component::finalizeEvent(GCF::FinalizeEvent*)
{
}

/**
This function is called from \ref GCF::Component::contentObjectLoadEvent(). You can
reimplement this function to return a \c QObject pointer based on the name and
information map given.

@param name name of the object that needs to be loaded
@param info key=value information map provided against the object that needs loading
@return pointer to a \c QObject that was loaded.
*/
QObject *GCF::Component::loadObject(const QString &name, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    return 0;
}

/**
This function is called from \ref GCF::Component::contentObjectUnloadEvent(). You can
reimplement this function to unload the given object. GCF will delete the object immediately
after return from this function, unless it was deleted explicitly by you in the reimplementation
of this function.

@param name name of the object that needs to be unloaded
@param object pointer to the \c QObject that needs to be unloaded
@param info key=value information map against the object that needs unloading
@return true on success, false otherwise. [Note: The return value is currently ignored]
*/
bool GCF::Component::unloadObject(const QString &name, QObject *object, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(object);
    Q_UNUSED(info);
    return true;
}

/**
This function is called from \ref GCF::Component::contentObjectMergeEvent(). You can reimplement
this function to merge \c child into \c parent. Here \c parent belongs to this component.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be merged (or parented) into \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
*/
bool GCF::Component::mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_ASSERT(parent != 0);
    Q_ASSERT(child != 0);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    child->setParent(parent);
    return true;
}

/**
This function is called from \ref GCF::Component::contentObjectUnmergeEvent(). You can reimplement
this function to unmerge \c child into \c parent. Here \c parent belongs to this component.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be unmerged (or parented) into \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
*/
bool GCF::Component::unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_ASSERT(parent != 0);
    Q_ASSERT(child != 0);
    Q_UNUSED(parent);
    Q_UNUSED(child);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    child->setParent(0);
    return true;
}

/**
This function is called from \ref GCF::Component::activateContentObjectEvent(). You can reimplement
this function to activate the \c child in a way suitable for \c parent.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be activated in \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
*/
bool GCF::Component::activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_ASSERT(parent != 0);
    Q_ASSERT(child != 0);
    Q_UNUSED(parent);
    Q_UNUSED(child);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    return true;
}

/**
This function is called from \ref GCF::Component::deactivateContentObjectEvent(). You can reimplement
this function to deactivate the \c child in a way suitable for \c parent.

@param parent pointer to a \c QObject added by this component.
@param child pointer to a \c QObject that needs to be deactivated in \c parent
@param parentInfo key=value information map associated with the \c parent
@param childInfo key=value information map associated with the \c child
@return true on success, false otherwise. [Note: The return value is currently ignored]
*/
bool GCF::Component::deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
{
    Q_ASSERT(parent != 0);
    Q_ASSERT(child != 0);
    Q_UNUSED(parent);
    Q_UNUSED(child);
    Q_UNUSED(parentInfo);
    Q_UNUSED(childInfo);
    return true;
}

/**
\internal
 */
void GCF::Component::setSettings(QSettings *settings)
{
    if(d->settings)
        return;

    d->settings = settings;
    d->settings->setParent(this);
}

///////////////////////////////////////////////////////////////////////////////

// Component Event Classes
const QEvent::Type GCF::InitializeEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4232) );
GCF::InitializeEvent::InitializeEvent(int initType)
    : QEvent(Kind), m_initType(initType) { }

const QEvent::Type GCF::SettingsLoadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4233) );
GCF::SettingsLoadEvent::SettingsLoadEvent(const QString &settingsFile, int settingsLoadType)
    : QEvent(Kind), m_settingsLoadType(settingsLoadType), m_settingsFile(settingsFile) { }

const QEvent::Type GCF::ContentLoadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4234) );
GCF::ContentLoadEvent::ContentLoadEvent(const QString &contentFile, int contentLoadType)
    : QEvent(Kind), m_contentLoadType(contentLoadType), m_contentFile(contentFile) { }

const QEvent::Type GCF::ActivationEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4235) );
GCF::ActivationEvent::ActivationEvent(int actType)
    : QEvent(Kind), m_activationType(actType) { }

const QEvent::Type GCF::DeactivationEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4236) );
GCF::DeactivationEvent::DeactivationEvent(int deactType)
    : QEvent(Kind), m_deactivationType(deactType) { }

const QEvent::Type GCF::SettingsUnloadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4237) );
GCF::SettingsUnloadEvent::SettingsUnloadEvent(int settingsUnloadType)
    : QEvent(Kind), m_settingsUnloadType(settingsUnloadType) { }

const QEvent::Type GCF::ContentUnloadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4238) );
GCF::ContentUnloadEvent::ContentUnloadEvent(int contentUnloadType)
    : QEvent(Kind), m_contentUnloadType(contentUnloadType) { }

const QEvent::Type GCF::FinalizeEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4239) );
GCF::FinalizeEvent::FinalizeEvent(int finType)
    : QEvent(Kind), m_finalizeType(finType) { }

const QEvent::Type GCF::ContentObjectLoadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4240) );
GCF::ContentObjectLoadEvent::ContentObjectLoadEvent(const QString &objName, const QVariantMap &info)
    : QEvent(Kind), m_objectName(objName), m_info(info), m_object(0) { }

const QEvent::Type GCF::ContentObjectMergeEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4241) );
GCF::ContentObjectMergeEvent::ContentObjectMergeEvent(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
    : QEvent(Kind), m_parent(parent), m_child(child), m_parentInfo(parentInfo), m_childInfo(childInfo) { }

const QEvent::Type GCF::ActivateContentObjectEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4242) );
GCF::ActivateContentObjectEvent::ActivateContentObjectEvent(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
    : QEvent(Kind), m_parent(parent), m_child(child), m_parentInfo(parentInfo), m_childInfo(childInfo) { }

const QEvent::Type GCF::DeactivateContentObjectEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4243) );
GCF::DeactivateContentObjectEvent::DeactivateContentObjectEvent(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
    : QEvent(Kind), m_parent(parent), m_child(child), m_parentInfo(parentInfo), m_childInfo(childInfo) { }

const QEvent::Type GCF::ContentObjectUnloadEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4244) );
GCF::ContentObjectUnloadEvent::ContentObjectUnloadEvent(const QString &objName, QObject *object, const QVariantMap &info)
    : QEvent(Kind), m_objectName(objName), m_info(info), m_object(object) { }

const QEvent::Type GCF::ContentObjectUnmergeEvent::Kind =
        static_cast<QEvent::Type>( QEvent::registerEventType(4245) );
GCF::ContentObjectUnmergeEvent::ContentObjectUnmergeEvent(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo)
    : QEvent(Kind), m_parent(parent), m_child(child), m_parentInfo(parentInfo), m_childInfo(childInfo) { }

QString GCF::ComponentEvent::eventName(QEvent::Type eventType)
{
#define EVENT(x) if(eventType == GCF::x::Kind) return #x
    EVENT(FinalizeEvent);
    EVENT(InitializeEvent);
    EVENT(ActivationEvent);
    EVENT(ContentLoadEvent);
    EVENT(SettingsLoadEvent);
    EVENT(DeactivationEvent);
    EVENT(ContentUnloadEvent);
    EVENT(SettingsUnloadEvent);
    EVENT(ContentObjectLoadEvent);
    EVENT(ContentObjectMergeEvent);
    EVENT(ContentObjectUnloadEvent);
    EVENT(ContentObjectUnmergeEvent);
    EVENT(ActivateContentObjectEvent);
    EVENT(DeactivateContentObjectEvent);
    return QString("Unknown!");
#undef EVENT
}

