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

#include <GCF3/Version>
#include <GCF3/Log>
#include <GCF3/IpcCall>
#include <GCF3/IpcServer>
#include <GCF3/Application>
#include <GCF3/SignalSpy>

#include <QElapsedTimer>
#include <QSignalSpy>
#include <QPointer>
#include <QProcess>
#include <QTcpSocket>
#include <QThread>
#include <QtConcurrentMap>
#include <QFutureWatcher>

#include "Caller.h"

const int ServerPort = 49772;
const int MaxSpinWaitTime = 60000; // 60 seconds

inline bool continueSpinWait(QElapsedTimer *timer)
{
    if(timer->hasExpired(qint64(::MaxSpinWaitTime)))
    {
        timer->invalidate();
        return false;
    }
    qApp->processEvents();
    return true;
}

#define SPIN_WAIT(eTimer) \
    if(eTimer.isValid()) \
        eTimer.restart(); \
    else \
        eTimer.start(); \
    while(continueSpinWait(&(eTimer)))

class IpcTest : public QObject
{
    Q_OBJECT
    
public:
    IpcTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testServerInstantiation();
    void setupIpcServer();
    void testValidCalls_data();
    void testValidCalls();
    void testCallToNonExistingObject();
    void testCallToEmptyMethod();
    void testCallToNonExistingMethod();
    void testCallWithMoreParameters();
    void testCallWithLessParameters();
    void testCallProtectedMethod();
    void testCallPrivateMethod();
    void testCallToObjectWithNoRemoteAccessPermissions();
    void testUnsupportedParameterType();
    void testUnsupportedReturnType();
    void testCallWithInvalidParameterType();
    void testCallWithInvalidParameterOrder();
    void testCallWithCompatibleParameters();
    void testCallWithResultReturn1();
    void testCallWithResultReturn2();
    void testCallToLongFunction();
    void testCallToLargeParameterFunction();
    void testCallToLargeReturnFunction();
    void testCallToLargeParameterAndReturnFunction();
    void testWaitForDone();
    void testCallsFromThread();
    void testMultipleCalls();
    void testCallsFromThreadPool();
    void killIpcServer();
    void testCallToNonExistingServer();
    void testCallOnWeakConnection();

private:
    QDir helperDirectory() const;
    QString logFileContents(bool deleteFile=true) const;

private:
    QProcess *m_remoteApp;
};

IpcTest::IpcTest() : m_remoteApp(0)
{
}

void IpcTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void IpcTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
    // qDebug() << "\n" << this->logFileContents();
}

void IpcTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void IpcTest::testServerInstantiation()
{
    GCF::IpcServer ipcServer;

    QVERIFY(ipcServer.isListening() == false);
    QVERIFY(ipcServer.serverPort() == 0);

    QVERIFY(ipcServer.listen());
    QVERIFY(ipcServer.isListening() == true);
    QVERIFY(ipcServer.serverPort() > 0);

    ipcServer.close();
    QVERIFY(ipcServer.isListening() == false);
    QVERIFY(ipcServer.serverPort() == 0);

    for(int i=0; i<20; i++)
    {
        QVERIFY(ipcServer.listen(QHostAddress::Any, ::ServerPort-1));
        QVERIFY(ipcServer.isListening() == true);
        QVERIFY(ipcServer.serverPort() == ::ServerPort-1);

        ipcServer.close();
        QVERIFY(ipcServer.isListening() == false);
        QVERIFY(ipcServer.serverPort() == 0);
    }
}

void IpcTest::setupIpcServer()
{
    QElapsedTimer timer;

#ifdef Q_OS_WIN
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp.exe");
#else
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp");
#endif

    QVERIFY( QFile::exists(appFile) );

    m_remoteApp = new QProcess(this);
    QSignalSpy startedSpy(m_remoteApp, SIGNAL(started()));

    m_remoteApp->start(appFile);
    SPIN_WAIT(timer)
    {
        if(startedSpy.count())
            break;
    }

    if(!startedSpy.count())
    {
        delete m_remoteApp;
        m_remoteApp = 0;
    }
    QVERIFY(startedSpy.count() == 1);

    qDebug() << "Running IpcServerApp[" << m_remoteApp->pid() << "] as server";

    QTest::qWait(100); // for the server process to start listening.

    if(m_remoteApp->state() != QProcess::Running)
    {
        qDebug() << m_remoteApp->readAllStandardError().trimmed();
        delete m_remoteApp;
        m_remoteApp = 0;
    }
}

