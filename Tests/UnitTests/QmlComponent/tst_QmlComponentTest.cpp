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
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QApplication>
#include <QNetworkAccessManager>

#include <GCF3/QmlApplication>
#include <GCF3/QmlComponent>
#include <GCF3/Version>
#include <GCF3/Log>

#include "GenericQmlComponent.h"

class QmlComponentTest : public QObject
{
    Q_OBJECT

public:
    QmlComponentTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testgAppPointer();
    void testLoadUnloadQmlComponent();
    void testQmlContextProperties();
    void testActivationAndDeactivation();
    void testAddQml();

private:
    QString logFileContents(bool deleteFile=true) const;
    QVariant qmlProperty(QObject *qmlObject, const QString &propName) const;

private:
    QQuickView *m_quickView;
};

QmlComponentTest::QmlComponentTest() : m_quickView(0)
{
}

void QmlComponentTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void QmlComponentTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void QmlComponentTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void QmlComponentTest::testgAppPointer()
{
    QVERIFY( gApp->inherits("QApplication") == true );
    QVERIFY( gApp->inherits("QGuiApplication") == true );
    QVERIFY( gApp->qmlEngine() == 0 );

    m_quickView = new QQuickView;
    connect(gApp, SIGNAL(aboutToQuit()), m_quickView, SLOT(close()));
    gApp->setQmlEngine(m_quickView->engine());
    QVERIFY( gApp->qmlEngine() == m_quickView->engine() );
}

void QmlComponentTest::testLoadUnloadQmlComponent()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    GenericQmlComponent *qmlComp = new GenericQmlComponent;
    qmlComp->setContentFile(":/QmlComponent1.xml");
    qmlComp->setName("QmlComponent1");
    qmlComp->LoadEvents = &loadEvents;
    qmlComp->UnloadEvents = &unloadEvents;
    qmlComp->load();

    QVERIFY(loadEvents.count() == 5);
    QVERIFY(unloadEvents.count() == 0);

    QVERIFY(loadEvents.at(0).Name == "object2");
    QVERIFY(loadEvents.at(0).Url == QUrl::fromLocalFile(QDir::currentPath() + "/TestQmlComponent2.qml"));
    QVERIFY(loadEvents.at(0).QmlObject != 0);
    QVERIFY(this->qmlProperty(loadEvents.at(0).QmlObject, "indicator").toString() == "Indicator2");

    QString expectedPath = QDir::currentPath() + "/../../../Tests/UnitTests/QmlComponent/TestQmlComponent3.qml";
    expectedPath = QFileInfo(expectedPath).absoluteFilePath();

    QVERIFY(loadEvents.at(1).Name == "object3");
    QVERIFY(loadEvents.at(1).Url == QUrl::fromLocalFile(expectedPath));
    QVERIFY(loadEvents.at(1).QmlObject != 0);
    QVERIFY(this->qmlProperty(loadEvents.at(1).QmlObject, "indicator").toString() == "Indicator3");

    QVERIFY(loadEvents.at(2).Name == "object4");
    QVERIFY(loadEvents.at(2).Url.toString() == "http://code.vcreatelogic.com/TestQmlComponent4.qml");
    // QVERIFY(loadEvents.at(2).QmlObject != 0);

    QVERIFY(loadEvents.at(3).Name == "object5");
    QVERIFY(loadEvents.at(3).Url.toString() == "qrc:///TestQmlComponent5.qml");
    QVERIFY(loadEvents.at(3).QmlObject != 0);
    QVERIFY(this->qmlProperty(loadEvents.at(3).QmlObject, "indicator").toString() == "Indicator5");

    QVERIFY(loadEvents.at(4).Name == "object6");
    QVERIFY(loadEvents.at(4).Url.toString() == "qrc:///TestQmlComponent25.qml");
    QVERIFY(loadEvents.at(4).QmlObject == 0);

    QString log = this->logFileContents();
    QVERIFY(log.contains("QML file TestQmlComponent1.qml not found"));
    QVERIFY(log.contains("Error while loading QML Object TestQmlComponent25.qml"));
    QVERIFY(log.contains("QML file ../../Tests/Sample.qml not found"));
    QVERIFY(log.contains("QML file C:/Sample.qml not found"));
    QVERIFY(log.contains("Cannot recognize URL scheme in jaggesh:///wow.qml. Only http, ftp, "
                         "file and qrc schemes are supported"));

    loadEvents.clear();
    unloadEvents.clear();

    GCF::ObjectTreeNode *qmlCompNode = gApp->objectTree()->node(qmlComp);
    QVERIFY(qmlCompNode != 0);
    QList<GCF::ObjectTreeNode*> qmlCompObjNodes = qmlCompNode->children();
    QObjectList qmlCompObjList;
    Q_FOREACH(GCF::ObjectTreeNode *qmlCompObjNode, qmlCompObjNodes)
        if(qmlCompObjNode->object())
            qmlCompObjList.prepend(qmlCompObjNode->object());

    qmlComp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == qmlCompObjList.count());
    for(int i=0; i<unloadEvents.count(); i++)
        QVERIFY(unloadEvents.at(i).QmlObject == qmlCompObjList.at(i));
}

