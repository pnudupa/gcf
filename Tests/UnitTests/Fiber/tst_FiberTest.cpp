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

#include "FiberAccess.h"

#include <math.h>

#include <GCF3/Version>
#include <GCF3/Log>
#include <GCF3/Application>
#include <GCF3/SignalSpy>

#include <QtTest>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QDate>
#include <QProcess>

#include <QDebug>

class FiberTest : public QObject
{
    Q_OBJECT

public:
    FiberTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void launchLocalFiber();
    void testCreateSession();

    // Test valid calls on Async and LazyAsync mode
    void testValidAsyncCalls_data();
    void testValidAsyncCalls();
    void testValidLazyAsyncCalls_data();
    void testValidLazyAsyncCalls();

    // Test functions with heavy load
    void testHeavyDutyFunctions_data();
    void testHeavyDutyFunctions();

    // Test for error messages
    void testUnsupportedDatatype_data();
    void testUnsupportedDatatype();
    void testRestrictedFunctions_data();
    void testRestrictedFunctions();
    void testIncorrectParameters_data();
    void testIncorrectParameters();

    void testCrashAndResume();

    void testTerminateSession();
    void shutdownLocalFiber();

private:
    FiberAccess *m_fiberAccess;
    bool m_localFiberServer;
};

FiberTest::FiberTest()
{
    m_localFiberServer = true;
    QString fiberAddress = "http://localhost";

    if(qApp->arguments().count() == 2)
    {
        fiberAddress = qApp->arguments().last();
        m_localFiberServer = false;
    }

    m_fiberAccess = new FiberAccess(fiberAddress, this);
}

void FiberTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void FiberTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void FiberTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void FiberTest::launchLocalFiber()
{
    if(!m_localFiberServer)
#if QT_VERSION >= 0x050000
        QSKIP("Not testing a local server");
#else
        QSKIP("Not testing a local server", SkipAll);
#endif

    QProcess::execute("FiberCtl start");
}

void FiberTest::testCreateSession()
{
    GCF::SignalSpy successSpy(m_fiberAccess, SIGNAL(sessionCreated()));
    GCF::SignalSpy failSpy(m_fiberAccess, SIGNAL(sessionCreationFailed()));

    m_fiberAccess->createSession();
    successSpy.wait(10000);

    QVERIFY(successSpy.count() == 1);
    QVERIFY(failSpy.count() == 0);
    QVERIFY(m_fiberAccess->hasSession() == true);
}

