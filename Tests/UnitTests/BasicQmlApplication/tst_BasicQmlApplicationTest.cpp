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
#include <QQmlEngine>
#include <QQmlContext>

#include <GCF3/QmlApplication>
#include <GCF3/QmlComponent>
#include <GCF3/Version>
#include <GCF3/Log>

class BasicQmlApplicationTest : public QObject
{
    Q_OBJECT
    
public:
    BasicQmlApplicationTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testgAppPointer();

    void testSettQmlEngine();

private:
    QString logFileContents(bool deleteFile=true) const;
};

BasicQmlApplicationTest::BasicQmlApplicationTest()
{
}

void BasicQmlApplicationTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void BasicQmlApplicationTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void BasicQmlApplicationTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void BasicQmlApplicationTest::testgAppPointer()
{
    QVERIFY( gApp->inherits("QApplication") == false );
    QVERIFY( gApp->inherits("QGuiApplication") == true );
}

void BasicQmlApplicationTest::testSettQmlEngine()
{
    gApp->setQmlEngine(0); // shouldnt crash
    QVERIFY(this->logFileContents().contains("Null QML Engine being set!"));

    QQmlEngine engine;
    gApp->setQmlEngine(&engine);
    QVERIFY(gApp->qmlEngine() == &engine);

    gApp->setQmlEngine(&engine);
    QVERIFY(this->logFileContents().contains("Cannot change QQmlEngine on GCF::QmlApplication, once it has been set."));

    gApp->setQmlEngine(0);
    QVERIFY(this->logFileContents().contains("Cannot change QQmlEngine on GCF::QmlApplication, once it has been set."));

    // Check if gApp and gcf objects are set on the qml engine
    QVERIFY(engine.rootContext()->contextProperty("gApp").value<QObject*>() == gApp);

    QObject *gcf = engine.rootContext()->contextProperty("gcf").value<QObject*>();
    QVERIFY(gcf != 0);
    QVERIFY(gcf->metaObject()->indexOfMethod("findObject(QString)") >= 0);
    QVERIFY(gcf->metaObject()->indexOfMethod("findImplementation(QString)") >= 0);
    QVERIFY(gcf->metaObject()->indexOfMethod("findImplementations(QString)") >= 0);
    QVERIFY(gcf->metaObject()->indexOfMethod("addQmlItem(QString,QObject*)") >= 0);
    QVERIFY(gcf->metaObject()->indexOfMethod("removeQmlItem(QString)") >= 0);
}

QString BasicQmlApplicationTest::logFileContents(bool deleteFile) const
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
    GCF::QmlApplication app(argc, argv);
    BasicQmlApplicationTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_BasicQmlApplicationTest.moc"
