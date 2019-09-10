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

/**
\ingroup gcf_core
\class GCF::ApplicationServices Application.h <GCF3/Application>

\brief This class abstracts the common functionalities provided
       by the different types of applications in GCF3

All application configurations in GCF inherrit this class so that
they can offer the following capabilities

<br/>

Read about the \ref gcf_component_model to learn how this can be used in the application.

<br/>

The single global instance of this class in the application can be accessed
using the \c gAppService global pointer.
*/

#include "Application.h"
#include "Application_p.h"

#include "ObjectTree.h"
#include "Component.h"
#include "Log.h"
#include "Version.h"

#include <QDir>
#include <QLibrary>
#include <QFileInfo>
#include <QMetaType>
#include <QSettings>
#include <QMetaMethod>
#include <QMetaObject>
#include <QDomElement>
#include <QDomDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace GCF
{

struct ComponentInfo
{
    ComponentInfo() : Active(false),
        Initialized(false) { }

    bool Active;
    bool Initialized;
};

struct ApplicationServicesData
{
    QDateTime launchTimestamp;
    ObjectTree objectTree;
    ObjectMap<ComponentInfo> componentMap;
    QVariantMap argumentsMap;
    GCF::JobListModel jobs; // global jobs-list

    void inductComponentIntoApp(Component *component);
    void initializeComponent(Component *component);
    void loadComponentSettings(Component *component);
    void loadComponentContent(Component *component);
    void loadComponentContent(Component *component, const QString &contentFile);
    void loadComponentContentXml(Component *component, const QString &contentFile);
    void loadComponentObjectDetails(Component *component, QObject *object, QDomElement objectE);
    void activateComponent(Component *component);

    void expungComponentFromApp(Component *component);
    void deactivateComponent(Component *component);
    void unloadComponentContent(Component *component);
    void unloadComponentSettings(Component *component);

    QDomElement firstChildElement(QDomElement parentE, const QString &childTag) {
        QDomNodeList children = parentE.childNodes();
        for(int i=0; i<children.count(); i++)
            if(children.at(i).toElement().tagName().toLower() == childTag)
                return children.at(i).toElement();
        return QDomElement ();
    }

    QString completePath(GCF::Component *component, const QString &path) {
        QStringList fields = path.split(".", QString::SkipEmptyParts);
        if(fields.count() == 0)
            return QString();
        if(fields.count() == 1) {
            fields.prepend(component->name());
            fields.prepend(gAppService->objectTree()->rootNode()->name());
        } else if(fields.count() == 2)
            fields.prepend(gAppService->objectTree()->rootNode()->name());
        return fields.join(".");
    }

    QMetaMethod findMethod(const QObject *object, const QString &methName) const {
        if(!object || methName.isEmpty())
            return QMetaMethod();
        const QMetaObject *mo = object->metaObject();
        for(int i=mo->methodCount()-1; i>=0; i--) {
            QMetaMethod method = mo->method(i);
#if QT_VERSION >= 0x050000
            if(method.methodSignature() == methName.toLatin1())
#else
            if(QString::fromLatin1(method.signature()) == methName)
#endif
                return method;
        }
        return QMetaMethod();
    }

    void loadArgumentsMap(const QStringList &args) {
        this->argumentsMap.clear();
        Q_FOREACH(QString arg, args) {
            if(arg.startsWith('-')) {
                // QStringList fields = arg.split(':', QString::SkipEmptyParts);
                QString key = arg.section(':', 0, 0);
                QString value = arg.section(':', 1);
                this->argumentsMap[key] = value;
            }
        }
    }
};

}

/**
 * Creates ApplicationServices instance
 */
static GCF::ApplicationServices *ApplicationServicesInstance = 0;
GCF::ApplicationServices *GCF::ApplicationServices::instance()
{
    return ::ApplicationServicesInstance;
}

/**
 * Constructor
 */
GCF::ApplicationServices::ApplicationServices()
{
    qRegisterMetaType<GCF::Component*>("GCF::Component");
    qRegisterMetaType<GCF::Result>("GCF::Result");
    qRegisterMetaType<GCF::Version>("GCF::Version");
    qRegisterMetaType<GCF::AbstractJob*>("GCF::AbstractJob*");
    qRegisterMetaType<QModelIndex>("QModelIndex");

    qRegisterMetaType<QJsonValue>("QJsonValue");
    qRegisterMetaTypeStreamOperators<QJsonValue>("QJsonValue");

    qRegisterMetaType<QJsonObject>("QJsonObject");
    qRegisterMetaTypeStreamOperators<QJsonObject>("QJsonObject");

    qRegisterMetaType<QJsonArray>("QJsonArray");
    qRegisterMetaTypeStreamOperators<QJsonArray>("QJsonArray");


    d = new ApplicationServicesData;
    d->launchTimestamp = QDateTime::currentDateTime();
    ::ApplicationServicesInstance = this;
}

/**
 * Destructor
 */
GCF::ApplicationServices::~ApplicationServices()
{
    ::ApplicationServicesInstance = 0;
    delete d;
}

/**
 * Returns the date-time at which application was launched.
 */
QDateTime GCF::ApplicationServices::launchTimestamp() const
{
    return d->launchTimestamp;
}

