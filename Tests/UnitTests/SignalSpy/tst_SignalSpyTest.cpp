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
#include <QMetaType>

#include <GCF3/SignalSpy>
#include <GCF3/Log>
#include <GCF3/Version>

struct MyType
{
    MyType() : integer(0), real(0) { }

    int integer;
    double real;
    QString string;
};
Q_DECLARE_METATYPE(MyType)

struct MyUnsupportedType
{
    int integer;
    double real;
    QString string;
};

class SignalSpyTest;
class SignalObject : public QObject
{
    Q_OBJECT

public:
    SignalObject(QObject *parent=0) : QObject(parent) { }

signals:
    friend class SignalSpyTest;
    void noParams();
    void integer(int val);
    void boolean(bool val);
    void real(double val);
    void string(const QString &val);
    void stringList(const QStringList &val);
    void variant(const QVariant &val);
    void variantList(const QVariantList &val);
    void variantMap(const QVariantMap &val);
    void byteArray(const QByteArray &val);
    void allParams(int intVal, bool boolVal, double doubleVal,
                const QString &strVal, const QStringList &strListVal,
                const QVariant &varVal, const QVariantList &varList,
                const QVariantMap &varMap, const QByteArray &byteArr);
    void myType(const MyType &val);
    void myUnsupportedType(const MyUnsupportedType &val);
};

class SignalSpyTest : public QObject
{
    Q_OBJECT
    
public:
    SignalSpyTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_noParams();
    void test_integer();
    void test_boolean();
    void test_real();
    void test_string();
    void test_stringList();
    void test_variant();
    void test_variantList();
    void test_variantMap();
    void test_byteArray();
    void test_allParams();
    void test_myType();

    void test_unsupportedType();
    void test_invalidSignal();
    void test_invalidObject();

    void test_wait();
};

SignalSpyTest::SignalSpyTest()
{
}

void SignalSpyTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void SignalSpyTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void SignalSpyTest::cleanup()
{
    GCF::Log::instance()->setHandler(0);
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void SignalSpyTest::test_noParams()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(noParams()));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "noParams()");

    object.noParams();

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 0);
}

void SignalSpyTest::test_integer()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(integer(int)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "integer(int)");

    object.integer(10);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(10));
}

void SignalSpyTest::test_boolean()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(boolean(bool)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "boolean(bool)");

    object.boolean(false);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(false));
}

void SignalSpyTest::test_real()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(real(double)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "real(double)");

    object.real(456.123);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(456.123));
}

void SignalSpyTest::test_string()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(string(QString)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "string(QString)");

    object.string("hello world");

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(QString("hello world")));
}

void SignalSpyTest::test_stringList()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(stringList(QStringList)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "stringList(QStringList)");

    QStringList list = QStringList() << "A" << "B" << "C";
    object.stringList(list);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(list));
}

void SignalSpyTest::test_variant()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(variant(QVariant)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "variant(QVariant)");

    QVariant dateVal = QVariant( QDate::currentDate() );
    object.variant(dateVal);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == dateVal);
}

void SignalSpyTest::test_variantList()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(variantList(QVariantList)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "variantList(QVariantList)");

    QVariantList varList;
    varList.append( QDate::currentDate() );
    varList.append( QString("this is a simple string") );
    varList.append( true );
    varList.append( 69123.123 );
    object.variantList(varList);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(varList));
}

void SignalSpyTest::test_variantMap()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(variantMap(QVariantMap)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "variantMap(QVariantMap)");

    QVariantMap varMap;
    varMap["january"] = 31;
    varMap["february"] = 28;
    varMap["march"] = 31;
    varMap["april"] = 30;
    varMap["may"] = 31;
    object.variantMap(varMap);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(varMap));
}

void SignalSpyTest::test_byteArray()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(byteArray(QByteArray)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "byteArray(QByteArray)");

    QByteArray bytes(4096, 'A');
    object.byteArray(bytes);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);
    QVERIFY(args.first() == QVariant(bytes));
}

