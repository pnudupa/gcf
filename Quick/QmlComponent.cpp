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

#include "QmlComponent.h"
#include "QmlApplication.h"

#include <QFileInfo>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

/**
\class GCF::QmlComponent QmlComponent.h <GCF3/QmlComponent>
\brief Extends \ref GCF::Component to ease creation of QML components
\ingroup gcf_quick


This class offers QML friendly methods to make creation of QML components in GCF applications
really simple. Please note that there is a difference in the meaning of the term \b QML-Component
when used in Qt 5 context and when used in GCF context. Unless otherwise stated the term
\b QML-Component in this class document refers to an instance of \ref GCF::QmlComponent class.

This class reimplements
\li \ref GCF::Component::initializeEvent() - to check whether the component is being loaded
in a GCF QML application or not. The component asserts a failure and crashes the application
otherwise.

\li \ref GCF::Component::activationEvent() and \ref GCF::Component::deactivationEvent() - to
gather changes to component activation and update the \ref active property.

\li \ref GCF::Component::contentObjectLoadEvent() - to identify qml object kinds and support
loading / instantiation of QML objects from \c .qml files.

\li \ref GCF::Component::contentObjectMergeEvent() - to ignore merge events for QML objects

\li \ref GCF::Component::contentObjectUnloadEvent() - to unload QML objects effectively

\li \ref GCF::Component::contentObjectUnmergeEvent() - to ignore unmerge events for QML objects

\li \ref GCF::Component::activateContentObjectEvent() - to ignore activation events for QML objects

\li \ref GCF::Component::deactivateContentObjectEvent() - to ignore deactivation events for QML objects

Content files for QML components can make use of \c type=qml and \c url= attributes in \c object
XML element to load QML objects. Example

\code
<content>
    <object name="root" type="qml" url="qrc:///RootItem.qml" >
    <object name="sample" type="qml" url="http://www.something.com/Sample.qml" >
    <object name="another" type="qml" url="file:///Users/jaggesh/Another.qml" >
</content>
\endcode

\note QML files loaded using this class, either through content-files or by calling the \ref addQml() function,
can make use of a \c gComponent object in their code to reference \c this component.

This class also offers a specialized virtual function called \ref loadQmlObject(), whose default
implementation eases out loading of QML objects from \c .qml files. Subclasses can reimplement
this function to customize the loading of QML objects.

The \ref addQml() methods may be used to add QML objects from \c .qml files - without loading
them from the content-file. Custom QML items can be added using \ref addQmlItem() method. QML
items added using \ref addQml() or \ref addQmlItem() methods are included in the object list.

\note This class is available only if GCF is compiled and linked against Qt 5.
*/

namespace  GCF
{

struct QmlComponentData
{
    QmlComponentData() : active(false), context(nullptr) { }

    QmlComponent *component;
    bool active;
#ifdef Q_OS_MAC
    char unused[7]; // Padding for aligning structure
#endif
    QQmlContext *context;

    QObject *loadQml(const QUrl &url);
    void logComponentSuccess(QQmlComponent *qmlComp);
    void logComponentError(QQmlComponent *qmlComp);
};

}

/**
Constructor. Creates an instance of this class and inserts it as a child
under \c parent.

\note It is recommended that subclasses of \ref GCF::GuiComponent declare their
constructor as public and destructor as protected. This will ensure that the
components are created on the heap always.
 */
GCF::QmlComponent::QmlComponent(QObject *parent)
    :GCF::Component(parent)
{
    d = new QmlComponentData;
    d->component = this;
}

/**
Destructor.

\note It is recommended that subclasses of \ref GCF::GuiComponent declare their
constructor as public and destructor as protected. This will ensure that the
components are created on the heap always.
 */
GCF::QmlComponent::~QmlComponent()
{
    delete d;
}

/**
Returns true if the component has been activated. False otherwise.

This function is called when the \c active property is READ. The \c active property
is notified as changed whenever this component receives a \ref activationEvent() or
\ref deactivationEvent(). Change notifications to this property are signaled using
the \ref activeChanged() signal.
 */
bool GCF::QmlComponent::isActive() const
{
    return d->active;
}

/**
\fn void GCF::QmlComponent::activeChanged(bool val)

This signal is emitted whenever the \c active property changes.
 */