/**
 * Returns the object-tree of the application.
 */
GCF::ObjectTree *GCF::ApplicationServices::objectTree() const
{
    return &(d->objectTree);
}

/**
 * Looks for the exposed object specfied by \c path in the application's
 * object-tree, and returns it if found.
 * Usage \code gFindObject("Application.GCF_Component.ObjectType1"); \endcode
 *
 * \param hierarchical path to the object inside object tree, separated by '.'
 */
QObject *GCF::ApplicationServices::findObject(const QString &path) const
{
    return d->objectTree.object(path);
}

/**
 * Loads the component into the application. This function will add the
 * specified component into the object tree and will initialize it.
 * For more details on initialization refer \ref GCF::Component.
 *
 * \param component pointer to the component which needs to be loaded.
 */
void GCF::ApplicationServices::loadComponent(GCF::Component *component)
{
    if(!component || d->componentMap.contains(component))
        return;

    d->inductComponentIntoApp(component);
}

/**
 * Unloads the component from the application. Unloading the component will
 * remove the component node from the applciation's object-tree.
 * For more details on unloading refer \ref GCF::Component.
 *
 * \param component pointer to the component which needs to be unloaded.
 */
void GCF::ApplicationServices::unloadComponent(GCF::Component *component)
{
    if(!component || d->componentMap.contains(component))
        d->expungComponentFromApp(component);
}

/**
 * \return list of components loaded by the application.
 */
QObjectList GCF::ApplicationServices::components() const
{
    return d->componentMap.keys();
}

/**
 * Activates the component specified by \c component. For
 * more details refer to \ref GCF::Component.
 *
 * \note to activate a component it should have been loaded by the application
 * first.
 *
 * \param component pointer to the component that needs to be activated.
 */
void GCF::ApplicationServices::activateComponent(GCF::Component *component)
{
    if(d->componentMap.contains(component))
        d->activateComponent(component);
}

/**
 * Deactivates the component specified by \c component. For
 * more details refer to \ref GCF::Component.
 *
 *\note to deactivate a component it should have been loaded by the application
 * first.
 *
 *\param component pointer to the component that needs to be deactivated.
 */
void GCF::ApplicationServices::deactivateComponent(GCF::Component *component)
{
    if(d->componentMap.contains(component))
        d->deactivateComponent(component);
}

/**
 * \return true if component is loaded; false otherwise.
 */
bool GCF::ApplicationServices::isLoaded(const GCF::Component *component) const
{
    GCF::Component *comp = const_cast<GCF::Component*>(component);
    return d->componentMap.value(comp).Initialized;
}

/**
 * \return true if component is active; false otherwise.
 */
bool GCF::ApplicationServices::isActive(const GCF::Component *component) const
{
    GCF::Component *comp = const_cast<GCF::Component*>(component);
    return d->componentMap.value(comp).Active;
}

/**
 * Unloads all the components loaded by the application. Also removes them
 * from the object tree.
 */
void GCF::ApplicationServices::unloadAllComponents()
{
    GCF::LogMessageBranch branch("Unloading all components");

    QList<ObjectTreeNode*> componentNodes = d->objectTree.rootNode()->children();
    for(int i=componentNodes.count()-1; i>=0; i--)
    {
        ObjectTreeNode *componentNode = componentNodes.at(i);
        GCF::Component *component = (GCF::Component*)(componentNode->object());
        this->unloadComponent(component);
    }
}

typedef const char *(*BuildVersionFunction)();
typedef GCF::Component *(*CreateInstanceFunction)();

/**
 Creates an instance of the component from the specified file.  The
 instantiation will fail if the file does not exist or if version of the
 component is not same as the application version.

 \note This function does not load the component. For loading the component
 call \ref GCF::ApplicationServices::loadComponent(Component *
 component) on the component pointer returned by this function.

 \note Do not specify the file extension while invoking this function

 Usage
 \code
 GCF::Component *component = gApp->instantiateComponent("Components/SimpleComponent");
 \endcode

 \param libraryName full or relative path to the library file.
*/
GCF::Component *GCF::ApplicationServices::instantiateComponent(const QString &libraryName)
{
    GCF::LogMessageBranch branch( QString("Creating component from library %1").arg(libraryName) );

    QString libraryFile = GCF::findLibrary(libraryName);
    if( libraryFile.isEmpty() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Could not find component library on this system");
        return 0;
    }

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Loading component from %1").arg(libraryFile));

    QLibrary library(libraryFile);
    if( !library.load() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Error while loading component: %1").arg(library.errorString()));
        return 0;
    }

    BuildVersionFunction versionFn = (BuildVersionFunction)(library.resolve("GCF3ComponentBuildVersion"));
    CreateInstanceFunction createFn = (CreateInstanceFunction)(library.resolve("GCF3CreateComponentInstance"));
    if(versionFn == 0 || createFn == 0)
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Library doesn't contain a GCF3 component");
        library.unload();
        return 0;
    }

    QString compVersionStr = QString::fromLatin1( versionFn() );
    GCF::Version compVersion(compVersionStr);
    if( !compVersion.isValid() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Component has an invalid version number: " + compVersionStr);
        library.unload();
        return 0;
    }

    if( compVersion != GCF::version() )
    {
        if( compVersion > GCF::version() )
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                        QString("Component is built for a higher version of GCF3 than the one used by this application. "
                                                "This application uses GCF %1, whereas the component uses %2.")
                                        .arg(GCF::version()).arg(compVersion));
        else
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                        QString("Component is built for a lower version of GCF3 than the one used by this application. "
                                                "This application uses GCF %1, whereas the component uses %2.")
                                        .arg(GCF::version()).arg(compVersion));
        library.unload();
        return 0;
    }

    GCF::Component *component = createFn();
    if(component == 0)
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Component library did not return any component object.");
        library.unload();
        return 0;
    }

    return component;
}

