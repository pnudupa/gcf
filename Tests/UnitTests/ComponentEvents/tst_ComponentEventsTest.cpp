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

#include <QString>
#include <QtTest>

#include <GCF3/GCFGlobal>
#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/Component>
#include <GCF3/ObjectTree>
#include <GCF3/Application>

#include "AppEventListener.h"

#include "SimpleComponent.h"
int SimpleComponent::Counter = 0;
QList<QEvent*> SimpleComponent::Events;

#include "SimpleContentComponent.h"
#include "GenericComponent.h"

class ComponentEventsTest : public QObject
{
    Q_OBJECT
    
public:
    ComponentEventsTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testApplicationInit();
    void testApplicationProcessArguments();
    void testBasicComponentLoad();
    void testBasicComponentUnload();
    void testSimpleComponentLoad();
    void testSimpleComponentUnload();
    void testSimpleContentComponent();
    void testAddAndRemoveContentObject();
    void testMergeAndUnmerge();
    void testActivationAndDeactivation();
    void testSettings();
    void testDuplicateObject();
    void testLoadUnloadObject();
    void testMergUnmergObject();
    void testActivateDeactivateObject();

private:
    QString logFileContents(bool deleteFile=true) const;
};

ComponentEventsTest::ComponentEventsTest()
{
}

void ComponentEventsTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ComponentEventsTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ComponentEventsTest::cleanup()
{
    this->logFileContents();
}

void ComponentEventsTest::testApplicationInit()
{
    QVERIFY( gApp->findObject("Application") == gApp );
    QVERIFY( gFindObject("Application") == gApp );

    QString log = this->logFileContents();
    QVERIFY( log.isEmpty() );

    gApp->loadComponent(0);
}

void ComponentEventsTest::testApplicationProcessArguments()
{
    QStringList args;
    args << "--fileName:C:/Sample/Directory/File.txt";
    args << "--timeout:8912";

    gApp->processArguments(args);

    QVERIFY(gApp->argumentsMap().value("--fileName").toString() == "C:/Sample/Directory/File.txt");
    QVERIFY(gApp->argumentsMap().value("--timeout").toInt() == 8912);
}

void ComponentEventsTest::testBasicComponentLoad()
{
    GCF::Component *component = new GCF::Component;
    QVERIFY(component->isActive() == false);
    QVERIFY(component->isLoaded() == false);

    AppEventListener listener;
    gApp->installEventFilter(&listener);
    gApp->loadComponent(component);
    QVERIFY(component->isActive() == true);
    QVERIFY(component->isLoaded() == true);

    QList<EventInfo> expectedEvents;
    expectedEvents << EventInfo(component, "InitializeEvent");  // PRE
    expectedEvents << EventInfo(component, "SettingsLoadEvent"); // PRE
    expectedEvents << EventInfo(component, "SettingsLoadEvent"); // POST
    expectedEvents << EventInfo(component, "ContentLoadEvent"); // PRE
    expectedEvents << EventInfo(component, "ContentLoadEvent"); // POST
    expectedEvents << EventInfo(component, "InitializeEvent"); // POST
    expectedEvents << EventInfo(component, "ActivationEvent"); // PRE
    expectedEvents << EventInfo(component, "ActivationEvent"); // POST

    QList<EventInfo> events = listener.events();

    QVERIFY(expectedEvents.count() == events.count());
    for(int i=0; i<expectedEvents.count(); i++)
    {
        QVERIFY( expectedEvents.at(i).Component == events.at(i).Component );
        QVERIFY( expectedEvents.at(i).EventName == events.at(i).EventName );
    }

    QVERIFY( gApp->objectTree()->node(component)->path() == "Application.GCF_Component" );
    QVERIFY( gApp->objectTree()->object("Application.GCF_Component") == component );
}

