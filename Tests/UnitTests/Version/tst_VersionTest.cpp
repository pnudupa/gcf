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
#include <GCF3/Version>

class VersionTest : public QObject
{
    Q_OBJECT
    
public:
    VersionTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testConstructor1_data();
    void testConstructor1();

    void testConstructor2();

    void testConstructor3_data();
    void testConstructor3();

    void testLessThanOperator_data();
    void testLessThanOperator();

    void testGreaterThanOperator_data();
    void testGreaterThanOperator();

    void testEqualsToOperator_data();
    void testEqualsToOperator();

    void testToString_data();
    void testToString();
};

VersionTest::VersionTest()
{
}

void VersionTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void VersionTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void VersionTest::testConstructor1_data()
{
    QTest::addColumn<GCF::Version>("version");
    QTest::addColumn<int>("major");
    QTest::addColumn<int>("minor");
    QTest::addColumn<int>("revision");

    QTest::newRow("Default constructor") << GCF::Version() << 0 << 0 << 0;
    QTest::newRow("Major only") << GCF::Version(1) << 1 << 0 << 0;
    QTest::newRow("Major and Minor only") << GCF::Version(1, 2) << 1 << 2 << 0;
    QTest::newRow("Major, Minor and Revision") << GCF::Version(1, 2, 3) << 1 << 2 << 3;
    QTest::newRow("Major, Minor and Revision maxed") << GCF::Version(0xffff, 0xffff, 0xffff) << 0xffff << 0xffff << 0xffff;
}

void VersionTest::testConstructor1()
{
    QFETCH(GCF::Version, version);
    QFETCH(int, major);
    QFETCH(int, minor);
    QFETCH(int, revision);

    QCOMPARE(version.major(), (quint16)major);
    QCOMPARE(version.minor(), (quint16)minor);
    QCOMPARE(version.revision(), (quint16)revision);
}

void VersionTest::testConstructor2()
{
    GCF::Version version1(1, 2, 3);
    GCF::Version version2;

    QVERIFY(version2.isValid() == false);

    version2 = version1;
    QVERIFY(version1.major() == version2.major());
    QVERIFY(version1.minor() == version2.minor());
    QVERIFY(version1.revision() == version2.revision());
}

void VersionTest::testConstructor3_data()
{
    QTest::addColumn<GCF::Version>("version");
    QTest::addColumn<int>("major");
    QTest::addColumn<int>("minor");
    QTest::addColumn<int>("revision");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<bool>("baseVersion");

    QTest::newRow("Invalid") << GCF::Version("") << 0 << 0 << 0 << false << false;
    QTest::newRow("0") << GCF::Version("0") << 0 << 0 << 0 << false << false;
    QTest::newRow("0.0") << GCF::Version("0.0") << 0 << 0 << 0 << false << false;
    QTest::newRow("0.0.0") << GCF::Version("0.0.0") << 0 << 0 << 0 << false << false;
    QTest::newRow("1") << GCF::Version("1") << 1 << 0 << 0 << true << true;
    QTest::newRow("1.1") << GCF::Version("1.1") << 1 << 1 << 0 << true << false;
    QTest::newRow("1.1.1") << GCF::Version("1.1.1") << 1 << 1 << 1 << true << false;
    QTest::newRow("11.3.8") << GCF::Version("11.3.8") << 11 << 3 << 8 << true << false;
    QTest::newRow("-11.3.8") << GCF::Version("-11.3.8") << 0 << 0 << 0 << false << false;
    QTest::newRow("INVALID VERSION STRING") << GCF::Version("INVALID VERSION STRING") << 0 << 0 << 0 << false << false;
    QTest::newRow("65536.65537.65538") << GCF::Version("65536.65537.65538") << 0 << 0 << 0 << false << false;
    QTest::newRow("65535.65535.65535") << GCF::Version("65535.65535.65535") << 65535 << 65535 << 65535 << true << false;
    QTest::newRow("11.A.8") << GCF::Version("11.A.8") << 0 << 0 << 0 << false << false;
    QTest::newRow("A.3.8") << GCF::Version("A.3.8") << 0 << 0 << 0 << false << false;
    QTest::newRow("11.3.a") << GCF::Version("11.3.a") << 0 << 0 << 0 << false << false;
    QTest::newRow("x.y.z") << GCF::Version("x.y.z") << 0 << 0 << 0 << false << false;
    QTest::newRow("1..3") << GCF::Version("1..3") << 0 << 0 << 0 << false << false;
    QTest::newRow("1.2.3.4.5.6.") << GCF::Version("1.2.3.4.5.6.") << 0 << 0 << 0 << false << false;
}

void VersionTest::testConstructor3()
{
    QFETCH(GCF::Version, version);
    QFETCH(int, major);
    QFETCH(int, minor);
    QFETCH(int, revision);
    QFETCH(bool, valid);
    QFETCH(bool, baseVersion);

    QCOMPARE(version.major(), (quint16)major);
    QCOMPARE(version.minor(), (quint16)minor);
    QCOMPARE(version.revision(), (quint16)revision);
    QCOMPARE(version.isValid(), valid);
    QCOMPARE(version.isBaseVersion(), baseVersion);
}