/**
 Instantiates the component from the file \c libraryName and loads it.

 \note Do not specify the file extension while invoking this function

 Usage
 \code
 GCF::Component *component = gApp->loadComponent("Components/SimpleComponent");
 \endcode

 \param libraryName full or relative path to the library file.
 */
GCF::Component *GCF::ApplicationServices::loadComponent(const QString &libraryName)
{
    GCF::LogMessageBranch branch( QString("Loading component from %1").arg(libraryName) );

    GCF::Component *component = this->instantiateComponent(libraryName);
    this->loadComponent(component);

    return component;
}

/**
 Overloads \ref GCF::ApplicationServices::loadComponent(Component *component).
 Loads the specified component and initializes it with the properties specified
 in \c properties.
 
 Usage
 \code
 QList< QPair<QByteArray,QVariant> > properties;
 properties << qMakePair<QByteArray,QVariant>("name", "ChangedSimpleComponent");
 properties << qMakePair<QByteArray,QVariant>("parameter1", 20);
 properties << qMakePair<QByteArray,QVariant>("property2", 456.123);
 properties << qMakePair<QByteArray,QVariant>("date", QDate::currentDate());
 
 GCF::Component *component = gApp->loadComponent("Components/SimpleComponent", properties);
 \endcode

  \param libraryName path to the library file.
  \param properties properties that needs to be set on component before loading
 */
GCF::Component *GCF::ApplicationServices::loadComponent(const QString &libraryName, const QList< QPair<QByteArray,QVariant> >& properties)
{
    GCF::LogMessageBranch branch( QString("Loading component from %1 with %2 properties").arg(libraryName).arg(properties.count()) );

    GCF::Component *component = this->instantiateComponent(libraryName);

    QPair<QByteArray,QVariant> prop;
    Q_FOREACH(prop, properties)
        component->setProperty(prop.first, prop.second);

    this->loadComponent(component);

    return component;
}

/**
\fn QList<Component*> GCF::ApplicationServices::loadComponents(const QStringList &libraries)

This function loops over each and every library name provided in \c libraries and invokes the
\ref loadComponent() method to load components. It returns a list of components that got
loaded.
*/

/**
 * \return \c QVariantMap which contains all the command line arguments.
 */
QVariantMap GCF::ApplicationServices::argumentsMap() const
{
    if(d->argumentsMap.isEmpty())
        d->loadArgumentsMap(QCoreApplication::arguments());
    return d->argumentsMap;
}

/**
 Parses the list of arguments supplied to the application. This method
 expects the strings in the arguments to be in a "-key:value" format.
 This would then be loaded as <key, value> pair in to an internal map which
 can then be retrieved using \ref GCF::ApplicationServices::argumentsMap().

 By default this method processes \c --loadComponents argument as
 a command, and can be used to load a comma separated list of components.

 Usage
  \code
  QStringList args;
  args << "--loadComponents:Components/SimpleComponent,Components/GuiComponent";
  gApp->processArguments(args);
  \endcode

  @param additionalArgs can be used to supply additional arguments, and is optional.
 */
void GCF::ApplicationServices::processArguments(const QStringList &additionalArgs)
{
    GCF::LogMessageBranch branch("Processing command-line arguments");

    const QStringList args = qApp->arguments() + additionalArgs;
    d->loadArgumentsMap(args);

    // Process known arguments
    QString loadComponentsArg = d->argumentsMap.value("--loadComponents").toString();
    QStringList componentNames = loadComponentsArg.isEmpty() ? QStringList() : loadComponentsArg.split(',');
    this->loadComponents(componentNames);
}

/**
 * @brief This method can be used to invoke a named method in an object
 * @param path complete path of the object in the \ref objectTree()
 * @param method name of the method to invoke in the object. (Only the name. Not the complete signature)
 * @param args list of arguments to to pass to the method
 * @param secureCall if true, then the function will check for allowmetaaccess attribute. If false, then it will ignore the attribute.
 * @return Upon success this function will return a true. Calling \ref GCF::Result::data() "data()" on the returned result
 * will provide the value returned by the invoked method.
 *
 * \note Only public methods and signals can be invoked by this function.
 *
 * \sa \ref gcf_content_xml_32
 */
GCF::Result GCF::ApplicationServices::invokeMethod(const QString &path, const QString &method, const QVariantList &args, bool secureCall) const
{
    return InvokeMethodHelper(secureCall).call(path, method, args);
}