inline void populateValidCallsData()
{
    QTest::addColumn<QString>("componentType");
    QTest::addColumn<QString>("componentName");
    QTest::addColumn<QString>("function");
    QTest::addColumn<QVariantList>("args");
    QTest::addColumn<QVariant>("expectedResult");

    QVariantList allParams;
    allParams.append(10);
    allParams.append(false);
    allParams.append(123.34);
    allParams.append(QString("string value"));
    allParams.append(QStringList() << "a" << "b" << "C");
    allParams.append(QVariant(QDate::currentDate()));

    QVariantList varList;
    varList.append( QDate::currentDate() );
    varList.append( QString("this is a simple string") );
    varList.append( true );
    varList.append( 69123.123 );
    allParams.append( QVariant(varList) );

    QVariantMap varMap;
    varMap["january"] = 31;
    varMap["february"] = 28;
    varMap["march"] = 31;
    varMap["april"] = 30;
    varMap["may"] = 31;

    allParams.append( QVariant(varMap) );

    QByteArray bytes(4096, 'A');
    allParams.append(bytes);

    QTest::newRow("coreNoParamsCoreComponent") << "CORE" << "SimpleCoreComponent" << "noParams" << (QVariantList() << QVariant()) << QVariant();
    QTest::newRow("coreIntegerCoreComponent") << "CORE" << "SimpleCoreComponent" << "integer" << (QVariantList() << 10) << QVariant(10);
    QTest::newRow("coreBooleanCoreComponent") << "CORE" << "SimpleCoreComponent" << "boolean" << (QVariantList() << true) << QVariant(true);
    QTest::newRow("coreRealCoreComponent") << "CORE" << "SimpleCoreComponent" << "real" << (QVariantList() << 10.12) << QVariant(10.12);
    QTest::newRow("coreStringCoreComponent") << "CORE" << "SimpleCoreComponent" << "string" << (QVariantList() << "Fiber rocks!") << QVariant("Fiber rocks!");
    QTest::newRow("coreStringListCoreComponent") << "CORE" << "SimpleCoreComponent" << "stringList" << (QVariantList() << QVariant(QStringList() << "One" << "Two" << "Three")) << QVariant((QStringList() << "One" << "Two" << "Three"));
    QTest::newRow("coreByteArrayCoreComponent") << "CORE" << "SimpleCoreComponent" << "byteArray" << (QVariantList() << bytes) << QVariant(bytes);
    QTest::newRow("coreTestResultFunctionCoreComponent") << "CORE" << "SimpleCoreComponent" << "testResultFunction" << (QVariantList() << true) << QVariant("Result is good!");
#if QT_VERSION >= 0x050000
    QTest::newRow("coreVariantCoreComponent") << "CORE" << "SimpleCoreComponent" << "variant" << (QVariantList() << QVariant(QDate::currentDate())) << QVariant(QDate::currentDate());
    QTest::newRow("coreVariantListCoreComponent") << "CORE" << "SimpleCoreComponent" << "variantList" << (QVariantList() << QVariant(varList)) << QVariant(varList);
    QTest::newRow("coreVariantMapCoreComponent") << "CORE" << "SimpleCoreComponent" << "variantMap" << (QVariantList() << varMap) << QVariant(varMap);
    QTest::newRow("coreAllParamsCoreComponent") << "CORE" << "SimpleCoreComponent" << "allParams" << allParams << QVariant(allParams);
#endif

    QTest::newRow("guiNoParamsCoreComponent") << "GUI" << "SimpleCoreComponent" << "noParams" << (QVariantList() << QVariant()) << QVariant();
    QTest::newRow("guiIntegerCoreComponent") << "GUI" << "SimpleCoreComponent" << "integer" << (QVariantList() << 10) << QVariant(10);
    QTest::newRow("guiBooleanCoreComponent") << "GUI" << "SimpleCoreComponent" << "boolean" << (QVariantList() << true) << QVariant(true);
    QTest::newRow("guiRealCoreComponent") << "GUI" << "SimpleCoreComponent" << "real" << (QVariantList() << 10.12) << QVariant(10.12);
    QTest::newRow("guiStringCoreComponent") << "GUI" << "SimpleCoreComponent" << "string" << (QVariantList() << "Fiber rocks!") << QVariant("Fiber rocks!");
    QTest::newRow("guiStringListCoreComponent") << "GUI" << "SimpleCoreComponent" << "stringList" << (QVariantList() << QVariant(QStringList() << "One" << "Two" << "Three")) << QVariant((QStringList() << "One" << "Two" << "Three"));
    QTest::newRow("guiByteArrayCoreComponent") << "GUI" << "SimpleCoreComponent" << "byteArray" << (QVariantList() << bytes) << QVariant(bytes);
    QTest::newRow("guiTestResultFunctionCoreComponent") << "GUI" << "SimpleCoreComponent" << "testResultFunction" << (QVariantList() << true) << QVariant("Result is good!");
#if QT_VERSION >= 0x50000
    QTest::newRow("guiVariantCoreComponent") << "GUI" << "SimpleCoreComponent" << "variant" << (QVariantList() << QVariant(QDate::currentDate())) << QVariant(QDate::currentDate());
    QTest::newRow("guiVariantListCoreComponent") << "GUI" << "SimpleCoreComponent" << "variantList" << (QVariantList() << QVariant(varList)) << QVariant(varList);
    QTest::newRow("guiVariantMapCoreComponent") << "GUI" << "SimpleCoreComponent" << "variantMap" << (QVariantList() << varMap) << QVariant(varMap);
    QTest::newRow("guiAllParamsCoreComponent") << "GUI" << "SimpleCoreComponent" << "allParams" << allParams << QVariant(allParams);
#endif

    QTest::newRow("guiNoParamsGuiComponent") << "GUI" << "SimpleGuiComponent" << "noParams" << (QVariantList() << QVariant()) << QVariant();
    QTest::newRow("guiIntegerGuiComponent") << "GUI" << "SimpleGuiComponent" << "integer" << (QVariantList() << 10) << QVariant(10);
    QTest::newRow("guiBooleanGuiComponent") << "GUI" << "SimpleGuiComponent" << "boolean" << (QVariantList() << true) << QVariant(true);
    QTest::newRow("guiRealGuiComponent") << "GUI" << "SimpleGuiComponent" << "real" << (QVariantList() << 10.12) << QVariant(10.12);
    QTest::newRow("guiStringGuiComponent") << "GUI" << "SimpleGuiComponent" << "string" << (QVariantList() << "Fiber rocks!") << QVariant("Fiber rocks!");
    QTest::newRow("guiStringListGuiComponent") << "GUI" << "SimpleGuiComponent" << "stringList" << (QVariantList() << QVariant(QStringList() << "One" << "Two" << "Three")) << QVariant((QStringList() << "One" << "Two" << "Three"));
    QTest::newRow("guiByteArrayGuiComponent") << "GUI" << "SimpleGuiComponent" << "byteArray" << (QVariantList() << bytes) << QVariant(bytes);
    QTest::newRow("guiTestResultFunctionCoreComponent") << "GUI" << "SimpleCoreComponent" << "testResultFunction" << (QVariantList() << true) << QVariant("Result is good!");
#if QT_VERSION >= 0x50000
    QTest::newRow("guiVariantGuiComponent") << "GUI" << "SimpleGuiComponent" << "variant" << (QVariantList() << QVariant(QDate::currentDate())) << QVariant(QDate::currentDate());
    QTest::newRow("guiVariantListGuiComponent") << "GUI" << "SimpleGuiComponent" << "variantList" << (QVariantList() << QVariant(varList)) << QVariant(varList);
    QTest::newRow("guiVariantMapGuiComponent") << "GUI" << "SimpleGuiComponent" << "variantMap" << (QVariantList() << varMap) << QVariant(varMap);
    QTest::newRow("guiAllParamsGuiComponent") << "GUI" << "SimpleGuiComponent" << "allParams" << allParams << QVariant(allParams);
#endif
}

