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

#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/GCFGlobal>

class ResultTest : public QObject
{
    Q_OBJECT
    
public:
    ResultTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testResultConstructors_data();
    void testResultConstructors();
    void testBoolConversion();
    void testImplicitBoolConversion();
    void testDebugStreamOperator_data();
    void testDebugStreamOperator();

private:
    void testData();
    GCF::Result implicitBoolConversionFunction1(bool input) { return input; }
    bool implicitBoolConversionFunction2(const GCF::Result &input) { return input; }
};

ResultTest::ResultTest()
{
}

void ResultTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ResultTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ResultTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void ResultTest::testResultConstructors_data()
{
    this->testData();
}

void ResultTest::testResultConstructors()
{
    QFETCH(GCF::Result, result);
    QFETCH(bool, success);
    QFETCH(QString, code);
    QFETCH(QString, message);
    QFETCH(QVariant, data);

    QCOMPARE(result.isSuccess(), success);
    QCOMPARE(result.code(), code);
    QCOMPARE(result.message(), message);
    QCOMPARE(result.data(), data);
}

void ResultTest::testBoolConversion()
{
    {
        bool success = false;
        GCF::Result result;
        result = success;

        QVERIFY(result.isSuccess() == success);
    }

    {
        bool success = true;
        GCF::Result result;
        result = success;

        QVERIFY(result.isSuccess() == success);
    }

    {
        GCF::Result result(true);
        bool success = result;

        QVERIFY(success == result.isSuccess());
    }

    {
        GCF::Result result(false);
        bool success = result;

        QVERIFY(success == result.isSuccess());
    }

    {
        GCF::Result result;
        bool success = result;

        QVERIFY(success == result.isSuccess());
    }
}

void ResultTest::testImplicitBoolConversion()
{
    {
        GCF::Result result = this->implicitBoolConversionFunction1(true);
        QVERIFY(result == true);
        QVERIFY(result.isSuccess() == true);
    }

    {
        GCF::Result result = this->implicitBoolConversionFunction1(false);
        QVERIFY(result == false);
        QVERIFY(result.isSuccess() == false);
    }

    {
        bool flag = this->implicitBoolConversionFunction2( GCF::Result(true) );
        QVERIFY(flag == true);
    }

    {
        bool flag = this->implicitBoolConversionFunction2( GCF::Result(false) );
        QVERIFY(flag == false);
    }

    {
        bool flag = this->implicitBoolConversionFunction2(true);
        QVERIFY(flag == true);
    }

    {
        bool flag = this->implicitBoolConversionFunction2(false);
        QVERIFY(flag == false);
    }
}

static QString ResultTest_qtMsg;
#if QT_VERSION >= 0x050000
void ResultTest_qtMsgToLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString logMsg = QString("%1 (%2:%3 in %4)").arg(localMsg.constData())
            .arg(context.file).arg(context.line).arg(context.function);

    switch (type)
    {
    case QtDebugMsg:
        ResultTest_qtMsg = msg;
        break;
    default:
        break;
    }
}
#else
void ResultTest_qtMsgToLogHandler(QtMsgType type, const char *msg)
{
    switch (type)
    {
    case QtDebugMsg:
        ResultTest_qtMsg = QString::fromLatin1(msg);
        break;
    default:
        break;
    }
}
#endif

void ResultTest::testDebugStreamOperator_data()
{
    this->testData();
}

void ResultTest::testDebugStreamOperator()
{
    QFETCH(GCF::Result, result);
    QFETCH(bool, success);
    QFETCH(QString, code);
    QFETCH(QString, message);

#if QT_VERSION >= 0x050000
    QtMessageHandler handler = qInstallMessageHandler(ResultTest_qtMsgToLogHandler);
#else
    QtMsgHandler handler = qInstallMsgHandler(ResultTest_qtMsgToLogHandler);
#endif

    qDebug() << result;

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(handler);
#else
    qInstallMsgHandler(handler);
#endif

    QString expectedMsg = QString("GCF::Result(%1. CODE:\"%2\". MSG:\"%3\") ")
            .arg(success ? "true" : "false").arg(code).arg(message);
    QCOMPARE(ResultTest_qtMsg, expectedMsg);
}

void ResultTest::testData()
{
    QTest::addColumn<GCF::Result>("result");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QString>("code");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QVariant>("data");

    QTest::newRow("default") << GCF::Result() << false << "FAILURE" << QString("unknown cause") << QVariant();
    QTest::newRow("true") << GCF::Result(true) << true << "SUCCESS" << QString() << QVariant();
    QTest::newRow("false") << GCF::Result(false) << false << "FAILURE" << QString("unknown cause") << QVariant();
    QTest::newRow("false,E_INVALID") << GCF::Result(false,"E_INVALID") << false << "E_INVALID" << QString("unknown cause") << QVariant();
    QTest::newRow("true,E_SUPER_CALL,'This was an amazing call'")
            << GCF::Result(true, "E_SUPER_CALL", "This was an amazing call") << true << QString() << QString() << QVariant();
    QTest::newRow("true,E_SUPER_CALL,'This was an amazing call',45")
            << GCF::Result(true, "E_SUPER_CALL", "This was an amazing call", 45) << true << QString() << QString() << QVariant(45);
    QTest::newRow("false,E_SUPER_CALL,'This was an amazing call',45")
            << GCF::Result(false, "E_SUPER_CALL", "This was an amazing call", 45) << false
            << "E_SUPER_CALL" << "This was an amazing call" << QVariant();
    QTest::newRow("false,'','',45") << GCF::Result(false, QString(), QString(), QVariant(45)) << false << "FAILURE" << QString("unknown cause") << QVariant();
}

QTEST_APPLESS_MAIN(ResultTest)

#include "tst_ResultTest.moc"