/**
 * @brief This method can be used to invoke a named method in an object
 * @param object pointer to a \c QObject in which a method needs to be invoked
 * @param method name of the method to invoke in the object. (Only the name. Not the complete signature)
 * @param args list of arguments to to pass to the method
 * @param secureCall if true, then the function will check for allowmetaaccess attribute. If false, then it will ignore the attribute.
 * @return Upon success this function will return a true. Calling \ref GCF::Result::data() "data()" on the returned result
 * will provide the value returned by the invoked method.
 *
 * \note Only public methods and signals can be invoked by this function.
 *
 * \sa \ref gcf_content_xml_32
 */
GCF::Result GCF::ApplicationServices::invokeMethod(QObject *object, const QString &method, const QVariantList &args, bool secureCall)
{
    if(!object || method.isEmpty())
        return GCF::Result(false);

    return InvokeMethodHelper(secureCall).call(object, method, args);
}

/**
 * @brief This method can be used to invoke a named method in an object
 * @param object pointer to a \c QObject in which a method needs to be invoked
 * @param method meta information of the method that needs to be invoked
 * @param args list of arguments to to pass to the method
 * @param secureCall if true, then the function will check for allowmetaaccess attribute. If false, then it will ignore the attribute.
 * @return Upon success this function will return a true. Calling \ref GCF::Result::data() "data()" on the returned result
 * will provide the value returned by the invoked method.
 *
 * \note Only public methods and signals can be invoked by this function.
 *
 * \sa \ref gcf_content_xml_32
 */
GCF::Result GCF::ApplicationServices::invokeMethod(QObject *object, const QMetaMethod &method, const QVariantList &args, bool secureCall)
{
    if(!object || method.enclosingMetaObject() != object->metaObject())
        return GCF::Result(false);

    return InvokeMethodHelper(secureCall).call(object, method, args);
}

/**
 \internal
 */
GCF::Result GCF::ApplicationServices::isMethodInvokable(const QMetaMethod &method, QObject *object)
{
    return InvokeMethodHelper().isMethodInvokable(method, object);
}

/**
 * \return the list of jobs for the application. See \ref GCF::AbstractJob
 * for more information.
 */
GCF::JobListModel *GCF::ApplicationServices::jobs() const
{
    return &d->jobs;
}

void GCF::ApplicationServicesData::inductComponentIntoApp(GCF::Component *component)
{
    GCF::ComponentInfo info;
    this->componentMap.insert(component, info);
    new ObjectTreeNode(this->objectTree.rootNode(), component->name(), component);

    // Begin inducting the component into the application
    GCF::LogMessageBranch branch( QString("Induction of component %1").arg(component->name()) );
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Starting induction...");
    this->initializeComponent(component);
    this->componentMap[component].Initialized = true;
    this->activateComponent(component);
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Completed induction.");
}

void GCF::ApplicationServicesData::initializeComponent(GCF::Component *component)
{
    /*
     * In this version of GCF, we will ignore whether any event
     * sent to the component was accepted or not. In a future release
     * we will provide an opportunity for component developers to
     * abort (and cleanly undo) initialization half-way through the
     * process.
     */

    GCF::LogMessageBranch branch( "Component initialization" );

    // Send pre-initialization event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-initialization event to component");
        GCF::InitializeEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    this->loadComponentSettings(component);
    this->loadComponentContent(component);

    // Send post-initialization event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-initialization event to component");
        GCF::InitializeEvent event(+1);
        qApp->sendEvent(component, &event);
    }
}

void GCF::ApplicationServicesData::loadComponentSettings(Component *component)
{
    QDir dir;
    dir.mkpath(GCF::settingsDirectory());
    dir = QDir(GCF::settingsDirectory());

    QString settingsFile;
    settingsFile = QString("%1.ini").arg( component->name() );
    settingsFile = dir.absoluteFilePath(settingsFile);

    GCF::LogMessageBranch branch( "Loading settings" );

    // Send pre-settings-load event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-settings-load event");
        GCF::SettingsLoadEvent event(settingsFile, -1);
        qApp->sendEvent(component, &event);

        if( !event.settingsFile().isEmpty() && QFile::exists(event.settingsFile()) )
            settingsFile = event.settingsFile();
    }

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Loading settings from %1").arg(settingsFile));

    QSettings *settings = new QSettings(settingsFile, QSettings::IniFormat, component);
    component->setSettings(settings);

    // Send post-settings load event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-settings-load event");
        GCF::SettingsLoadEvent event(settingsFile, +1);
        qApp->sendEvent(component, &event);
    }
}

void GCF::ApplicationServicesData::loadComponentContent(Component *component)
{
    QDir dir;
    dir.mkpath(GCF::contentDirectory());
    dir = QDir(GCF::contentDirectory());

    QString contentFile;
    contentFile = QString("%1.xml").arg( component->name() );
    contentFile = dir.absoluteFilePath(contentFile);

    GCF::LogMessageBranch branch( "Loading content" );

    // Load pre-content-load event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-content-load event");
        GCF::ContentLoadEvent event(contentFile, -1);
        qApp->sendEvent(component, &event);

        if( !event.contentFile().isEmpty() && QFile::exists(event.contentFile()) )
            contentFile = event.contentFile();
    }

    this->loadComponentContent(component, contentFile);

    // Load post-content-load event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-content-load event");
        GCF::ContentLoadEvent event(contentFile, +1);
        qApp->sendEvent(component, &event);
    }
}