void QmlComponentTest::testQmlContextProperties()
{
    GenericQmlComponent *qmlComp = new GenericQmlComponent;
    qmlComp->setContentFile(":/QmlComponent2.xml");
    qmlComp->setName("QmlComponent2");
    qmlComp->load();

    QObject *qmlObj = gFindObject("Application.QmlComponent2.qmlObj1");
    QVERIFY(qmlObj != 0);

    QVERIFY(qmlComp->String == "active");
    QVERIFY(this->qmlProperty(qmlObj, "active").value<bool>() == true);
    QVERIFY(this->qmlProperty(qmlObj, "indicator").value<QString>() == "Indicator1");
    QVERIFY(this->qmlProperty(qmlObj, "a").value<QObject*>() == gFindObject("Application.QmlComponent2.a"));
    QVERIFY(this->qmlProperty(qmlObj, "b").value<QObject*>() == gFindObject("Application.QmlComponent2.b"));
    QVERIFY(this->qmlProperty(qmlObj, "c").value<QObject*>() == gFindObject("Application.QmlComponent2.c"));
    QVERIFY(this->qmlProperty(qmlObj, "d").value<QObject*>() == gFindObject("Application.QmlComponent2.d"));

    qmlComp->unload();
}

void QmlComponentTest::testActivationAndDeactivation()
{
    GenericQmlComponent *qmlComp = new GenericQmlComponent;
    qmlComp->setContentFile(":/QmlComponent2.xml");
    qmlComp->setName("QmlComponent2");
    qmlComp->load();

    QSignalSpy spy(qmlComp, SIGNAL(activeChanged(bool)));

    qmlComp->deactivate();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().first().toBool() == false);
    QVERIFY(qmlComp->isActive() == false);
    QVERIFY(qmlComp->String == "inactive");
    spy.clear();

    qmlComp->activate();
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().first().toBool() == true);
    QVERIFY(qmlComp->isActive() == true);
    QVERIFY(qmlComp->String == "active");
    spy.clear();

    qmlComp->unload();
}

void QmlComponentTest::testAddQml()
{
    GenericQmlComponent *qmlComp = new GenericQmlComponent;
    qmlComp->setName("QmlComponent2");

    QString log;

    QPointer<QObject> qmlObj = qmlComp->addQml(QUrl("qrc:///TestQmlComponent1.qml"));
    QVERIFY(qmlObj.data() == 0);
    log = this->logFileContents();
    QVERIFY(log.contains("Cannot load QML object from \"qrc:///TestQmlComponent1.qml\" before the component is loaded"));
    QVERIFY(log.contains("QML object was not added"));

    qmlComp->load();

    qmlObj = qmlComp->addQml(QUrl("qrc:///TestQmlComponent1.qml"));
    QVERIFY(qmlObj.data() != 0);
    QVERIFY(qmlObj->objectName() == "TestQmlComponent1");
    QVERIFY(qmlObj.data() == gFindObject("Application.QmlComponent2.TestQmlComponent1"));

    QVERIFY(qmlComp->String == "active");
    QVERIFY(this->qmlProperty(qmlObj, "active").value<bool>() == true);
    QVERIFY(this->qmlProperty(qmlObj, "indicator").value<QString>() == "Indicator1");
    QVERIFY(this->qmlProperty(qmlObj, "a") == QVariant());
    QVERIFY(this->qmlProperty(qmlObj, "b") == QVariant());
    QVERIFY(this->qmlProperty(qmlObj, "c") == QVariant());
    QVERIFY(this->qmlProperty(qmlObj, "d") == QVariant());

    qmlComp->deactivate();
    QVERIFY(qmlComp->String == "inactive");

    qmlComp->activate();
    QVERIFY(qmlComp->String == "active");

    log = this->logFileContents();
    QVERIFY(log.contains("QML object successfully added"));

    QPointer<QObject> qmlObj2 = qmlComp->addQml("Custom", QUrl("qrc:///TestQmlComponent1.qml"));
    QVERIFY(qmlObj2.data() != 0);
    QVERIFY(qmlObj2->objectName() == "Custom");
    QVERIFY(qmlObj2.data() == gFindObject("Application.QmlComponent2.Custom"));
    log = this->logFileContents();
    QVERIFY(log.contains("QML object successfully added"));

    QVERIFY(qmlComp->addQml("Custom", QUrl("qrc:///TestQmlComponent1.qml")) == 0);
    log = this->logFileContents();
    QVERIFY(log.contains("QML object was not added"));

    QVERIFY(qmlComp->addQml("Unknown", QUrl()) == 0);
    log = this->logFileContents();
    QVERIFY(log.contains("QML object was not added"));

    QVERIFY(qmlComp->addQml("Unknown", QUrl("qrc:///NonExistingFile.qml")) == 0);
    log = this->logFileContents();
    QVERIFY(log.contains("QML object was not added"));

    qmlComp->unload();
    QVERIFY(qmlObj.data() == 0);
    QVERIFY(qmlObj2.data() == 0);
}

QString QmlComponentTest::logFileContents(bool deleteFile) const
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

QVariant QmlComponentTest::qmlProperty(QObject *qmlObject, const QString &propName) const
{
    QQmlProperty property(qmlObject, propName);
    if(property.isValid())
    {
        QVariant value = property.read();
        return value;
    }

    return QVariant();
}

int main(int argc, char *argv[])
{
    GCF::QmlApplication app(argc, argv);
    QmlComponentTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_QmlComponentTest.moc"