void IpcTest::testValidCalls_data()
{
    QTest::addColumn<QString>("function");

    QTest::newRow("noParams") << "noParams";
    QTest::newRow("integer") << "integer";
    QTest::newRow("boolean") << "boolean";
    QTest::newRow("real") << "real";
    QTest::newRow("string") << "string";
    QTest::newRow("stringList") << "stringList";
    QTest::newRow("variant") << "variant";
    QTest::newRow("variantList") << "variantList";
    QTest::newRow("variantMap") << "variantMap";
    QTest::newRow("byteArray") << "byteArray";
    QTest::newRow("allParams") << "allParams";
}

void IpcTest::testValidCalls()
{
    QElapsedTimer timer;
    QFETCH(QString, function);

    QVERIFY(m_remoteApp != 0);
    QVERIFY(m_remoteApp->state() == QProcess::Running);

#ifdef Q_OS_WIN
    QString appFile = this->helperDirectory().absoluteFilePath("IpcClientApp.exe");
#else
    QString appFile = this->helperDirectory().absoluteFilePath("IpcClientApp");
#endif

    QVERIFY( QFile::exists(appFile) );

    // Prepare to start a client process and install spies on it
    QProcess proc;
    QSignalSpy startedSpy(&proc, SIGNAL(started()));
    QSignalSpy finishedSpy(&proc, SIGNAL(finished(int)));

    // Start the remote process as a client
    proc.start(appFile, QStringList() << "--function" << function);
    SPIN_WAIT(timer)
    {
        if(startedSpy.count())
            break;
    }
    QVERIFY(startedSpy.count() == 1);

    qDebug() << "Running IpcClientApp[" << proc.pid() << "] as client to test " << function;

    // Wait for the remote process to report PASS or FAIL
    SPIN_WAIT(timer)
    {
        if(finishedSpy.count())
            break;
    }

    QString procMsg = proc.readAllStandardError().trimmed();
    if(procMsg != "SUCCESS")
        qDebug() << procMsg;

    QVERIFY(finishedSpy.count() == 1);
    QVERIFY(finishedSpy.first().first().toInt() == 0);
    QVERIFY(procMsg == "SUCCESS");
}

void IpcTest::testCallToNonExistingObject()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.UnknownObject", "unknownMethod",
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Object 'Application.UnknownObject' doesnt exist");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallToEmptyMethod()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", QString(),
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Method unspecified");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallToNonExistingMethod()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "nonExistingMethod",
                                        QVariantList() << 1 << "a" << "c");

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Method 'nonExistingMethod' was not found in object");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallWithMoreParameters()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "functionWith2Parameters",
                                        QVariantList() << QVariantMap() << "simple string" << 45);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Parameter count mismatch");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallWithLessParameters()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "functionWith2Parameters",
                                        QVariantList() << QVariantMap() );

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Parameter count mismatch");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallProtectedMethod()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "protectedMethod",
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Cannot call a non-public method");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallPrivateMethod()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "privateMethod",
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Cannot call a non-public method");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallToObjectWithNoRemoteAccessPermissions()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.Object", "deleteLater",
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Meta access for this object was denied");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testUnsupportedParameterType()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "unsupportedParameter",
                                        QVariantList() << QDate::currentDate());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Argument type 'QDate' not supported");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testUnsupportedReturnType()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "unsupportedReturn",
                                        QVariantList() << 14 << 11 << 2005);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Return type 'QDate' not supported");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallWithInvalidParameterType()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    QVariantMap varMap;
    varMap["january"] = 31;
    varMap["february"] = 28;
    varMap["march"] = 31;
    varMap["april"] = 30;
    varMap["may"] = 31;
    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "variantList",
                                        QVariantList() << varMap);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Invalid parameter type. Expecting 'QVariantList' but found 'QVariantMap'");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallWithInvalidParameterOrder()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "functionWith2Parameters",
                                        QVariantList() << QString() << QVariantMap());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "Invalid parameter type. Expecting 'QVariantMap' but found 'QString'");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallWithCompatibleParameters()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "real",
                                        QVariantList() << QString("45.6778"));

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);
    QVERIFY(call->errorMessage().isEmpty() == true);
    QVERIFY(call->result() == QVariant(45.6778));
    QVERIFY(call->isSuccess() == true);
}