void ComponentEventsTest::testBasicComponentUnload()
{
    GCF::Component *component = (GCF::Component*)(gApp->components().first());

    AppEventListener listener;
    gApp->installEventFilter(&listener);
    gApp->unloadComponent(component);

    QList<EventInfo> expectedEvents;
    expectedEvents << EventInfo(component, "FinalizeEvent"); // PRE
    expectedEvents << EventInfo(component, "DeactivationEvent"); // PRE
    expectedEvents << EventInfo(component, "DeactivationEvent"); // POST
    expectedEvents << EventInfo(component, "ContentUnloadEvent"); // PRE
    expectedEvents << EventInfo(component, "ContentUnloadEvent"); // POST
    expectedEvents << EventInfo(component, "SettingsUnloadEvent"); // PRE
    expectedEvents << EventInfo(component, "SettingsUnloadEvent"); // POST
    expectedEvents << EventInfo(component, "FinalizeEvent"); // PRE

    QList<EventInfo> events = listener.events();

    QVERIFY(expectedEvents.count() == events.count());
    for(int i=0; i<expectedEvents.count(); i++)
    {
        QVERIFY( expectedEvents.at(i).Component == events.at(i).Component );
        QVERIFY( expectedEvents.at(i).EventName == events.at(i).EventName );
    }

    QVERIFY( gApp->objectTree()->node(component) == 0 );
    QVERIFY( gApp->objectTree()->object("Application.GCF_Component") == 0 );
    QVERIFY( gApp->objectTree()->node("Application.GCF_Component") == 0 );
}

void ComponentEventsTest::testSimpleComponentLoad()
{
    SimpleComponent *component = new SimpleComponent;
    QVERIFY(component->events().count() == 0);

    gApp->loadComponent(component);
    QList<QEvent*> events = component->events();

    QVERIFY(events.count() == 8);

    QVERIFY(events.at(0)->type() == GCF::InitializeEvent::Kind);
    QVERIFY( ((GCF::InitializeEvent*)(events.at(0)))->isPreInitialize() == true );
    QVERIFY( ((GCF::InitializeEvent*)(events.at(0)))->isPostInitialize() == false );

    QVERIFY(events.at(1)->type() == GCF::SettingsLoadEvent::Kind);
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(1)))->isPreSettingsLoad() == true );
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(1)))->isPostSettingsLoad() == false );
    QString settingsFile = GCF::settingsDirectory() + "/SimpleComponent.ini";
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(1)))->settingsFile() == settingsFile );

    QVERIFY(events.at(2)->type() == GCF::SettingsLoadEvent::Kind);
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(2)))->isPreSettingsLoad() == false );
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(2)))->isPostSettingsLoad() == true );
    QVERIFY( ((GCF::SettingsLoadEvent*)(events.at(2)))->settingsFile() == settingsFile );

    QVERIFY(events.at(3)->type() == GCF::ContentLoadEvent::Kind);
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(3)))->isPreContentLoad() == true );
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(3)))->isPostContentLoad() == false );
    QString contentFile = GCF::contentDirectory() + "/SimpleComponent.xml";
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(3)))->contentFile() == contentFile );

    QVERIFY(events.at(4)->type() == GCF::ContentLoadEvent::Kind);
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(4)))->isPreContentLoad() == false );
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(4)))->isPostContentLoad() == true );
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(4)))->contentFile() == contentFile );

    QVERIFY(events.at(5)->type() == GCF::InitializeEvent::Kind);
    QVERIFY( ((GCF::InitializeEvent*)(events.at(5)))->isPreInitialize() == false );
    QVERIFY( ((GCF::InitializeEvent*)(events.at(5)))->isPostInitialize() == true );

    QVERIFY(events.at(6)->type() == GCF::ActivationEvent::Kind);
    QVERIFY( ((GCF::ActivationEvent*)(events.at(6)))->isPreActivation() == true );
    QVERIFY( ((GCF::ActivationEvent*)(events.at(6)))->isPostActivation() == false );

    QVERIFY(events.at(7)->type() == GCF::ActivationEvent::Kind);
    QVERIFY( ((GCF::ActivationEvent*)(events.at(7)))->isPreActivation() == false );
    QVERIFY( ((GCF::ActivationEvent*)(events.at(7)))->isPostActivation() == true );

    component->clear();
}