void GCF::ApplicationServicesData::loadComponentContent(Component *component, const QString &contentFile)
{
    /*
     We want to support a fixed set of component settings file format.
     For the moment - XML, JSON and QML are under consideration. Lets not
     keep the settings format extensible and mess it up with too many
     file formats.
     */

    QFileInfo info(contentFile);
    if( !info.exists() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Aborting load-content, because content-file %1 doesnt exist")
                                    .arg(contentFile));
        return;
    }

    if( !info.exists() )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Aborting load-content, because content-file %1 cannot be opened for reading")
                                    .arg(contentFile));
        return;
    }

    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Loading content from %1").arg(contentFile));

    if(info.suffix() == "xml")
    {
        this->loadComponentContentXml(component, contentFile);
        return;
    }

    // TODO: Support JSON also
}

void GCF::ApplicationServicesData::loadComponentContentXml(Component *component, const QString &contentFile)
{
    GCF::LogMessageBranch branch("Loading ContentXML");

    QFile file(contentFile);
    file.open( QFile::ReadOnly );

    QDomDocument doc;
    int errLine=0, errCol=0;
    QString errMsg;
    if( !doc.setContent(&file, &errMsg, &errLine, &errCol) )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString("Error parsing ContentXML at %1:%2 - %3").arg(errLine).arg(errCol).arg(errMsg));
        return;
    }

    // Lets first include the object in the application's object-tree (under component's node)
    ObjectTreeNode *componentNode = this->objectTree.node(component);
    Q_ASSERT(componentNode != 0);

    /*
     * The ContentXML file is similar to GUIXML file of GCF 2.x. Only much much simpler.
     *
     *  <Content>
     *      <Object Name="..." Hint="..." Parent="..."/>
     *      <Object Name="..." Hint="..." Parent="..."/>
     *      <Object Name="..." Hint="..." Parent="..."/>
     *      <!-- ....... -->
     *  </Content>
     *
     * Tag names and attribute names are not case-sensitive.
     * (They used to be case-sensitive before)
     */

    QDomElement rootE = doc.documentElement();
    if( rootE.tagName().toLower() != "content" )
    {
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "ContentXML has an unknown root XML element");
        return;
    }

    QDomNodeList contentEList = rootE.childNodes();
    for(int i=0; i<contentEList.count(); i++)
    {
        QDomNode contentENode = contentEList.at(i);
        if(!contentENode.isElement())
            continue;

        QDomElement contentE = contentENode.toElement();
        if( contentE.tagName().toLower() != "object" )
        {
            GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Unknown XML element %1").arg(contentE.tagName()));
            continue;
        }

        GCF::LogMessageBranch objectBranch(QString("Loading object %1").arg(contentE.attribute("name")));

        QDomNamedNodeMap attrMap = contentE.attributes();
        QVariantMap objectInfo;
        for(int a=0; a<attrMap.count(); a++)
        {
            QDomAttr attr = attrMap.item(a).toAttr();
            objectInfo[ attr.name().toLower() ] = attr.value();
        }

        // Prepare to load the object
        QString objectName = objectInfo.value("name").toString();
        objectInfo.remove("name");

        QString parentName = objectInfo.value("parent").toString();
        objectInfo.remove("parent");

        if(objectName.isEmpty())
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Object name not specified");
            continue;
        }

        // For addressing GCF-12
        // We need to ensure that we dont send a content-load-event for another object
        // of the same name.
        if(componentNode->node(objectName))
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                QString("Duplicate object. An object by name %1 has already been loaded").arg(objectName));
            continue;
        }

        // Load the object by sending a ContentObjectLoad event to the component
        GCF::ContentObjectLoadEvent event(objectName, objectInfo);
        qApp->sendEvent(component, &event);

        // If no object was loaded - then report as warning, continue with the
        // next object.
        if(event.object() == 0)
        {
            GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                          QString("Object %1 was not loaded by the component").arg(objectName));
            continue;
        }

        // If an object was infact created, then
        // 0. Load object's properties and perform signal/slot connections
        // 1. Include the object in the application's object-tree (under component's node)
        // 2. Perform GUI merging
        this->loadComponentObjectDetails(component, event.object(), contentE);

        ObjectTreeNode *objectNode = new ObjectTreeNode(componentNode, objectName, event.object(), objectInfo);
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Object %1 was loaded").arg(objectName));

        // Check if GUI Merging is necessary
        if(parentName.isEmpty())
            continue;

        // Now perform GUI merging. First lookup the node for parent-object
        if( !parentName.isEmpty() )
        {
            if(!parentName.contains("."))
                parentName = QString("%1.%2.%3").arg(this->objectTree.rootNode()->name())
                        .arg(component->name()).arg(parentName);
            else if(parentName.section('.', 0, 0) != this->objectTree.rootNode()->name())
                parentName = QString("%1.%2").arg(this->objectTree.rootNode()->name()).arg(parentName);
        }
        ObjectTreeNode *parentObjectNode = this->objectTree.node(parentName);

        // If parent-object was not already present, then report an error and continue
        if(!parentObjectNode)
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                        QString("Object %1 could not be merged into %2, because no such parent was found.")
                                        .arg(objectName).arg(parentName));
            continue;
        }

        // Now - get the component of the parent-object
        Component *parentObjectComponent = qobject_cast<GCF::Component*>(parentObjectNode->parent()->object());
        if(!parentObjectComponent)
        {
            GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                        QString("Component of %1 was not found.").arg(parentName));
            continue;
        }

        // Now despatch a merge event.
        GCF::ContentObjectMergeEvent mergeEvent(parentObjectNode->object(), event.object(),
                                                parentObjectNode->info(), objectInfo);
        qApp->sendEvent(parentObjectComponent, &mergeEvent);
        objectNode->writableInfo()["parent"] = parentObjectNode->path();

        // At this point we assume that the merging was done.
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("Object %1 was merged into %2").arg(objectName).arg(parentName));
    }
}

