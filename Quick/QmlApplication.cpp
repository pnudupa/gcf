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

#include "QmlApplication.h"
#include "../Core/MapToObject.h"

#include <QQmlEngine>
#include <QQmlContext>

/**
\class GCF::QmlApplicationServices QmlApplication.h <GCF3/QmlApplication>
\brief Base-class for \ref GCF::QmlApplication
\ingroup gcf_quick


This class extends \ref GCF::ApplicationServices by offering certain Qml specific
application services in addition to what the base class offers. Its subclass
\ref GCF::QmlApplication should be instantiated for writing QML+GCF applications.
Whenever an instance of \ref GCF::QmlApplication is created, you must call the
\ref setQmlEngine() method to set the application engine that will be used by
your QML application.

You will never have to use this class directly, because in most cases you will create
an instance of \ref GCF::QmlApplication and use that. There is one exception though:
read documentation for \ref initQmlEngine() to know more.

\note This class is available only if GCF is compiled and linked against Qt 5.
 */

namespace GCF
{
struct QmlApplicationServicesData
{
    QmlApplicationServicesData() : qmlEngine(nullptr) { }

    QQmlEngine *qmlEngine;
};

class System : public QObject
{
    Q_OBJECT

public:
    System(QObject *parent=nullptr);
    ~System();

    Q_INVOKABLE QObject *findObject(const QString &path) const;
    Q_INVOKABLE QObject *findImplementation(const QString &className) const;
    Q_INVOKABLE QObjectList findImplementations(const QString &className) const;

    Q_INVOKABLE void addQmlItem(const QString &name, QObject *object);
    Q_INVOKABLE void removeQmlItem(const QString &name);

private:
    GCF::MapToObject<QString> m_itemMap;
};
}

Q_GLOBAL_STATIC(GCF::System, SystemObject)

static GCF::QmlApplicationServices *QmlApplicationServicesInstance=nullptr;

/**
This function returns pointer to the only instance of \ref GCF::QmlApplicationServices in the
application. The function returns NULL if no such instance was created.
 */
GCF::QmlApplicationServices *GCF::QmlApplicationServices::instance()
{
    return ::QmlApplicationServicesInstance;
}

/**
Constructor
 */
GCF::QmlApplicationServices::QmlApplicationServices()
{
    d = new QmlApplicationServicesData;
    ::QmlApplicationServicesInstance = this;
}

/**
Destructor
 */
GCF::QmlApplicationServices::~QmlApplicationServices()
{
    ::QmlApplicationServicesInstance=nullptr;
    delete d;
}

/**
Using this function you can set the QML engine for use with your GCF application. This
function has to be called before any QML code is loaded and evaluated by the engine.

\param engine pointer to a \c QQmlEngine that needs to be set as GCF's Qml engine

Example:
\code
int main(int argc, char **argv)
{
    GCF::QmlApplication a(argc, argv);

    QQuickView qmlView;
    a.setQmlEngine(qmlView.engine());
    qmlView.setSource(....);
    qmlView.show();

    return a.exec();
}
\endcode

This function sets two context-properties on the engine's root-context.

\li \c gApp - references the GCF application object
\li \c gcf - references an internal GCF object that offers the following methods
    - \c findObject(string) : returns an object at the specified path
    - \c findImplementation(string) : returns an object that implements the specified class/interface
    - \c findImplementations(string) : returns all objects that implement the specified
    class/interface
    - \c addQmlItem(string,object) : adds a QML item to the object tree with the first parameter
    as name. Once added, the item will be exposed as \c "Application.{name}"
    - \c removeQmlItem(string) : removes a QML item, whose name is specified in the parameter,
    from the object tree
    .

At the end the \ref initQmlEngine() method is called. You can subclass from
\ref GCF::QmlApplicationServices or \ref GCF::QmlApplication and reimplement this method
to customize further initialization of the \c engine.

\note this function can be called only once. Once the engine is set, further calls to this
function become a no-op.
*/
void GCF::QmlApplicationServices::setQmlEngine(QQmlEngine *engine)
{
    if(d->qmlEngine)
    {
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
            "Cannot change QQmlEngine on GCF::QmlApplication, once it has been set.");
        return;
    }

    d->qmlEngine = engine;

    if(!d->qmlEngine)
    {
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
            "Null QML Engine being set!");
        return;
    }

    d->qmlEngine->rootContext()->setContextProperty("gApp", gApp);
    d->qmlEngine->rootContext()->setContextProperty("gcf", ::SystemObject());

    this->initQmlEngine(d->qmlEngine);
}