void ComponentEventsTest::testSimpleComponentUnload()
{
    QVERIFY(SimpleComponent::Counter == 1);

    SimpleComponent *component = (SimpleComponent*)(gApp->components().first());
    gApp->unloadComponent(component);

    QList<QEvent*> events = component->events();
    QVERIFY(events.count() == 8);

    QVERIFY(events.at(0)->type() == GCF::FinalizeEvent::Kind);
    QVERIFY( ((GCF::FinalizeEvent*)events.at(0))->isPreFinalize() == true );
    QVERIFY( ((GCF::FinalizeEvent*)events.at(0))->isPostFinalize() == false );

    QVERIFY(events.at(1)->type() == GCF::DeactivationEvent::Kind);
    QVERIFY( ((GCF::DeactivationEvent*)events.at(1))->isPreDeactivation() == true );
    QVERIFY( ((GCF::DeactivationEvent*)events.at(1))->isPostDeactivation() == false );

    QVERIFY(events.at(2)->type() == GCF::DeactivationEvent::Kind);
    QVERIFY( ((GCF::DeactivationEvent*)events.at(2))->isPreDeactivation() == false );
    QVERIFY( ((GCF::DeactivationEvent*)events.at(2))->isPostDeactivation() == true );

    QVERIFY(events.at(3)->type() == GCF::ContentUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(3))->isPreContentUnload() == true );
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(3))->isPostContentUnload() == false );

    QVERIFY(events.at(4)->type() == GCF::ContentUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(4))->isPreContentUnload() == false );
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(4))->isPostContentUnload() == true );

    QVERIFY(events.at(5)->type() == GCF::SettingsUnloadEvent::Kind);
    QVERIFY( ((GCF::SettingsUnloadEvent*)events.at(5))->isPreSettingsUnload() == true );
    QVERIFY( ((GCF::SettingsUnloadEvent*)events.at(5))->isPostSettingsUnload() == false );

    QVERIFY(events.at(6)->type() == GCF::SettingsUnloadEvent::Kind);
    QVERIFY( ((GCF::SettingsUnloadEvent*)events.at(6))->isPreSettingsUnload() == false );
    QVERIFY( ((GCF::SettingsUnloadEvent*)events.at(6))->isPostSettingsUnload() == true );

    QVERIFY(events.at(7)->type() == GCF::FinalizeEvent::Kind);
    QVERIFY( ((GCF::FinalizeEvent*)events.at(7))->isPreFinalize() == false );
    QVERIFY( ((GCF::FinalizeEvent*)events.at(7))->isPostFinalize() == true );

    component->clear();
    QVERIFY(SimpleComponent::Counter == 0);
}