void GCF::ApplicationServicesData::loadComponentObjectDetails(GCF::Component *component, QObject *object, QDomElement objectE)
{
    /*
    <content>

        <object name="button" > <!-- lets say the component creates a QPushButton -->

            <property>
                <key>text</key>
                <value>My Push Button</value>
            </property>

            <property>
                <key>text</key>
                <value>Window.buttonEditor::text</value>
            </property>

            <connection>
                <signal>clicked()</signal>
                <receiver>RocketLauncher::launch()</receiver>
            </connection>

            <connection>
                <signal>Window.buttonEditor::textChanged(QString)</signal>
                <receiver>setText(QString)</receiver>
            </connection>

        </object>

    </content>
    */

    if(!component || !object)
        return; // No need to log anything

    QString name = objectE.attribute("name");
    GCF::LogMessageBranch branch( QString("Loading object details for %1").arg(name) );

    QDomElement childE = objectE.firstChildElement();
    while(!childE.isNull())
    {
        QString tag = childE.tagName().toLower();
        if( tag == "property" )
        {
            QDomElement keyE = this->firstChildElement(childE, "key");
            QDomElement valueE = this->firstChildElement(childE, "value");
            if(!keyE.isNull() && !keyE.text().isEmpty())
            {
                GCF::LogMessageBranch propBranch( QString("Loading property %1").arg(keyE.text()) );
                QString valueStr = valueE.text();
                if(valueStr.isEmpty())
                    GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, "Cannot set empty property value");
                else if(valueStr.contains("::"))
                {
                    QString objectPath = this->completePath(component, valueStr.section("::", 0, 0));
                    QString propName = valueStr.section("::", 1, 1);
                    QObject *otherObject = gFindObject(objectPath);
                    if(!objectPath.isEmpty() && !propName.isEmpty() && otherObject != 0)
                        object->setProperty(keyE.text().toLatin1(), otherObject->property(propName.toLatin1()));
                    else
                        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                QString("Cannot evaluate value of %1").arg(valueStr));
                }
                else
                    object->setProperty(keyE.text().toLatin1(), QVariant(valueStr));
            }
        }
        else if( tag == "connection" )
        {
            QDomElement senderE = this->firstChildElement(childE, "sender");
            QDomElement receiverE = this->firstChildElement(childE, "receiver");

            GCF::LogMessageBranch connectionBranch( QString("Loading connection between %1 and %2")
                                              .arg(senderE.text()).arg(receiverE.text()) );
            QObject *sender = 0;
            QMetaMethod signalMethod;
            QObject *receiver = 0;
            QMetaMethod receiverMethod;

            if(!senderE.isNull() && !senderE.text().isEmpty())
            {
                QString signalText = senderE.text();
                if( signalText.contains("::") )
                {
                    sender = gFindObject(signalText.section("::", 0, 0));
                    signalMethod = this->findMethod(sender, signalText.section("::", 1, 1));
                }
                else
                {
                    sender = object;
                    signalMethod = this->findMethod(sender, signalText);
                }
            }

            if(!receiverE.isNull() && !receiverE.text().isEmpty())
            {
                QString memberText = receiverE.text();
                if( memberText.contains("::") )
                {
                    receiver = gFindObject(memberText.section("::", 0, 0));
                    receiverMethod = this->findMethod(receiver, memberText.section("::", 1, 1));
                }
                else
                {
                    receiver = object;
                    receiverMethod = this->findMethod(receiver, memberText);
                }
            }

            if(sender && signalMethod.enclosingMetaObject() && receiver && receiverMethod.enclosingMetaObject())
                QObject::connect(sender, signalMethod, receiver, receiverMethod);
            else
            {
                if(!sender || !signalMethod.enclosingMetaObject())
                    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                                QString("Cannot find signal info from %1").arg(senderE.text()));
                if(!receiver || !receiverMethod.enclosingMetaObject())
                    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                                QString("Cannot find member info from %1").arg(receiverE.text()));
            }
        }

        childE = childE.nextSiblingElement();
    }
}

