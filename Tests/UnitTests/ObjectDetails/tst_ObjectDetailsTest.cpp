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

#include <GCF3/Application>

#include "GenericComponent.h"
#include "ConnectionTestComponent.h"

class ObjectDetailsTest : public QObject
{
    Q_OBJECT
    
public:
    ObjectDetailsTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testLoadProperties();
    void testLoadConnections();

private:
    QString logFileContents(bool deleteFile=true) const;
};

ObjectDetailsTest::ObjectDetailsTest()
{
}

void ObjectDetailsTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectDetailsTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectDetailsTest::cleanup()
{
    this->logFileContents();
}

void ObjectDetailsTest::testLoadProperties()
{
    GenericComponent *comp = new GenericComponent;
    comp->setContentFile(":/Properties.xml");
    comp->load();

    QString log = this->logFileContents();

    QObject *object1 = gFindObject("Application.Properties.Object1");
    QVERIFY(object1 != 0);
    QVERIFY(object1->dynamicPropertyNames().contains("property1"));
    QVERIFY(object1->dynamicPropertyNames().contains("property2"));
    QVERIFY(object1->property("property1").toString() == "object1_value1");
    QVERIFY(object1->property("property2").toString() == "object1_value2");

    QObject *object2 = gFindObject("Application.Properties.Object2");
    QVERIFY(object2 != 0);
    QVERIFY(object2->dynamicPropertyNames().contains("property1"));
    QVERIFY(object2->dynamicPropertyNames().contains("property2"));
    QVERIFY(object2->dynamicPropertyNames().contains("property3") == false);
    QVERIFY(object2->property("property1").toString() == "object1_value1");
    QVERIFY(object2->property("property2").toString() == "object2_value2");

    QVERIFY(log.contains("Cannot evaluate value of Object4::property1"));

    comp->unload();
}

void ObjectDetailsTest::testLoadConnections()
{
    ConnectionTestComponent *comp = new ConnectionTestComponent;
    comp->load();

    QString log = this->logFileContents();

    Sender *sender1 = gFindObject<Sender>("Application.Connections.sender1");
    Sender *sender2 = gFindObject<Sender>("Application.Connections.sender2");
    Receiver *receiver1 = gFindObject<Receiver>("Application.Connections.receiver1");
    Receiver *receiver2 = gFindObject<Receiver>("Application.Connections.receiver2");

    QVERIFY(sender1 != 0);
    QVERIFY(sender2 != 0);
    QVERIFY(receiver1 != 0);
    QVERIFY(receiver2 != 0);

    // Verify that the connections were indeed established.
    sender1->sendEmptySignal();
    QVERIFY(receiver1->lastSlot() == "emptySlot()");
    QVERIFY(receiver2->lastSlot().isEmpty());

    sender2->sendEmptySignal();
    QVERIFY(receiver2->lastSlot() == "emptySlot()");
    QVERIFY(receiver1->lastSlot().isEmpty());

    sender1->sendIntegerSignal(20);
    QVERIFY(receiver1->lastSlot() == "integerSlot(20)");
    QVERIFY(receiver2->lastSlot().isEmpty());

    sender2->sendIntegerSignal(20);
    QVERIFY(receiver2->lastSlot() == "integerSlot(20)");
    QVERIFY(receiver1->lastSlot().isEmpty());

    sender1->sendStringSignal("hello world");
    QVERIFY(receiver1->lastSlot() == "stringSlot(hello world)");
    QVERIFY(receiver2->lastSlot().isEmpty());

    sender2->sendStringSignal("hello world");
    QVERIFY(receiver2->lastSlot() == "stringSlot(hello world)");
    QVERIFY(receiver1->lastSlot().isEmpty());

    // Verify that error-log messages were generated
    QVERIFY(log.contains("Cannot find signal info from unknownSender::unknownSignal()"));
    QVERIFY(log.contains("Cannot find member info from unknownSlot()"));
    QVERIFY(log.contains("Cannot find signal info from unknownSignal()"));
    QVERIFY(log.contains("Cannot find member info from unknownReceiver::unknownSlot()"));

    comp->unload();
}

QString ObjectDetailsTest::logFileContents(bool deleteFile) const
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
    ObjectDetailsTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_ObjectDetailsTest.moc"