void ComponentEventsTest::testSimpleContentComponent()
{
    SimpleContentComponent *component = new SimpleContentComponent;
    QVERIFY(component->events().count() == 0);
    QVERIFY(component->objectMap().isEmpty() == true);

    gApp->loadComponent(component);

    // Test whether the objects got loaded from the content-xml file or not
    QVERIFY(component->objectMap().count() == 3);
    QVERIFY(component->objectMap().keys().at(0) == "object1");
    QVERIFY(component->objectMap().keys().at(1) == "object2");
    QVERIFY(component->objectMap().keys().at(2) == "object3");
    QVERIFY(Object::Count() == 3);

    // Test whether ContentObjectLoadEvent were delivered properly and processed or not
    QList<QEvent*> events = component->events();
    QVERIFY(events.count() == 8+3);

    QVERIFY(events.at(4)->type() == GCF::ContentObjectLoadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(4))->objectName() == "object1" );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(4))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(4))->object() == Object::List().at(0) );

    QVERIFY(events.at(5)->type() == GCF::ContentObjectLoadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->objectName() == "object2" );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().isEmpty() == false );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().count() == 4 );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().keys() == QStringList() << "a" << "b" << "c" << "layoutposition" );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().value("a").toInt() == 10 );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().value("b").toInt() == 20 );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().value("c").toInt() == 30 );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->info().value("layoutposition").toString() == "1,2" );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(5))->object() == Object::List().at(1) );

    QVERIFY(events.at(6)->type() == GCF::ContentObjectLoadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(6))->objectName() == "object3" );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(6))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectLoadEvent*)events.at(6))->object() == Object::List().at(2) );

    QVERIFY(events.at(7)->type() == GCF::ContentLoadEvent::Kind);
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(7)))->isPreContentLoad() == false );
    QVERIFY( ((GCF::ContentLoadEvent*)(events.at(7)))->isPostContentLoad() == true );

    // Test whether loaded objects are stored properly in the object tree or not
    QVERIFY(gFindObject("Application.SimpleContentComponent.object1") == Object::List().at(0));
    QVERIFY(gFindObject("Application.SimpleContentComponent.object2") == Object::List().at(1));
    QVERIFY(gFindObject("Application.SimpleContentComponent.object3") == Object::List().at(2));

    // Lets delete object2 - and unload the component. We should get only 2 ContentObjectUnload events
    delete Object::List().at(1);

    // Ensure that the object did get deleted
    QVERIFY(gFindObject("Application.SimpleContentComponent.object1") == Object::List().first());
    QVERIFY(gFindObject("Application.SimpleContentComponent.object2") == 0);
    QVERIFY(gFindObject("Application.SimpleContentComponent.object3") == Object::List().last());
    QVERIFY(Object::Count() == 2);
    QObject *object1 = Object::List().first();
    QObject *object3 = Object::List().last();

    // Now unload the component
    component->clear();
    gApp->unloadComponent(component);
    QVERIFY(Object::Count() == 0);

    // Test whether the 2 ContentObjectUnload events did get delivered or not
    events = component->events();
    QVERIFY(events.count() == 10);

    // Note objects get deleted in the reverse order
    QVERIFY(events.at(4)->type() == GCF::ContentObjectUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->objectName() == "object3" );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->object() == object3 );

    QVERIFY(events.at(5)->type() == GCF::ContentObjectUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->objectName() == "object1" );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->object() == object1 );

    QVERIFY(events.at(6)->type() == GCF::ContentUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(6))->isPreContentUnload() == false );
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(6))->isPostContentUnload() == true );

    component->clear();
}

void ComponentEventsTest::testAddAndRemoveContentObject()
{
    SimpleComponent *component = new SimpleComponent;
    gApp->loadComponent(component);

    // Add few content objects
    component->addContentObject("object1", new Object);
    component->addContentObject("object2", new Object);
    component->addContentObject("object3", new Object);

    // Check if they exist
    QVERIFY(gFindObject("Application.SimpleComponent.object1") == Object::List().at(0));
    QVERIFY(gFindObject("Application.SimpleComponent.object2") == Object::List().at(1));
    QVERIFY(gFindObject("Application.SimpleComponent.object3") == Object::List().at(2));

    // Remove the custom objects
    component->removeContentObject("object1");
    component->removeContentObject("object2");
    component->removeContentObject("object3");

    // Check if they have disappeared
    QVERIFY(gFindObject("Application.SimpleComponent.object1") == 0);
    QVERIFY(gFindObject("Application.SimpleComponent.object2") == 0);
    QVERIFY(gFindObject("Application.SimpleComponent.object3") == 0);
    QVERIFY(Object::Count() == 3);
    QVERIFY(gApp->objectTree()->node("Application.SimpleComponent.object1") == 0);
    QVERIFY(gApp->objectTree()->node("Application.SimpleComponent.object2") == 0);
    QVERIFY(gApp->objectTree()->node("Application.SimpleComponent.object3") == 0);
    while(Object::Count()) delete Object::List().first();

    // Add some more custom objects and see if ContentUnloadEvent is delivered for those objects
    // while unloading the component
    component->addContentObject("object4", new Object);
    component->addContentObject("object5", new Object);
    component->clear();
    QObject *object4 = Object::List().first();
    QObject *object5 = Object::List().last();

    // Now unload the component
    component->clear();
    gApp->unloadComponent(component);
    QVERIFY(Object::Count() == 0);

    // Test whether the 2 ContentObjectUnload events did get delivered or not
    QList<QEvent*> events = component->events();
    QVERIFY(events.count() == 10);

    // Note objects get deleted in the reverse order
    QVERIFY(events.at(4)->type() == GCF::ContentObjectUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->objectName() == "object5" );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(4))->object() == object5 );

    QVERIFY(events.at(5)->type() == GCF::ContentObjectUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->objectName() == "object4" );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->info().isEmpty() == true );
    QVERIFY( ((GCF::ContentObjectUnloadEvent*)events.at(5))->object() == object4 );

    QVERIFY(events.at(6)->type() == GCF::ContentUnloadEvent::Kind);
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(6))->isPreContentUnload() == false );
    QVERIFY( ((GCF::ContentUnloadEvent*)events.at(6))->isPostContentUnload() == true );

    component->clear();
}