void IpcTest::testCallWithResultReturn1()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "testResultFunction",
                                        QVariantList() << true);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);
    QVERIFY(call->errorMessage().isEmpty() == true);
    QVERIFY(call->result() == QVariant(QString("Result is good!")));
    QVERIFY(call->isSuccess() == true);
}

void IpcTest::testCallWithResultReturn2()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "testResultFunction",
                                        QVariantList() << false);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "E_BAD_FUNC: Something went wrong here.");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallToLongFunction()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    int timeout = 200;
    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "longFunction",
                                        QVariantList() << timeout*2);
    call->setTimeoutDuration(timeout);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "A call timeout occured");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}

void IpcTest::testCallToLargeParameterFunction()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    const int fiftyMegaBytes = 52428800; // 1024*1024*50;
    QByteArray param(fiftyMegaBytes, 'A');
    const int timeout = 20000;

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "largeParameter",
                                        QVariantList() << param);
    call->setTimeoutDuration(timeout);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);
    QVERIFY(call->errorMessage().isEmpty());
    QVERIFY(call->result() == QVariant(fiftyMegaBytes));
    QVERIFY(call->isSuccess() == true);
}

void IpcTest::testCallToLargeReturnFunction()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    const int fiftyMegaBytes = 52428800; // 1024*1024*50;
    QByteArray payload(fiftyMegaBytes, '@');
    const int timeout = 20000;

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "largeReturnType",
                                        QVariantList() << fiftyMegaBytes);
    call->setTimeoutDuration(timeout);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);
    QVERIFY(call->errorMessage().isEmpty());
    QVERIFY(call->result() == QVariant(payload));
    QVERIFY(call->isSuccess() == true);
}

void IpcTest::testCallToLargeParameterAndReturnFunction()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    const int fiftyMegaBytes = 52428800; // 1024*1024*50;
    QByteArray param(fiftyMegaBytes, 'A');
    QByteArray payload(fiftyMegaBytes, '@');
    const int timeout = 20000;

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "largeParameterAndReturnType",
                                        QVariantList() << param);
    call->setTimeoutDuration(timeout);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);
    QVERIFY(call->errorMessage().isEmpty());
    QVERIFY(call->result() == QVariant(payload));
    QVERIFY(call->isSuccess() == true);
}

void IpcTest::testWaitForDone()
{
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService",
                                        "integer", QVariantList() << 10);
    bool success = call->waitForDone();
    QVERIFY(success);
    QVERIFY(call->isSuccess() == true);
    QVERIFY(call->result() == QVariant(10));
    QVERIFY(call->errorMessage().isEmpty());
}

void IpcTest::testCallsFromThread()
{
    uint seed = (uint)(QDateTime::currentMSecsSinceEpoch());
    qsrand(seed);

    const int maxThreads = 100;
    QThread threads[maxThreads];
    QList<Caller*> callers;
    qDebug() << "Calling simultaneously from " << maxThreads << " threads";

    // Load a caller into each thread and fire them away
    for(int i=0; i<maxThreads; i++)
    {
        Caller *caller = new Caller;
        callers.append(caller);

        connect(&threads[i], SIGNAL(started()), caller, SLOT(beginCall()));
        caller->moveToThread( &threads[i] );

        threads[i].start();
    }

    // Wait for all threads to complete
    bool success = true;
    for(int i=0; i<maxThreads; i++)
    {
        bool done = !threads[i].wait(::MaxSpinWaitTime);
        success &= callers.at(i)->wasCallSuccessful();
        if(!done)
        {
            if(!callers.at(i)->wasCallSuccessful())
                qDebug() << "Call " << i << " failed : " << callers.at(i)->callMessage();
            threads[i].terminate();
        }
    }

    qDeleteAll(callers);
    callers.clear();

    QVERIFY(success);
}