void FiberTest::testValidAsyncCalls_data()
{
    populateValidCallsData();
}

void FiberTest::testValidAsyncCalls()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QVariantList, args);
    QFETCH(QVariant, expectedResult);

    GCF::SignalSpy resultSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall(componentName, componentName, "TestService", function, componentType, args);
    resultSpy.wait(30000);

    QVariant result = m_fiberAccess->lastCallResult().value("result");

    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(result, expectedResult);
}

void FiberTest::testValidLazyAsyncCalls_data()
{
    populateValidCallsData();
}

void FiberTest::testValidLazyAsyncCalls()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QVariantList, args);
    QFETCH(QVariant, expectedResult);

    GCF::SignalSpy resultAnnounceSpy(m_fiberAccess, SIGNAL(lazyAsyncCallResultAnnounced(int)));
    m_fiberAccess->lazyAsyncCall(componentName, componentName, "TestService", function, componentType, args);
    resultAnnounceSpy.wait(10000);

    QCOMPARE(resultAnnounceSpy.count(), 1);

    GCF::SignalSpy downloadProgressSpy(m_fiberAccess, SIGNAL(lazyAsyncCallResultDownloadProgress(int,int)));
    GCF::SignalSpy resultDownloadSpy(m_fiberAccess, SIGNAL(lazyAsyncCallResultAvailable()));
    m_fiberAccess->downloadLazyAsyncCallResult(50);
    resultDownloadSpy.wait(30000);

    QVariant result = m_fiberAccess->lastCallResult().value("result");
    QCOMPARE(result, expectedResult);

    int numberOfProgressSignalEmitted = downloadProgressSpy.count();
    int expectedNumberOfSignalEmission = ::ceil((double)m_fiberAccess->availableResponseSize() / 50);
    QCOMPARE(numberOfProgressSignalEmitted, expectedNumberOfSignalEmission);
}