void ComponentEventsTest::testMergeAndUnmerge()
{
    GenericComponent *platformComponent = new GenericComponent;
    platformComponent->setContentFile(":/GenericComponent/PlatformComponent.xml");
    platformComponent->load();
    platformComponent->clear();

    QVERIFY( gFindObject("Application.PlatformComponent.workspace") != 0 );
    QVERIFY( gFindObject("Application.PlatformComponent.leftDockArea") != 0 );
    QVERIFY( gFindObject("Application.PlatformComponent.rightDockArea") != 0 );
    QVERIFY( gFindObject("Application.PlatformComponent.bottomDockArea") != 0 );

    GenericComponent *treeComponent = new GenericComponent;
    treeComponent->setContentFile(":/GenericComponent/TreeComponent.xml");
    treeComponent->load();
    treeComponent->clear();

    QVERIFY( gFindObject("Application.TreeComponent.treeView") != 0 );
    QVERIFY( gFindObject("Application.TreeComponent.hierarchy") != 0 );
    QVERIFY( gFindObject("Application.TreeComponent.editorArea") != 0 );

    GenericComponent *editorComponent = new GenericComponent;
    editorComponent->setContentFile(":/GenericComponent/EditorComponent.xml");
    editorComponent->load();
    editorComponent->clear();

    QVERIFY( gFindObject("Application.EditorComponent.textEditor") != 0 );
    QVERIFY( gFindObject("Application.EditorComponent.systemEditor") != 0 );

    QVERIFY(platformComponent->events().count() == 6); // 3 merge and 3 activate object
    QVERIFY(treeComponent->events().count() == 2); // 2 merge and 2 activate object
    QVERIFY(editorComponent->events().count() == 0);

    QList<QEvent*> events = platformComponent->events();
    QVERIFY(events.at(0)->type() == GCF::ContentObjectMergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(0))->parent() == gFindObject("Application.PlatformComponent.leftDockArea") );
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(0))->child() == gFindObject("Application.TreeComponent.treeView") );

    QVERIFY(events.at(1)->type() == GCF::ContentObjectMergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(1))->parent() == gFindObject("Application.PlatformComponent.rightDockArea") );
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(1))->child() == gFindObject("Application.TreeComponent.hierarchy") );

    QVERIFY(events.at(4)->type() == GCF::ContentObjectMergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(4))->parent() == gFindObject("Application.PlatformComponent.workspace") );
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(4))->child() == gFindObject("Application.EditorComponent.textEditor") );

    events = treeComponent->events();
    QVERIFY(events.at(0)->type() == GCF::ContentObjectMergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(0))->parent() == gFindObject("Application.TreeComponent.editorArea") );
    QVERIFY( ((GCF::ContentObjectMergeEvent*)events.at(0))->child() == gFindObject("Application.EditorComponent.systemEditor") );

    platformComponent->clear();
    treeComponent->clear();
    editorComponent->clear();

    // Lets see if unload events are delivered properly or not
    QObject *textEditorObject = gFindObject("Application.EditorComponent.textEditor");
    QObject *systemEditorObject = gFindObject("Application.EditorComponent.systemEditor");
    editorComponent->unload();
    QVERIFY(Object::Count() == 7);

    QVERIFY(platformComponent->events().count() == 2); // 1 deactivate + 1 unmerge
    QVERIFY(treeComponent->events().count() == 2); // 1 deactivate + 1 unmerge

    events = platformComponent->events();

    QVERIFY(events.at(1)->type() == GCF::ContentObjectUnmergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(1))->parent() == gFindObject("Application.PlatformComponent.workspace") );
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(1))->child() == textEditorObject );

    events = treeComponent->events();
    QVERIFY(events.at(1)->type() == GCF::ContentObjectUnmergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(1))->parent() == gFindObject("Application.TreeComponent.editorArea") );
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(1))->child() == systemEditorObject );

    platformComponent->clear();
    treeComponent->clear();

    QObject *treeViewObject = gFindObject("Application.TreeComponent.treeView");
    QObject *hierarchyObject = gFindObject("Application.TreeComponent.hierarchy");
    treeComponent->unload();
    QVERIFY(Object::Count() == 4);

    QVERIFY(platformComponent->events().count() == 4); // 2 deactivate + 2 unmerge

    events = platformComponent->events();
    QVERIFY(events.at(2)->type() == GCF::ContentObjectUnmergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(2))->parent() == gFindObject("Application.PlatformComponent.rightDockArea") );
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(2))->child() == hierarchyObject );

    QVERIFY(events.at(3)->type() == GCF::ContentObjectUnmergeEvent::Kind);
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(3))->parent() == gFindObject("Application.PlatformComponent.leftDockArea") );
    QVERIFY( ((GCF::ContentObjectUnmergeEvent*)events.at(3))->child() == treeViewObject );

    platformComponent->clear();

    platformComponent->unload();
    QVERIFY(Object::Count() == 0);
}