void IpcTest::testMultipleCalls()
{
    const int maxCallers = 1000;
    qDebug() << "Issuing " << maxCallers << " calls in sequence";

    QList<Caller*> callers;
    for(int i=0; i<maxCallers; i++)
    {
        callers.append(new Caller);
        callers.last()->beginCall();
    }

    // Wait for callers to complete
    bool success = true;
    for(int i=0; i<callers.count(); i++)
    {
        Caller *caller = callers.at(i);
        if(!caller->isDone())
        {
            GCF::SignalSpy spy(caller, SIGNAL(done(bool)));
            spy.wait(::MaxSpinWaitTime);
        }

        success &= caller->wasCallSuccessful();
        if(!caller->wasCallSuccessful())
            qDebug() << "Call at " << i << " failed : " << caller->callMessage();
    }

    qDeleteAll(callers);

    QVERIFY(success);
}

bool placeCall(bool /*unused*/)
{
    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, 49772,
                                          "Application.TestService",
                                          "largeReturnType",
                                          QVariantList() << 65536);
    bool ret = call->waitForDone();
    delete call;
    return ret;
}

void IpcTest::testCallsFromThreadPool()
{
    const int maxCallers = 1000;
    qDebug() << "Issuing " << maxCallers << " calls in a thread-pool using QtConcurrent";

    QList<bool> inputList = QVector<bool>(maxCallers, false).toList();
    QFuture<bool> future = QtConcurrent::mapped(inputList, placeCall);
    QFutureWatcher<bool> watcher;
    watcher.setFuture(future);

    GCF::SignalSpy spy(&watcher, SIGNAL(finished()));
    spy.wait(::MaxSpinWaitTime);

    bool success = true;
    QList<bool> results = future.results();
    for(int i=0; i<results.count(); i++)
    {
        success = success & results.at(i);
        if(!results.at(i))
            qDebug() << "Call at " << i << " had failed";
    }

    QVERIFY(success);
}

void IpcTest::killIpcServer()
{
    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                    "Application.TestService", "terminate",
                                    QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    QSignalSpy procSpy(m_remoteApp, SIGNAL(finished(int)));

    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == true);

    SPIN_WAIT(timer)
    {
        if(procSpy.count())
            break;
    }

    QVERIFY(m_remoteApp->state() == QProcess::NotRunning);
    delete m_remoteApp;
    m_remoteApp = 0;
}

void IpcTest::testCallToNonExistingServer()
{
    QElapsedTimer timer;

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "integer",
                                        QVariantList() << 10);
    call->setTimeoutDuration(500);

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "A connection timeout occured");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);
}


void IpcTest::testCallOnWeakConnection()
{
    this->setupIpcServer();

    QElapsedTimer timer;
    QVERIFY(m_remoteApp != 0);

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                        "Application.TestService", "terminateNow",
                                        QVariantList());

    QSignalSpy doneSpy(call, SIGNAL(done(bool)));
    SPIN_WAIT(timer)
    {
        if(doneSpy.count())
            break;
    }

    QVERIFY(doneSpy.count() == 1);
    QVERIFY(doneSpy.first().first().toBool() == false);
    QVERIFY(call->errorMessage() == "The connection was cut before any response could be received");
    QVERIFY(call->result() == QVariant());
    QVERIFY(call->isSuccess() == false);

    QTest::qWait(100);
    QVERIFY(m_remoteApp->state() == QProcess::NotRunning);
    m_remoteApp = 0;
}

QDir IpcTest::helperDirectory() const
{
    QDir dir = QDir::current();
    dir.cdUp();
    dir.cd("Helpers");
    return dir;
}

QString IpcTest::logFileContents(bool deleteFile) const
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
    IpcTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_IpcTest.moc"