void FiberTest::testHeavyDutyFunctions_data()
{
    QTest::addColumn<QString>("componentType");
    QTest::addColumn<QString>("componentName");
    QTest::addColumn<QString>("function");
    QTest::addColumn<QVariantList>("args");
    QTest::addColumn<QVariant>("expectedResult");

    // The largeDataSize function test parameters (size and timeout) should be adjusted
    // based on the client and server platforms. Hence by default a small size is
    // specified here
    int largeDataSize = 4 * 1024;
    QByteArray data(largeDataSize, '@');

    QTest::newRow("coreLongFunctionCoreComponent") << "CORE" << "SimpleCoreComponent" << "longFunction" << (QVariantList() << 400) << QVariant();
    QTest::newRow("coreLargeParameterCoreComponent") << "CORE" << "SimpleCoreComponent" << "largeParameter" << (QVariantList() << QVariant(data)) << QVariant(largeDataSize);
    QTest::newRow("coreLargeReturnTypeCoreComponent") << "CORE" << "SimpleCoreComponent" << "largeReturnType" << (QVariantList() << largeDataSize) << QVariant(data);
    QTest::newRow("coreLargeParameterAndReturnTypeCoreComponent") << "CORE" << "SimpleCoreComponent" << "largeParameterAndReturnType" << (QVariantList() << QVariant(data)) << QVariant(data);

    QTest::newRow("guiLongFunctionCoreComponent") << "GUI" << "SimpleCoreComponent" << "longFunction" << (QVariantList() << 400) << QVariant();
    QTest::newRow("guiLargeParameterCoreComponent") << "GUI" << "SimpleCoreComponent" << "largeParameter" << (QVariantList() << data) << QVariant(largeDataSize);
    QTest::newRow("guiLargeReturnTypeCoreComponent") << "GUI" << "SimpleCoreComponent" << "largeReturnType" << (QVariantList() << largeDataSize) << QVariant(data);
    QTest::newRow("guiLargeParameterAndReturnTypeCoreComponent") << "GUI" << "SimpleCoreComponent" << "largeParameterAndReturnType" << (QVariantList() << QVariant(data)) << QVariant(data);

    QTest::newRow("guiLongFunctionGuiComponent") << "GUI" << "SimpleGuiComponent" << "longFunction" << (QVariantList() << 400) << QVariant();
    QTest::newRow("guiLargeParameterGuiComponent") << "GUI" << "SimpleGuiComponent" << "largeParameter" << (QVariantList() << data) << QVariant(largeDataSize);
    QTest::newRow("guiLargeReturnTypeGuiComponent") << "GUI" << "SimpleGuiComponent" << "largeReturnType" << (QVariantList() << largeDataSize) << QVariant(data);
    QTest::newRow("guiLargeParameterAndReturnTypeGuiComponent") << "GUI" << "SimpleGuiComponent" << "largeParameterAndReturnType" << (QVariantList() << QVariant(data)) << QVariant(data);
}

void FiberTest::testHeavyDutyFunctions()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QVariantList, args);
    QFETCH(QVariant, expectedResult);

    GCF::SignalSpy resultAnnounceSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall(componentName, componentName, "TestService", function, componentType, args);
    resultAnnounceSpy.wait(30000);

    QVERIFY(resultAnnounceSpy.count() == 1);

    QVariantMap result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == true);
    QVERIFY(result.value("result") == expectedResult);
}