void VersionTest::testLessThanOperator_data()
{
    QTest::addColumn<GCF::Version>("v1");
    QTest::addColumn<GCF::Version>("v2");
    QTest::addColumn<bool>("v1LessThanV2");

    QTest::newRow("Invalid < Invalid") << GCF::Version() << GCF::Version() << false;
    QTest::newRow("1 < Invalid") << GCF::Version(1) << GCF::Version() << false;
    QTest::newRow("Invalid < 1") << GCF::Version() << GCF::Version(1) << true;
    QTest::newRow("1.2.0 < 1.1.0") << GCF::Version("1.2.0") << GCF::Version("1.1.0") << false;
    QTest::newRow("1.1.0 < 1.2.0") << GCF::Version(1,1,0) << GCF::Version(1,2,0) << true;
    QTest::newRow("1.2.0 < 1.2.1") << GCF::Version("1.2.0") << GCF::Version("1.2.1") << true;
    QTest::newRow("1.2.1 < 1.2.0") << GCF::Version(1,2,1) << GCF::Version("1.2.0") << false;
    QTest::newRow("1.2.3 < 1.2.3") << GCF::Version(1,2,3) << GCF::Version("1.2.3") << false;
}

void VersionTest::testLessThanOperator()
{
    QFETCH(GCF::Version, v1);
    QFETCH(GCF::Version, v2);
    QFETCH(bool, v1LessThanV2);

    QCOMPARE(v1 < v2, v1LessThanV2);
}

void VersionTest::testGreaterThanOperator_data()
{
    QTest::addColumn<GCF::Version>("v1");
    QTest::addColumn<GCF::Version>("v2");
    QTest::addColumn<bool>("v1GreaterThanV2");

    QTest::newRow("Invalid > Invalid") << GCF::Version() << GCF::Version() << false;
    QTest::newRow("1 > Invalid") << GCF::Version(1) << GCF::Version() << true;
    QTest::newRow("Invalid > 1") << GCF::Version() << GCF::Version(1) << false;
    QTest::newRow("1.2.0 > 1.1.0") << GCF::Version("1.2.0") << GCF::Version("1.1.0") << true;
    QTest::newRow("1.1.0 > 1.2.0") << GCF::Version(1,1,0) << GCF::Version(1,2,0) << false;
    QTest::newRow("1.2.0 > 1.2.1") << GCF::Version("1.2.0") << GCF::Version("1.2.1") << false;
    QTest::newRow("1.2.1 > 1.2.0") << GCF::Version(1,2,1) << GCF::Version("1.2.0") << true;
    QTest::newRow("1.2.3 > 1.2.3") << GCF::Version(1,2,3) << GCF::Version("1.2.3") << false;
}

void VersionTest::testGreaterThanOperator()
{
    QFETCH(GCF::Version, v1);
    QFETCH(GCF::Version, v2);
    QFETCH(bool, v1GreaterThanV2);

    QCOMPARE(v1 > v2, v1GreaterThanV2);
}

void VersionTest::testEqualsToOperator_data()
{
    QTest::addColumn<GCF::Version>("v1");
    QTest::addColumn<GCF::Version>("v2");
    QTest::addColumn<bool>("v1EqualsV2");

    QTest::newRow("Invalid == Invalid") << GCF::Version() << GCF::Version() << true;
    QTest::newRow("1 == Invalid") << GCF::Version(1) << GCF::Version() << false;
    QTest::newRow("Invalid == 1") << GCF::Version() << GCF::Version(1) << false;
    QTest::newRow("1.2.0 == 1.1.0") << GCF::Version("1.2.0") << GCF::Version("1.1.0") << false;
    QTest::newRow("1.2.3 == 1.2.3") << GCF::Version(1,2,3) << GCF::Version("1.2.3") << true;
}

void VersionTest::testEqualsToOperator()
{
    QFETCH(GCF::Version, v1);
    QFETCH(GCF::Version, v2);
    QFETCH(bool, v1EqualsV2);

    QCOMPARE(v1 == v2, v1EqualsV2);
}

void VersionTest::testToString_data()
{
    QTest::addColumn<GCF::Version>("version");
    QTest::addColumn<QString>("versionString");

    QTest::newRow("Invalid") << GCF::Version() << QString();
    QTest::newRow("1") << GCF::Version(1) << "1.0.0";
    QTest::newRow("1.2") << GCF::Version(1,2) << "1.2.0";
    QTest::newRow("1.2.3") << GCF::Version(1,2,3) << "1.2.3";
    QTest::newRow("3.a.5") << GCF::Version("3.a.5") << QString();
    QTest::newRow("56.28.90") << GCF::Version("56.28.90") << "56.28.90";
}

void VersionTest::testToString()
{
    QFETCH(GCF::Version, version);
    QFETCH(QString, versionString);

    QCOMPARE(version.toString(), versionString);
}

QTEST_APPLESS_MAIN(VersionTest)

#include "tst_VersionTest.moc"