void ComponentEventsTest::testActivationAndDeactivation()
{
    GenericComponent *platformComponent = new GenericComponent;
    platformComponent->setContentFile(":/GenericComponent/PlatformComponent.xml");
    platformComponent->load();

    GenericComponent *treeComponent = new GenericComponent;
    treeComponent->setContentFile(":/GenericComponent/TreeComponent.xml");
    treeComponent->load();

    platformComponent->clear();
    treeComponent->clear();

    // When the treeComponent is now deactivated; the platformComponent must get
    // 2 DeactivateContentObjectEvent events.
    // FIXME: #pragma message("Rewrite the following line after fixing GCF-30")
    ((GCF::Component*)treeComponent)->deactivate();

    QList<QEvent*> events = platformComponent->events();
    QVERIFY(events.count() == 2);

    QVERIFY(events.at(0)->type() == GCF::DeactivateContentObjectEvent::Kind);
    QVERIFY( ((GCF::DeactivateContentObjectEvent*)events.at(0))->parent() == gFindObject("Application.PlatformComponent.leftDockArea") );
    QVERIFY( ((GCF::DeactivateContentObjectEvent*)events.at(0))->child() == gFindObject("Application.TreeComponent.treeView") );

    QVERIFY(events.at(1)->type() == GCF::DeactivateContentObjectEvent::Kind);
    QVERIFY( ((GCF::DeactivateContentObjectEvent*)events.at(1))->parent() == gFindObject("Application.PlatformComponent.rightDockArea") );
    QVERIFY( ((GCF::DeactivateContentObjectEvent*)events.at(1))->child() == gFindObject("Application.TreeComponent.hierarchy") );

    events = treeComponent->events();
    QVERIFY(events.count() == 2);
    QVERIFY(events.at(0)->type() == GCF::DeactivationEvent::Kind);
    QVERIFY( ((GCF::DeactivationEvent*)events.at(0))->isPreDeactivation() == true );
    QVERIFY( ((GCF::DeactivationEvent*)events.at(0))->isPostDeactivation() == false );

    QVERIFY(events.at(1)->type() == GCF::DeactivationEvent::Kind);
    QVERIFY( ((GCF::DeactivationEvent*)events.at(1))->isPreDeactivation() == false );
    QVERIFY( ((GCF::DeactivationEvent*)events.at(1))->isPostDeactivation() == true );

    treeComponent->clear();
    platformComponent->clear();

    // When the treeComponent is now activated; the platformComponent must get
    // 2 ActivateContentObjectEvent events
    // FIXME: #pragma message("Rewrite the following line after fixing GCF-30")
    ((GCF::Component*)treeComponent)->activate();

    events = platformComponent->events();
    QVERIFY(events.count() == 2);

    QVERIFY(events.at(0)->type() == GCF::ActivateContentObjectEvent::Kind);
    QVERIFY( ((GCF::ActivateContentObjectEvent*)events.at(0))->parent() == gFindObject("Application.PlatformComponent.leftDockArea") );
    QVERIFY( ((GCF::ActivateContentObjectEvent*)events.at(0))->child() == gFindObject("Application.TreeComponent.treeView") );

    QVERIFY(events.at(1)->type() == GCF::ActivateContentObjectEvent::Kind);
    QVERIFY( ((GCF::ActivateContentObjectEvent*)events.at(1))->parent() == gFindObject("Application.PlatformComponent.rightDockArea") );
    QVERIFY( ((GCF::ActivateContentObjectEvent*)events.at(1))->child() == gFindObject("Application.TreeComponent.hierarchy") );

    events = treeComponent->events();
    QVERIFY(events.count() == 2);
    QVERIFY(events.at(0)->type() == GCF::ActivationEvent::Kind);
    QVERIFY( ((GCF::ActivationEvent*)events.at(0))->isPreActivation() == true );
    QVERIFY( ((GCF::ActivationEvent*)events.at(0))->isPostActivation() == false );

    QVERIFY(events.at(1)->type() == GCF::ActivationEvent::Kind);
    QVERIFY( ((GCF::ActivationEvent*)events.at(1))->isPreActivation() == false );
    QVERIFY( ((GCF::ActivationEvent*)events.at(1))->isPostActivation() == true );

    treeComponent->unload();
    platformComponent->unload();
}