/**
Returns pointer to the \c QQmlEngine that was set using the \ref setQmlEngine() method.
NULL pointer if no engine was set.
 */
QQmlEngine *GCF::QmlApplicationServices::qmlEngine() const
{
    return d->qmlEngine;
}

/**
\fn virtual void GCF::QmlApplicationServices::initQmlEngine(QQmlEngine *)

You can reimplement this function to customize the initialization of the \c QQmlEngine
passed as paramete to this function. This function is called from \ref setQmlEngine().
By the time this function is called, the \c gcf and \c gApp context properties have
already been set.

The default implementation does nothing.
 */

GCF::System::System(QObject *parent)
    :QObject(parent)
{
}

GCF::System::~System()
{
}

QObject *GCF::System::findObject(const QString &path) const
{
    return gFindObject(path);
}

QObject *GCF::System::findImplementation(const QString &className) const
{
    GCF::ObjectTreeNode *node = gApp->objectTree()->findObjectNode(className);
    if(node)
        return node->object();

    return nullptr;
}

QObjectList GCF::System::findImplementations(const QString &className) const
{
    QList<GCF::ObjectTreeNode*> nodes = gApp->objectTree()->findObjectNodes(className);
    QObjectList objects;
    Q_FOREACH(GCF::ObjectTreeNode *node, nodes)
        objects.append(node->object());

    return objects;
}

void GCF::System::addQmlItem(const QString &name, QObject *object)
{
    new GCF::ObjectTreeNode( gApp->objectTree()->rootNode(),
                             name, object );
}

void GCF::System::removeQmlItem(const QString &name)
{
    GCF::ObjectTreeNode *node = gApp->objectTree()->rootNode()->node(name);
    if(node)
        delete node;
}

/**
\class GCF::QmlApplication QmlApplication.h <GCF3/QmlApplication>
\brief Offers an application object for GCF QML applications
\ingroup gcf_quick


This class implements both \c QGuiApplication and \ref GCF::QmlApplicationServices.
The only function it offers is \ref processArgumentsAndExec(). You must create
an instance of this class if you are building a QML application in GCF and call
the \ref GCF::QmlApplicationServices::setQmlEngine() method to set a QML engine for
your application. Shown below is a typical \c main() function in a QML application

\code
#include <GCF3/QmlApplication>
#include <GCF3/QmlComponent>

int main(int argc, char **argv)
{
    GCF::QmlApplication a(argc, argv);

    QQuickView qmlView;

    // This part is very important. You must
    // set a QML engine with the application!
    a.setQmlEngine(qmlView.engine());

    ....
    ....
    ....

    return a.processArgumentsAndExec();
}
\endcode

\note This class is available only if GCF is compiled and linked against Qt 5.
\note Qml applications \b cannot use QtWidgets module
*/

/**
\fn GCF::QmlApplication::QmlApplication(int &argc, char **argv)

Constructor.

@param argc reference to an integer that holds number of command-line arguments
passed to the applicaiton.
@param argv pointer to an array of character strings that hold the command-line
arguments.
*/

/**
\fn GCF::QmlApplication::~QmlApplication()

Destructor
  */

/**
\fn void GCF::QmlApplication::processArgumentsAndExec(const QStringList &additionalArgs=QStringList())

Process all arguments from \c qApp->arguments() and \c additionalArgs and invokes
\ref QApplication::exec().

\sa GCF::Application::processArgumentsAndExec()
 */

#include "QmlApplication.moc"