/**
This function can be used to create a QML object from \c url and add it as a content-object
of this component.

\param name name to be set on QML object loaded from \c url.
\param url location of the \c .qml file from which the QML object should be loaded.
\c http, \c file, \c ftp and \c qrc schemes are supported.
\return pointer to the loaded QML object upon success, null upon failure.

\note QML objects added using this function can access a \c gComponent object in their code
to access methods on this component.

\sa \ref GCF::Component::addContentObject()
*/
QObject *GCF::QmlComponent::addQml(const QString &name, const QUrl &url)
{
    GCF::LogMessageBranch branch( QString("Adding QML object from %1 as %2").arg(url.toString()).arg(name) );

    GCF::ObjectTreeNode *componentNode = gAppService->objectTree()->node(this);
    if(!componentNode)
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Cannot load QML object from \"%1\" before the component is loaded")
                                    .arg(url.toString()));
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("QML object was not added"));
        return nullptr;
    }

    if(componentNode->node(name))
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Another object by name \"%1\" has already been added").arg(name));
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("QML object was not added"));
        return nullptr;
    }

    QObject *qmlObj = d->loadQml(url);
    if(qmlObj)
    {
        qmlObj->setObjectName(name);

        QVariantMap info;
        info["url"] = url;

        this->addContentObject(name, qmlObj, info);

        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("QML object successfully added"));
    }
    else
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("QML object was not added"));

    return qmlObj;
}

/**
This function adds a QML object from \c url.

\param url location of the \c .qml file from which the QML object should be loaded.
\c http, \c file, \c ftp and \c qrc schemes are supported.
\return pointer to the loaded QML object upon success, null upon failure.

Name of the QML object is inferred from the file-name of the \c .qml file specified in \c url.

\note QML objects added using this function can access a \c gComponent object in their code
to access methods on this component.
 */
QObject *GCF::QmlComponent::addQml(const QUrl &url)
{
    QString name = url.path();
    name = name.split('/', QString::SkipEmptyParts).last();
    name = name.split('.', QString::SkipEmptyParts).first();
    return this->addQml(name, url);
}

/**
\fn void GCF::QmlComponent::addQmlItem(const QString &name, QObject *object)

Adds a QML item as content object to this component

\param name name of the QML object
\param object pointer to the QML object

\note This function can be called from within QML code using \c gComponent.addQmlItem(string,object)
 */

/**
\fn void GCF::QmlComponent::removeQmlItem(QObject *object)

Removes a QML item as content object from this component

\param object pointer to the QML object that needs to be removed

\note This function can be called from within QML code using \c gComponent.removeQmlItem(object)
*/

/**
\fn void GCF::QmlComponent::removeQmlItem(const QString &name)

Removes a QML item as content object from this component

\param name name of the QML object that needs to be removed

\note This function can be called from within QML code using \c gComponent.removeQmlItem(string)
 */

/**
\fn QObject *GCF::QmlComponent::qmlItem(const QString &name) const

Returns pointer to a QML object (or item) whose name is \c name. For this function to work a QML
object should have been added using \ref addQmlItem() for the same name before.

\param name name of the QML item/object to return
\return pointer to the QML object. NULL if no such object was found.

\note This function can be called from within QML code using \c gComponent.qmlItem(string)
*/

/**
This event handler is implemented to ASSERT crash the application, if the component is attempted for loading
within a non QML application. It calls the base class implementation of the event handler otherwise.
 */
void GCF::QmlComponent::initializeEvent(GCF::InitializeEvent *e)
{
    if(e->isPreInitialize())
        Q_ASSERT(gApp && gApp->qmlEngine() != nullptr);

    GCF::Component::initializeEvent(e);
}

/**
Updates the \c active property and calls the base class implementation of this event handler
 */
void GCF::QmlComponent::activationEvent(GCF::ActivationEvent *e)
{
    if(e->isPostActivation())
    {
        d->active = true;
        emit activeChanged(d->active);
    }

    GCF::Component::activationEvent(e);
}

/**
Updates the \c active property and calls the base class implementation of this event handler
 */
void GCF::QmlComponent::deactivationEvent(GCF::DeactivationEvent *e)
{
    if(e->isPreDeactivation())
    {
        d->active = false;
        emit activeChanged(d->active);
    }

    GCF::Component::deactivationEvent(e);
}