void FiberTest::testUnsupportedDatatype_data()
{
    QTest::addColumn<QString>("componentType");
    QTest::addColumn<QString>("componentName");
    QTest::addColumn<QString>("function");
    QTest::addColumn<QVariantList>("args");
    QTest::addColumn<QString>("expectedResult");

    QString errorMessage = "Argument type 'QDate' not supported";
    QTest::newRow("coreUnsupportedParameterCoreComponent") << "CORE" << "SimpleCoreComponent" << "unsupportedParameter" << (QVariantList() << QDate::currentDate()) << errorMessage;
    QTest::newRow("guiUnsupportedParameterCoreComponent") << "GUI" << "SimpleCoreComponent" << "unsupportedParameter" << (QVariantList() << QDate::currentDate()) << errorMessage;
    QTest::newRow("guiUnsupportedParameterGuiComponent") << "GUI" << "SimpleGuiComponent" << "unsupportedParameter" << (QVariantList() << QDate::currentDate()) << errorMessage;

    errorMessage = "Return type 'QDate' not supported";
    QTest::newRow("coreUnsupportedReturnCoreComponent") << "CORE" << "SimpleCoreComponent" << "unsupportedReturn" << (QVariantList() << 12 << 12 << 12) << errorMessage;
    QTest::newRow("guiUnsupportedReturnCoreComponent") << "GUI" << "SimpleCoreComponent" << "unsupportedReturn" << (QVariantList() << 12 << 12 << 12) << errorMessage;
    QTest::newRow("guiUnsupportedReturnGuiComponent") << "GUI" << "SimpleGuiComponent" << "unsupportedReturn" << (QVariantList() << 12 << 12 << 12) << errorMessage;
}

void FiberTest::testUnsupportedDatatype()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QVariantList, args);
    QFETCH(QString, expectedResult);

    GCF::SignalSpy resultAnnounceSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall(componentName, componentName, "TestService", function, componentType, args);
    resultAnnounceSpy.wait(10000);

    QVERIFY(resultAnnounceSpy.count() == 1);

    QVariantMap result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == false);
#if QT_VERSION >= 0x50000
    // QVERIFY(result.value("error").toString() == expectedResult);
#endif
}

void FiberTest::testRestrictedFunctions_data()
{
    QTest::addColumn<QString>("componentType");
    QTest::addColumn<QString>("componentName");
    QTest::addColumn<QString>("function");
    QTest::addColumn<QString>("expectedResult");

    QString errorMessage = "Cannot call a non-public method";
    QTest::newRow("coreProtectedMethodCoreComponent") << "CORE" << "SimpleCoreComponent" << "protectedMethod"  << errorMessage;
    QTest::newRow("guiProtectedMethodCoreComponent") << "GUI" << "SimpleCoreComponent" << "protectedMethod"  << errorMessage;
    QTest::newRow("guiProtectedMethodGuiComponent") << "GUI" << "SimpleGuiComponent" << "protectedMethod"  << errorMessage;

    QTest::newRow("corePrivateMethodCoreComponent") << "CORE" << "SimpleCoreComponent" << "privateMethod"  << errorMessage;
    QTest::newRow("guiPrivateMethodCoreComponent") << "GUI" << "SimpleCoreComponent" << "privateMethod"  << errorMessage;
    QTest::newRow("guiPrivateMethodGuiComponent") << "GUI" << "SimpleGuiComponent" << "privateMethod"  << errorMessage;
}

void FiberTest::testRestrictedFunctions()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QString, expectedResult);

    GCF::SignalSpy resultAnnounceSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall(componentName, componentName, "TestService", function, componentType);
    resultAnnounceSpy.wait(10000);

    QVERIFY(resultAnnounceSpy.count() == 1);

    QVariantMap result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == false);
    QVERIFY(result.value("error").toString() == expectedResult);
}