void ComponentEventsTest::testSettings()
{
    QString settingsFile;

    // Write some settings and then load it back as is!
    GenericComponent *component = new GenericComponent;
    component->load();
    component->writableSettings()->setValue("Company", "VCreate Logic Pvt. Ltd.");
    component->writableSettings()->setValue("City", "Bangalore");
    component->unload();

    component = new GenericComponent;
    component->load();
    QVERIFY(component->settings()->value("Company").toString() == "VCreate Logic Pvt. Ltd.");
    QVERIFY(component->settings()->value("City").toString() == "Bangalore");
    component->unload();

    settingsFile = GCF::settingsDirectory() + "/GenericComponent.ini";
    QFile::remove(settingsFile);

    // Write a custom settings file and see if we can write/read from that
    settingsFile = GCF::applicationDataDirectoryPath() + "/TempSettings.ini";
    {
        QSettings settings(settingsFile, QSettings::IniFormat );
        settings.setValue("Language", "C++");
        settings.setValue("Platforms", "Windows, Linux, Mac");
    }

    component = new GenericComponent;
    component->setSettingsFile(settingsFile);
    component->load();
    QVERIFY(component->settings()->value("Language").toString() == "C++");
    QVERIFY(component->settings()->value("Platforms").toString() == "Windows, Linux, Mac");
    QVERIFY(component->settings()->contains("Company") == false);
    QVERIFY(component->settings()->contains("City") == false);
    component->unload();

    QFile::remove(settingsFile);
}

void ComponentEventsTest::testDuplicateObject()
{
    GenericComponent *component = new GenericComponent;
    component->setContentFile(":/GenericComponent/DuplicateObjects.xml");
    component->load();

    QList<QEvent*> events = component->events();
    QVERIFY(events.count() == 9);
    QVERIFY(events.at(4)->type() == GCF::ContentObjectLoadEvent::Kind);

    GCF::ContentObjectLoadEvent *e = (GCF::ContentObjectLoadEvent*)events.at(4);
    QVERIFY(e->objectName() == "object");
    QVERIFY(e->object() == Object::List().first());

    component->clear();
    component->unload();
}