/**
This event handler is implemented to look for a \c type key in the information map
(\ref GCF::ContentObjectLoadEvent::info()) provided by the event-object. If the type
is \c qml, then it looks for a \c url key in the information map and loads the \c .qml
file specified in \c url using the \ref loadQmlObject() method.

If the \c type key is not \c qml, then the base class implementation of this event
handler is called; which in turn calls the \ref loadObject() method.
 */
void GCF::QmlComponent::contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e)
{
    if(e->info().value("type").toString() == "qml")
    {
        QString urlStr = e->info().value("url").toString();
        QUrl url;
        QString scheme = urlStr.contains("://") ? urlStr.section("://", 0, 0) : QString();
        if(scheme.length() >= 3 && (scheme == "http" || scheme == "ftp" || scheme == "file" || scheme == "qrc"))
            url = QUrl(urlStr);
        else if(scheme.isEmpty())
        {
            bool condition = false;
#ifdef Q_OS_WIN
            condition = (urlStr.length() >= 3 && urlStr.at(0).isDigit() && urlStr.at(1) == ':' && urlStr.at(2) == '/');
#else
            condition = (urlStr.length() >= 1 && urlStr.at(0) == '/');
#endif
            if(!condition)
                urlStr = QFileInfo(urlStr).absoluteFilePath();

            if(!QFileInfo(urlStr).exists())
                GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                            QString("QML file %1 not found").arg(e->info().value("url").toString()));
            else
                url = QUrl::fromLocalFile(urlStr);
        }
        else
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                        QString("Cannot recognize URL scheme in %1. Only http, ftp, "
                                                "file and qrc schemes are supported").arg(e->info().value("url").toString()));

        if(url.isValid())
        {
            QObject *object = this->loadQmlObject(e->objectName(), url, e->info());
            e->setObject(object);
        }

        return;
    }

    GCF::Component::contentObjectLoadEvent(e);
}

/**
This event hanlder is implemented to ignore merge events for QML objects
 */
void GCF::QmlComponent::contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e)
{
    if(e->childInfo().value("type").toString() == "qml" ||
            e->parentInfo().value("type").toString() == "qml")
        return; // ignore

    GCF::Component::contentObjectMergeEvent(e);
}

/**
This event handler is implemented to look for a \c type key in the information map
(\ref GCF::ContentObjectLoadEvent::info()) provided by the event-object. If the type
is \c qml, then it calls the \ref unloadQmlObject() method to unload the QML object.

If the \c type key is not \c qml, then the base class implementation of this event
handler is called; which in turn calls the \ref unloadObject() method.
 */
void GCF::QmlComponent::contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e)
{
    if(e->info().value("type").toString() == "qml")
        e->setAccepted( this->unloadQmlObject(e->objectName(), e->object(), e->info()) );
    else
        GCF::Component::contentObjectUnloadEvent(e);
}

/**
This event hanlder is implemented to ignore unmerge events for QML objects
 */
void GCF::QmlComponent::contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e)
{
    if(e->childInfo().value("type").toString() == "qml" ||
            e->parentInfo().value("type").toString() == "qml")
        return; // ignore

    GCF::Component::contentObjectUnmergeEvent(e);
}

/**
This event hanlder is implemented to ignore activation events for QML objects
 */
void GCF::QmlComponent::activateContentObjectEvent(GCF::ActivateContentObjectEvent *e)
{
    if(e->childInfo().value("type").toString() == "qml" ||
            e->parentInfo().value("type").toString() == "qml")
        return; // ignore

    GCF::Component::activateContentObjectEvent(e);
}

/**
This event hanlder is implemented to ignore deactivation events for QML objects
 */
void GCF::QmlComponent::deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e)
{
    if(e->childInfo().value("type").toString() == "qml" ||
            e->parentInfo().value("type").toString() == "qml")
        return; // ignore

    GCF::Component::deactivateContentObjectEvent(e);
}

/**
This function can be implemented to customize the loading a QML object from \c url. The default
implementation loads the QML file at URL and returns a pointer to it. The default implementation
also ensures that a \c gComponent object is available in the QML code that offers access to this
component class.

\param name name of the QML object that needs to be loaded
\param url URL from which the QML file needs to be loaded. If you reimplement this function you must
ensure that \c http, \c ftp, \c qrc and \c file URL schemes are fully supported.
\param info key=value information map associated with the QML object that needs to be loaded
\return pointer to the QML object that was loaded.
 */
