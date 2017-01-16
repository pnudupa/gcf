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
#include <QPointer>

#include <GCF3/Version>
#include <GCF3/Component>
#include <GCF3/Application>

class LoadComponentTest : public QObject
{
    Q_OBJECT
    
public:
    LoadComponentTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testLoadComponent();
    void testLoadMultipleInstances();

    void testLoadHigherVersionComponent();
    void testLoadLowerVersionComponent();
    void testLoadInvalidComponent();
    void testLoadNonGCFLibrary();

    void testProcessArguments();

    void testLoadComponentWithProperties();

private:
    QString logFileContents(bool deleteFile=false) const;
};

LoadComponentTest::LoadComponentTest()
{
}

void LoadComponentTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void LoadComponentTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void LoadComponentTest::cleanup()
{
    gApp->unloadAllComponents();
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void LoadComponentTest::testLoadComponent()
{
    QVERIFY(gApp->components().count() == 0);

    QPointer<GCF::Component> component = gApp->loadComponent("Components/SimpleComponent");
    QVERIFY(component.data() != 0);
    QVERIFY(component->name() == "SimpleComponent");
    QVERIFY(gApp->components().count() == 1);
    component->unload();
    QVERIFY(component.data() == 0);
    QVERIFY(gApp->components().count() == 0);
}

void LoadComponentTest::testLoadMultipleInstances()
{
    GCF::ObjectList components;

    for(int i=0; i<20; i++)
    {
        GCF::Component *comp = gApp->instantiateComponent("Components/SimpleComponent");
        QVERIFY(comp->isLoaded() == false);
        QVERIFY(comp->isActive() == false);

        comp->setProperty("name", QString("SimpleComponent%1").arg(i+1));
        comp->load();

        QVERIFY(comp->isLoaded() == true);
        QVERIFY(comp->isActive() == true);
        QVERIFY(gApp->components().count() == i+1);

        components.add(comp);
    }

    for(int i=0; i<20; i++)
    {
        QString name = QString("SimpleComponent%1").arg(i+1);
        QString path = QString("Application.%1").arg(name);
        QVERIFY(gApp->objectTree()->object(path) == components.at(i));
    }

    while(components.count())
    {
        GCF::Component *comp = (GCF::Component*)components.last();
        comp->unload();

        QVERIFY(gApp->objectTree()->rootNode()->children().count() == components.count());
        QVERIFY(gApp->components().count() == components.count());
    }
}

void LoadComponentTest::testLoadHigherVersionComponent()
{
    QString lib = GCF::findLibrary("Components/HigherVersionComponent");
    QVERIFY( QFile::exists(lib) );

    GCF::Component *component = 0;

    // Load using GCF::Application::loadComponent()
    component = gApp->loadComponent("Components/HigherVersionComponent");
    QVERIFY(component == 0);
    QString errLog = QString("Component is built for a higher version of GCF3 than the one used by this application. "
                             "This application uses GCF %1, whereas the component uses %2.")
                        .arg(GCF::version()).arg("4.0.0");
    QVERIFY(this->logFileContents().contains(errLog));

    // Load using GCF::Application::instantiateComponent()
    component = gApp->instantiateComponent("Components/HigherVersionComponent");
    QVERIFY(component == 0);
    QVERIFY(this->logFileContents().contains(errLog));

    if(component)
        component->unload();
}

void LoadComponentTest::testLoadLowerVersionComponent()
{
    QString lib = GCF::findLibrary("Components/LowerVersionComponent");
    QVERIFY( QFile::exists(lib) );

    GCF::Component *component = 0;

    // Load using GCF::Application::loadComponent()
    component = gApp->loadComponent("Components/LowerVersionComponent");
    QVERIFY(component == 0);
    QString errLog = QString("Component is built for a lower version of GCF3 than the one used by this application. "
                             "This application uses GCF %1, whereas the component uses %2.")
                        .arg(GCF::version()).arg("2.0.0");
    QVERIFY(this->logFileContents().contains(errLog));

    // Load using GCF::Application::instantiateComponent()
    component = gApp->instantiateComponent("Components/LowerVersionComponent");
    QVERIFY(component == 0);
    QVERIFY(this->logFileContents().contains(errLog));

    if(component)
        component->unload();
}

void LoadComponentTest::testLoadInvalidComponent()
{
    QString lib = GCF::findLibrary("Components/InvalidComponent");
    QVERIFY( QFile::exists(lib) );

    GCF::Component *component = 0;

    // Load using GCF::Application::loadComponent()
    component = gApp->loadComponent("Components/InvalidComponent");
    QVERIFY(component == 0);
    QString errLog = QString("Component library did not return any component object.");
    QVERIFY(this->logFileContents().contains(errLog));

    // Load using GCF::Application::instantiateComponent()
    component = gApp->instantiateComponent("Components/InvalidComponent");
    QVERIFY(component == 0);
    QVERIFY(this->logFileContents().contains(errLog));

    if(component)
        component->unload();
}

void LoadComponentTest::testLoadNonGCFLibrary()
{
#ifdef Q_OS_WIN32
#if QT_VERSION >= 0x050000
    QString library = "sqldrivers/qsqlite";
#else
    QString library = "sqldrivers/qsqlite4";
#endif
#else
    QString library = "sqldrivers/qsqlite";
#endif

    QVERIFY( QFile::exists( GCF::findLibrary(library) ) );

    GCF::Component *component = 0;

    // Load using GCF::Application::loadComponent()
    component = gApp->loadComponent(library);
    QVERIFY(component == 0);
    QString errLog = QString("Library doesn't contain a GCF3 component");
    QVERIFY(this->logFileContents().contains(errLog));

    // Load using GCF::Application::instantiateComponent()
    component = gApp->instantiateComponent(library);
    QVERIFY(component == 0);
    QVERIFY(this->logFileContents().contains(errLog));

    if(component)
        component->unload();
}

void LoadComponentTest::testProcessArguments()
{
#ifdef Q_OS_WIN32
#if QT_VERSION >= 0x050000
    QString library = "sqldrivers/qsqlite";
#else
    QString library = "sqldrivers/qsqlite4";
#endif
#else
    QString library = "sqldrivers/qsqlite";
#endif

    QStringList args;
#ifdef Q_OS_WIN32
    args << "--loadComponents:Components/SimpleComponent,Components/HigherVersionComponent,Components/LowerVersionComponent,Components/InvalidComponent," + library;
#else
    args << "--loadComponents:Components/SimpleComponent,Components/HigherVersionComponent,Components/LowerVersionComponent,Components/InvalidComponent," + library;
#endif

    gApp->processArguments(args);

    QVERIFY(gApp->components().count() == 1);

    QString logs = this->logFileContents();

    QString errLog = QString("Library doesn't contain a GCF3 component");
    QVERIFY(logs.contains(errLog));

    errLog = QString("Component library did not return any component object.");
    QVERIFY(logs.contains(errLog));

    errLog = QString("Component is built for a lower version of GCF3 than the one used by this application. "
                             "This application uses GCF %1, whereas the component uses %2.")
                        .arg(GCF::version()).arg("2.0.0");
    QVERIFY(logs.contains(errLog));

    errLog = QString("Component is built for a higher version of GCF3 than the one used by this application. "
                             "This application uses GCF %1, whereas the component uses %2.")
                        .arg(GCF::version()).arg("4.0.0");
    QVERIFY(logs.contains(errLog));
}

void LoadComponentTest::testLoadComponentWithProperties()
{
    QList< QPair<QByteArray,QVariant> > properties;
    properties << qMakePair<QByteArray,QVariant>("name", "ChangedSimpleComponent");
    properties << qMakePair<QByteArray,QVariant>("parameter1", 20);
    properties << qMakePair<QByteArray,QVariant>("property2", 456.123);
    properties << qMakePair<QByteArray,QVariant>("date", QDate::currentDate());

    GCF::Component *component = gApp->loadComponent("Components/SimpleComponent", properties);

    QVariantMap map;
    QMetaObject::invokeMethod(component, "propertiesBeforeInitialize", Qt::DirectConnection,
                              Q_RETURN_ARG(QVariantMap,map));

    QVERIFY(map.value("name").toString() == "ChangedSimpleComponent");
    QVERIFY(map.value("parameter1").toInt() == 20);
    QVERIFY(map.value("property2").toDouble() == 456.123);
    QVERIFY(map.value("date").toDate() == QDate::currentDate());
    QVERIFY(component->property("name").toString() == "ChangedSimpleComponent");
}

QString LoadComponentTest::logFileContents(bool deleteFile) const
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
    LoadComponentTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_LoadComponentTest.moc"