void GCF::ApplicationServicesData::activateComponent(GCF::Component *component)
{
    GCF::LogMessageBranch branch( QString("Component Activation for %1").arg(component->name()) );
    if(this->componentMap.value(component).Active == true)
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Component already active!");
        return;
    }

    // Send pre-activation event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-activation event");
        GCF::ActivationEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    // Perform activation
    ObjectTreeNode *componentNode = this->objectTree.node(component);
    if( !componentNode )
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "No node was found for the component in the application's object-tree");
    else
    {
        QList<ObjectTreeNode*> componentObjectNodes = componentNode->children();
        Q_FOREACH(ObjectTreeNode *componentObjectNode, componentObjectNodes)
        {
            if(!componentObjectNode->object())
                continue; // This is not an error or log-worthy scenario

            QString parent = componentObjectNode->info().value("parent").toString();
            if(parent.isEmpty())
                continue;

            ObjectTreeNode *parentNode = this->objectTree.node(parent);
            if(!parentNode->object())
                continue; // This is not an error or log-worthy scenario

            Component *parentComponent = qobject_cast<GCF::Component*>(parentNode->parent()->object());
            if(!parentComponent)
            {
                GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                           QString("Parent %1 for object %1 no longer exists")
                                           .arg(parent).arg(componentObjectNode->name()));
                continue;
            }

            GCF::ActivateContentObjectEvent actEvent(parentNode->object(), componentObjectNode->object(),
                                                     parentNode->info(), componentObjectNode->info());
            qApp->sendEvent(parentComponent, &actEvent);
            GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                       QString("Object %1 was activated").arg(componentObjectNode->name()));
        }
    }

    // Sending post-activation event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-activation event");
        GCF::ActivationEvent event(+1);
        qApp->sendEvent(component, &event);
    }

    this->componentMap[component].Active = true;
}

void GCF::ApplicationServicesData::expungComponentFromApp(GCF::Component *component)
{
    GCF::LogMessageBranch branch( QString("Expunge of component %1").arg(component->name()) );

    // Send pre-finalize event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-finalize event to component.");
        GCF::FinalizeEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    this->deactivateComponent(component);
    this->unloadComponentContent(component);
    this->unloadComponentSettings(component);

    // Send post-finalize event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-finalize event to component.");
        GCF::FinalizeEvent event(+1);
        qApp->sendEvent(component, &event);
    }

    // Delete the sub-tree dedicated to the component
    ObjectTreeNode *componentNode = this->objectTree.node(component);
    delete componentNode;

    // Delete the component right away.
    delete component;
}

void GCF::ApplicationServicesData::deactivateComponent(GCF::Component *component)
{
    GCF::LogMessageBranch branch( QString("Component Deactivation for %1").arg(component->name()) );
    if(this->componentMap.value(component).Active == false)
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Component already deactivated!");
        return;
    }

    // Send pre-activation event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-deactivation event");
        GCF::DeactivationEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    // Perform activation
    ObjectTreeNode *componentNode = this->objectTree.node(component);
    if( !componentNode )
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "No node was found for the component in the application's object-tree");
    else
    {
        QList<ObjectTreeNode*> componentObjectNodes = componentNode->children();
        Q_FOREACH(ObjectTreeNode *componentObjectNode, componentObjectNodes)
        {
            if(!componentObjectNode->object())
                continue; // This is not an error or log-worthy scenario

            QString parent = componentObjectNode->info().value("parent").toString();
            if(parent.isEmpty())
                continue;

            ObjectTreeNode *parentNode = this->objectTree.node(parent);
            if(!parentNode->object())
                continue; // This is not an error or log-worthy scenario

            Component *parentComponent = qobject_cast<GCF::Component*>(parentNode->parent()->object());
            if(!parentComponent)
            {
                GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                           QString("Parent %1 for object %1 no longer exists")
                                           .arg(parent).arg(componentObjectNode->name()));
                continue;
            }

            GCF::DeactivateContentObjectEvent deactEvent(parentNode->object(), componentObjectNode->object(),
                                                         parentNode->info(), componentObjectNode->info());
            qApp->sendEvent(parentComponent, &deactEvent);
            GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                       QString("Object %1 was deactivated").arg(componentObjectNode->name()));
        }
    }

    // Sending post-activation event
    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-deactivation event");
        GCF::DeactivationEvent event(+1);
        qApp->sendEvent(component, &event);
    }

    this->componentMap[component].Active = false;
}

void GCF::ApplicationServicesData::unloadComponentContent(GCF::Component *component)
{
    GCF::LogMessageBranch branch( "Unloading content" );

    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-content-unload-event to the component.");
        GCF::ContentUnloadEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    ObjectTreeNode *componentNode = this->objectTree.node(component);
    QList<ObjectTreeNode*> objectNodes = componentNode ? componentNode->children() : QList<ObjectTreeNode*>();
    for(int i=objectNodes.count()-1; i>=0; i--)
    {
        ObjectTreeNode *objectNode = objectNodes.at(i);
        if(!objectNode->object())
            continue; // This is not an error or log-worthy issue

        QString parent = objectNode->info().value("parent").toString();

        GCF::LogMessageBranch objectBranch( QString("Unloading object %1").arg(objectNode->name()) );

        if(!parent.isEmpty())
        {
            // Unmerging is necessary
            ObjectTreeNode *parentNode = this->objectTree.node(parent);
            Component *parentComponent = qobject_cast<GCF::Component*>(parentNode->parent()->object());
            if(parentComponent)
            {
                GCF::ContentObjectUnmergeEvent event(parentNode->object(), objectNode->object(),
                                                     parentNode->info(), objectNode->info());
                qApp->sendEvent(parentComponent, &event);
                GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                           QString("Object %1 was unmerged from %2").arg(objectNode->name()).arg(parent));
            }
            else
                GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                            QString("Could not unmerge Object %1 from %2").arg(objectNode->name()).arg(parent));
        }

        // Unload the object itself
        GCF::ContentObjectUnloadEvent event(objectNode->name(), objectNode->object(), objectNode->info());
        qApp->sendEvent(component, &event);
        delete objectNode->object();

        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, QString("Object %1 was unloaded").arg(objectNode->name()));
    }

    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-content-unload-event to the component.");
        GCF::ContentUnloadEvent event(+1);
        qApp->sendEvent(component, &event);
    }
}