void SignalSpyTest::test_allParams()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(allParams(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "allParams(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)");

    QVariantList inputArgs;
    inputArgs.append(10);
    inputArgs.append(false);
    inputArgs.append(123.34);
    inputArgs.append(QString("string value"));
    inputArgs.append(QStringList() << "a" << "b" << "C");
    inputArgs.append(QVariant(QDate::currentDate()));

    QVariantList varList;
    varList.append( QDate::currentDate() );
    varList.append( QString("this is a simple string") );
    varList.append( true );
    varList.append( 69123.123 );
    inputArgs.append( QVariant(varList) );

    QVariantMap varMap;
    varMap["january"] = 31;
    varMap["february"] = 28;
    varMap["march"] = 31;
    varMap["april"] = 30;
    varMap["may"] = 31;

    inputArgs.append( QVariant(varMap) );

    QByteArray bytes(4096, 'A');
    inputArgs.append(bytes);

    object.allParams( inputArgs.at(0).toInt(), inputArgs.at(1).toBool(), inputArgs.at(2).toDouble(),
                      inputArgs.at(3).toString(), inputArgs.at(4).toStringList(),
                      inputArgs.at(5), inputArgs.at(6).toList(), inputArgs.at(7).toMap(), inputArgs.at(8).toByteArray() );

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList signalArgs = qspy.first().first().toList();
    QVERIFY(signalArgs.count() == 9);
    QVERIFY(signalArgs == inputArgs);
}

void SignalSpyTest::test_myType()
{
#if QT_VERSION < 0x050000
    qRegisterMetaType<MyType>("MyType");
#endif

    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(myType(MyType)));
    QSignalSpy qspy(&spy, SIGNAL(caughtSignal(QVariantList)));

    QVERIFY(spy.isValid() == true);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "myType(MyType)");

    MyType inputType;
    inputType.integer = 20;
    inputType.real = 456.789;
    inputType.string = "hello world";
    object.myType(inputType);

    QVERIFY(qspy.count() == 1);
    QVERIFY(qspy.first().count() == 1);

    QVariantList args = qspy.first().first().toList();
    QVERIFY(args.count() == 1);

    MyType signalType = args.at(0).value<MyType>();
    QVERIFY(signalType.integer == inputType.integer);
    QVERIFY(signalType.real == inputType.real);
    QVERIFY(signalType.string == inputType.string);
}

void SignalSpyTest::test_unsupportedType()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(myUnsupportedType(MyUnsupportedType)));

    QVERIFY(spy.isValid() == false);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == "myUnsupportedType(MyUnsupportedType)");
}

void SignalSpyTest::test_invalidSignal()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(nonExistingSignal(QString,QByteArray)));

    QVERIFY(spy.isValid() == false);
    QVERIFY(spy.sender() == &object);
    QVERIFY(spy.signal() == QByteArray());
}

void SignalSpyTest::test_invalidObject()
{
    GCF::SignalSpy spy(0, 0);

    QVERIFY(spy.isValid() == false);
    QVERIFY(spy.sender() == 0);
    QVERIFY(spy.signal() == QByteArray());
}

void SignalSpyTest::test_wait()
{
    SignalObject object;
    GCF::SignalSpy spy(&object, SIGNAL(noParams()));

    QTimer::singleShot(1000, &object, SIGNAL(noParams()));
    QVERIFY(spy.wait(500) == false);
    QVERIFY(spy.count() == 0);
    QVERIFY(spy.wait(600) == true);
    QVERIFY(spy.count() == 1);
    spy.clear();

    QTimer::singleShot(500, &object, SIGNAL(noParams()));
    QVERIFY(spy.wait(1000) == true);
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(spy.wait(1000) == false);
}

QTEST_MAIN(SignalSpyTest)

#include "tst_SignalSpyTest.moc"