void FiberTest::testIncorrectParameters_data()
{
    QTest::addColumn<QString>("componentType");
    QTest::addColumn<QString>("componentName");
    QTest::addColumn<QString>("function");
    QTest::addColumn<QVariantList>("args");
    QTest::addColumn<QString>("expectedResult");

    QString errorMessage = "Invalid session type";
    QTest::newRow("invalidComponentType") << "FUN" << "SimpleCoreComponent" << "allParams" << (QVariantList() << QVariant()) << errorMessage;

    errorMessage = "Couldn't load service library Component";
    QTest::newRow("coreInvalidComponentName") << "CORE" << "Component" << "allParams" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Method 'helloWorld' was not found in object";
    QTest::newRow("coreInvalidMethodNameOnCoreComponent") << "CORE" << "SimpleCoreComponent" << "helloWorld" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Parameter count mismatch";
    QTest::newRow("coreInvalidNumberOfParametersOnCoreComponent") << "CORE" << "SimpleCoreComponent" << "allParams" << (QVariantList() << QVariant()) << errorMessage;

    errorMessage = "Couldn't load service library Component";
    QTest::newRow("guiInvalidComponentName") << "GUI" << "Component" << "allParams" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Method 'helloWorld' was not found in object";
    QTest::newRow("guiInvalidMethodNameOnCoreComponent") << "GUI" << "SimpleCoreComponent" << "helloWorld" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Parameter count mismatch";
    QTest::newRow("guiInvalidNumberOfParametersOnCoreComponent") << "GUI" << "SimpleCoreComponent" << "allParams" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Method 'helloWorld' was not found in object";
    QTest::newRow("guiInvalidMethodNameOnGuiComponent") << "GUI" << "SimpleGuiComponent" << "helloWorld" << (QVariantList() << QVariant()) << errorMessage;
    errorMessage = "Parameter count mismatch";
    QTest::newRow("guiInvalidNumberOfParametersOnGuiComponent") << "GUI" << "SimpleGuiComponent" << "allParams" << (QVariantList() << QVariant()) << errorMessage;
}

void FiberTest::testIncorrectParameters()
{
    QFETCH(QString, componentType);
    QFETCH(QString, componentName);
    QFETCH(QString, function);
    QFETCH(QVariantList, args);
    QFETCH(QString, expectedResult);

    GCF::SignalSpy resultAnnounceSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall(componentName, componentName, "TestService", function, componentType, args);
    resultAnnounceSpy.wait(10000);

    QVERIFY(resultAnnounceSpy.count() == 1);

    QVariantMap result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == false);
    QVERIFY(result.value("error").toString() == expectedResult);
}

void FiberTest::testCrashAndResume()
{
    GCF::SignalSpy crashSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall("SimpleGuiComponent", "SimpleGuiComponent", "TestService", "noParams", "CORE");
    crashSpy.wait(10000);

    QVERIFY(crashSpy.count() == 1);

    QVariantMap result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == false);
    QVERIFY(result.value("error").toString() == "Crashed the server");

    GCF::SignalSpy successSpy(m_fiberAccess, SIGNAL(asyncCallResultAvailable()));
    m_fiberAccess->asyncCall("SimpleCoreComponent", "SimpleCoreComponent", "TestService", "noParams", "CORE");
    successSpy.wait(10000);

    QVERIFY(successSpy.count() == 1);

    result = m_fiberAccess->lastCallResult();
    QVERIFY(result.value("success").toBool() == true);
    QVERIFY(result.value("result") == QVariant());
}

void FiberTest::testTerminateSession()
{
    GCF::SignalSpy successSpy(m_fiberAccess, SIGNAL(sessionTerminated()));
    GCF::SignalSpy failSpy(m_fiberAccess, SIGNAL(sessionTerminationFailed()));

    m_fiberAccess->terminateSession();
    successSpy.wait(10000);

    QVERIFY(successSpy.count() == 1);
    QVERIFY(failSpy.count() == 0);
    QVERIFY(m_fiberAccess->hasSession() == false);
}

void FiberTest::shutdownLocalFiber()
{
    if(!m_localFiberServer)
#if QT_VERSION >= 0x050000
        QSKIP("Not testing a local server");
#else
        QSKIP("Not testing a local server", SkipAll);
#endif

    QProcess::execute("FiberCtl stop");
}

int main(int argc, char *argv[])
{
    GCF::Application app(argc, argv);
    FiberTest tc;
    return QTest::qExec(&tc);
}

#include "tst_FiberTest.moc"