void GCF::ApplicationServicesData::unloadComponentSettings(GCF::Component *component)
{
    GCF::LogMessageBranch branch("Unloading component settings");

    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending pre-settings-unload-event to the component.");
        GCF::SettingsUnloadEvent event(-1);
        qApp->sendEvent(component, &event);
    }

    QSettings *settings = const_cast<QSettings*>( component->settings() );
    component->setSettings(0);
    delete settings;

    {
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Sending post-settings-unload-event to the component.");
        GCF::SettingsUnloadEvent event(+1);
        qApp->sendEvent(component, &event);
    }
}

// todo GCF::ApplicationServices::invokeMethod(const QString &path, const QString &method, const QVariantList &args, bool secureCall=true) const;
// todo GCF::ApplicationServices::invokeMethod(QObject *object, const QString &method, const QVariantList &args, bool secureCall=true);
// todo GCF::ApplicationServices::invokeMethod(QObject *object, const QMetaMethod &method, const QVariantList &args, bool secureCall=true);
// todo GCF::ApplicationServices::isMethodInvokable(const QMetaMethod &method, QObject *object=0);

/**
 * \fn GCF::ApplicationServices::translate(const QString &string) const
 * Returns the translation for \c string in "GCF" context, if it is available.
 *
 */

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::Application Application.h <GCF3/Application>
\brief \c GCF::Application represents the non-gui, terminal application
 configuration available in GCF3.

All non-gui, terminal applications should make use of this application configuration,
by creating an instance of \c GCF::Application just like how it is done with
\c QtCoreApplication

\code

int main(int argc, char **argv)
{
    GCF::Application app(argc, argv);
    return app.processArgumentsAndExec();
}

\endcode

\note When using this appplication configuration, you cannot load any gui/qml components
in your application.
*/

/**
 * Returns the instance of the application.
 */
GCF::Application *GCF::Application::instance()
{
    return qobject_cast<GCF::Application*>(qApp);
}

/**
 * Constructor.
 */
GCF::Application::Application(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAppAboutToQuit()));
}

/**
 * Destructor.
 */
GCF::Application::~Application()
{
}

/**
 * \fn GCF::Application::processArgumentsAndExec(const QStringList
 * &additionalArgs=QStringList())
 *
 * Processes the application arguments, and additional arguments if any,
 * and starts the application's event-loop.
 */

void GCF::Application::onAppAboutToQuit()
{
    GCF::LogMessageBranch branch("Application Shutdown");
    this->unloadAllComponents();
}

/**
\fn QObject *gFindObject(const QString &path)
\ingroup gcf_core

\htmlonly
<pre>
#include &lt;GCF3/Application&gt;
</pre>
\endhtmlonly

*/

/**
\fn QObject *gFindObject(const QString &path)
\ingroup gcf_core


\htmlonly
<pre>
#include &lt;GCF3/Application&gt;
</pre>
\endhtmlonly

Returns the object at \c path.
*/

/**
\fn T *gFindObject<T>(const QString &path, GCF::ObjectTreeNode **objectNode=0)
\ingroup gcf_core

\htmlonly
<pre>
#include &lt;GCF3/Application&gt;
</pre>
\endhtmlonly

Returns the object at \c path. If \c objectNode is specified, then it returns a pointer to the
tree-node that has the object registered.
*/

/**
\fn T *gFindObject<T>(GCF::ObjectTreeNode **objectNode=0)
\ingroup gcf_core

\htmlonly
<pre>
#include &lt;GCF3/Application&gt;
</pre>
\endhtmlonly

Searches for an object in the \ref GCF::ApplicationServices::objectTree() "object-tree" of type \c T
and returns a pointer to it. If no such object was found, then this function returns a NULL pointer.

If \c objectNode is specified, then it returns a pointer to the \ref GCF::ObjectTreeNode "tree-node"
that has the object registered.
*/

/**
\fn QList<T*> gFindObjects<T>(QList<GCF::ObjectTreeNode*> *objectNodes=0)
\ingroup gcf_core

\htmlonly
<pre>
#include &lt;GCF3/Application&gt;
</pre>
\endhtmlonly

Searches for all objects in the \ref GCF::ApplicationServices::objectTree() "object-tree" of type \c T
and returns a list of pointers to it. If no such object was found, then this function returns an
empty list.

If \c objectNodes is specified, then it returns list of \ref GCF::ObjectTreeNode "tree-nodes" through
it.
*/