void ComponentEventsTest::testLoadUnloadObject()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    GenericComponent *component = new GenericComponent;
    component->setContentFile(":/GenericComponent/LoadUnloadObjects.xml");
    component->LoadObjectEvents = &loadEvents;
    component->UnloadObjectEvents = &unloadEvents;
    component->load();

    QVERIFY(loadEvents.count() == 4);
    QVERIFY(unloadEvents.count() == 0);

    QStringList expectedNames = QStringList() << "object1" << "object2" << "object3"
                                              << "object4";
    QObjectList objects;
    for(int i=0; i<4; i++)
    {
        LoadEvent evt = loadEvents.at(i);

        QVERIFY(evt.Name == expectedNames.at(i));
        QVERIFY(evt.Info.value("type").toString() == "object");
        QVERIFY(evt.Object == Object::List().at(i));

        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    component->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 4);

    for(int i=0; i<4; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);

        QVERIFY(evt.Name == expectedNames.at(3-i));
        QVERIFY(evt.Info.value("type").toString() == "object");
        QVERIFY(evt.Object == objects.at(3-i));
    }
}

void ComponentEventsTest::testMergUnmergObject()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    GenericComponent *component = new GenericComponent;
    component->setContentFile(":/GenericComponent/MergeUnmergeObjects.xml");
    component->MergeEvents = &mergeEvents;
    component->UnmergeEvents = &unmergeEvents;
    component->load();

    QVERIFY(mergeEvents.count() == 2);
    QVERIFY(unmergeEvents.count() == 0);

    QObject *parent = gFindObject("MergeUnmergeObjects.parent1");
    QVERIFY(parent != 0);

    QList< QPair<QObject*,QObject*> > objectObjectPairs;
    for(int i=0; i<2; i++)
    {
        MergeEvent evt = mergeEvents.at(i);

        QVERIFY(evt.Parent == parent);
        QVERIFY(evt.Child == Object::List().at(i+1));
        objectObjectPairs.append(qMakePair(evt.Parent, evt.Child));
    }

    mergeEvents.clear();
    unmergeEvents.clear();
    component->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 2);

    for(int i=0; i<2; i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);

        QVERIFY(evt.Parent == objectObjectPairs.at(1-i).first);
        QVERIFY(evt.Child == objectObjectPairs.at(1-i).second);
    }
}

void ComponentEventsTest::testActivateDeactivateObject()
{
    QList<MergeEvent> activateEvents;
    QList<UnmergeEvent> deactivateEvents;

    GenericComponent *component = new GenericComponent;
    component->setContentFile(":/GenericComponent/ActivateDeactivateObjects.xml");
    component->ActivateEvents = &activateEvents;
    component->DeactivateEvents = &deactivateEvents;
    component->load();

    QVERIFY(activateEvents.count() == 2);
    QVERIFY(deactivateEvents.count() == 0);

    QObject *parent = gFindObject("ActivateDeactivateObjects.parent1");
    QVERIFY(parent != 0);

    QList< QPair<QObject*,QObject*> > objectObjectPairs;

    for(int i=0; i<2; i++)
    {
        MergeEvent evt = activateEvents.at(i);

        QVERIFY(evt.Parent == parent);
        QVERIFY(evt.Child == Object::List().at(i+1) );
        objectObjectPairs.append(qMakePair(evt.Parent, evt.Child));
    }

    activateEvents.clear();
    deactivateEvents.clear();
    component->unload();

    QVERIFY(activateEvents.count() == 0);
    QVERIFY(deactivateEvents.count() == 2);

    for(int i=0; i<2; i++)
    {
        UnmergeEvent evt = deactivateEvents.at(i);

        QVERIFY(evt.Parent == objectObjectPairs.at(i).first);
        QVERIFY(evt.Child == objectObjectPairs.at(i).second);
    }
}

QString ComponentEventsTest::logFileContents(bool deleteFile) const
{
    QString retString;
    {
        QFile file( GCF::Log::instance()->logFileName() );
        file.open( QFile::ReadOnly );
        retString = file.readAll();
    }

    if(deleteFile)
        QFile::remove( GCF::Log::instance()->logFileName() );
    return retString;
}

int main(int argc, char *argv[])
{
    GCF::Application app(argc, argv);
    ComponentEventsTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_ComponentEventsTest.moc"