QObject *GCF::QmlComponent::loadQmlObject(const QString &name, const QUrl &url, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    return d->loadQml(url);
}

/**
This function can be implemented to customize the unloading of a QML object. The default implementation
simply deletes the QML object.

\param name name of the QML object that needs to be unloaded
\param qmlObject pointer to the QMl object that needs to be unloaded
\param info key=value information map associated with the QML object that needs to be unloaded
\return true on successful unloading of the object, false otherwise. The default implemnetation
always returns true.
 */
bool GCF::QmlComponent::unloadQmlObject(const QString &name, QObject *qmlObject, const QVariantMap &info)
{
    Q_UNUSED(name);
    Q_UNUSED(info);
    delete qmlObject;
    return false;
}

void GCF::QmlComponent::onComponentStatusChanged(QQmlComponent::Status status)
{
    QQmlComponent *qmlComp = qobject_cast<QQmlComponent*>(this->sender());
    if(qmlComp)
    {
        switch(status)
        {
        case QQmlComponent::Loading:
        case QQmlComponent::Null:
            break; // wait for the next update
        case QQmlComponent::Ready:
            d->logComponentSuccess(qmlComp);
            // qmlComp will be deleted by logComponentSuccess
            break;
        case QQmlComponent::Error:
            d->logComponentError(qmlComp);
            // qmlComp will be deleted by logComponentError
            break;
        }
    }
    else
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Slot is called inapprpriately");
}

QObject *GCF::QmlComponentData::loadQml(const QUrl &url)
{
    QString name = url.path();
    while(name.length() && name.at(0) == '/')
        name.remove(0, 1);

    if(!this->context)
    {
        // This is the context for the component itself. Each
        // QML item loaded from an item will be created within its own
        // context, that is a child of this context.
        this->context = new QQmlContext(gApp->qmlEngine(), component);
        this->context->setContextProperty("gComponent", component);
    }

    QQmlComponent *qmlComp = new QQmlComponent(gApp->qmlEngine(), component);
    qmlComp->setObjectName(name);
    qmlComp->loadUrl(url);

    QQmlContext *qmlObjContext = new QQmlContext(this->context, component);
    QObject *qmlObj = qmlComp->create(qmlObjContext);
    if(!qmlObj)
    {
        this->logComponentError(qmlComp);
        delete qmlObjContext;
        delete qmlComp;
        return nullptr;
    }

    qmlObj->setObjectName(name);

    // We want the context of this object to automatically get deleted when the
    // object gets deleted. We can either do a signal/slot connection or we
    // can simply reparent the context to the object. I choose to do the latter
    // because it is simple.
    qmlObjContext->setParent(qmlObj);

    // By the time this function returns, qmlComp must either be deleted or scheduled
    // to be deleted. In case of error; qmlObj must also be deleted.

    if(qmlComp->isReady())
    {
        this->logComponentSuccess(qmlComp);
        // qmlComp will be deleted by logComponentSuccess
    }
    else if(qmlComp->isError())
    {
        this->logComponentError(qmlComp);
        delete qmlObj;
        qmlObj = nullptr;
        // qmlComp will be deleted by logComponentError
    }
    else
    {
        QObject::connect(qmlComp, SIGNAL(statusChanged(QQmlComponent::Status)),
                         component, SLOT(onComponentStatusChanged(QQmlComponent::Status)));
        // qmlComp will be deleted from the onComponentStatusChanged() slot.
    }

    return qmlObj;
}

void GCF::QmlComponentData::logComponentSuccess(QQmlComponent *qmlComp)
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("QML Object %1 loaded successfully").arg(qmlComp->objectName()));
    qmlComp->deleteLater();
}

void GCF::QmlComponentData::logComponentError(QQmlComponent *qmlComp)
{
    GCF::LogMessageBranch branch( QString("Error while loading QML Object %1").arg(qmlComp->objectName()) );

    QList<QQmlError> errors = qmlComp->errors();
    Q_FOREACH(QQmlError error, errors)
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("Error %1:%2 - %3")
                                   .arg(error.url().toString())
                                   .arg(error.line())
                                   .arg(error.description()));
    }

    qmlComp->deleteLater();
}

