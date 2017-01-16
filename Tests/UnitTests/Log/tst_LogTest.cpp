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

#include <QtTest>
#include <QtDebug>
#include <QString>
#include <QThread>

#include <GCF3/Version>
#include <GCF3/Log>

#include "LogMessageHandler.h"
#include "Logger.h"

class LogTest : public QObject
{
    Q_OBJECT
    
public:
    LogTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testLogInstance();
    void testLogHandler();
    void testLogFileName();
    void testSimpleLogging();
    void testBranchLogging();
    void testNestedBranchLogging();
    void testThreadedLogging();
    void testLogModel();

private:
    QString logFileContents(bool deleteFile=true) const;
};

LogTest::LogTest()
{
}

void LogTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void LogTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void LogTest::cleanup()
{
    GCF::Log::instance()->setHandler(0);
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void LogTest::testLogInstance()
{
    QVERIFY(GCF::Log::instance() != 0);
}

void LogTest::testLogHandler()
{
    // Default handler must be the log itself
    QVERIFY(GCF::Log::instance()->handler() == GCF::Log::instance());

    // If we change the handler - then that handler must be used
    EmptyLogMessageHandler handler;
    GCF::Log::instance()->setHandler(&handler);
    QVERIFY(GCF::Log::instance()->handler() == &handler);

    // Setting handler to NULL should actually take handler as GCF::Log itself
    GCF::Log::instance()->setHandler(0);
    QVERIFY(GCF::Log::instance()->handler() == GCF::Log::instance());
}

void LogTest::testLogFileName()
{
    // Check if the default log-file name is returned properly or not
    QString dt = GCF::Log::instance()->timestamp().toString("d_M_yyyy_h_m_s_z");
    QString expected = QString("%1/Logs/%2.txt")
            .arg( GCF::applicationDataDirectoryPath() )
            .arg( dt );
    QVERIFY(GCF::Log::instance()->logFileName() == expected);

    // Check if changing of log file works or not
    QString change = "log_file.txt";
    GCF::Log::instance()->setLogFileName(change);
    QVERIFY(GCF::Log::instance()->logFileName() == change);

    // If a NULL string is set, then the log file should become default again
    GCF::Log::instance()->setLogFileName( QString() );
    QVERIFY(GCF::Log::instance()->logFileName() == expected);
}

void LogTest::testSimpleLogging()
{
    MessageToStringHandler handler;
    GCF::Log::instance()->setHandler(&handler);

    // Test fatal logging
    GCF::Log::instance()->fatal(GCF_DEFAULT_LOG_CONTEXT, "Fatal Log");
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    // Test error logging
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Error Log");
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    // Test warning logging
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Warning Log");
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    // Test debug logging
    GCF::Log::instance()->debug(GCF_DEFAULT_LOG_CONTEXT, "Debug Log");
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    // Test info logging
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Info Log");
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();
}

void LogTest::testBranchLogging()
{
    MessageToStringHandler handler;
    GCF::Log::instance()->setHandler(&handler);

    {
        GCF::LogMessageBranch branch("Fatal Branch");
        GCF::Log::instance()->fatal(GCF_DEFAULT_LOG_CONTEXT, "Fatal Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch branch("Error Branch");
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Error Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch branch("Warning Branch");
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, "Warning Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch branch("Debug Branch");
        GCF::Log::instance()->debug(GCF_DEFAULT_LOG_CONTEXT, "Debug Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch branch("Info Branch");
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Info Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();
}


void LogTest::testNestedBranchLogging()
{
    MessageToStringHandler handler;
    GCF::Log::instance()->setHandler(&handler);

    {
        GCF::LogMessageBranch root("Root Branch");
        GCF::LogMessageBranch branch("Fatal Branch");
        GCF::Log::instance()->fatal(GCF_DEFAULT_LOG_CONTEXT, "Fatal Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch root("Root Branch");
        GCF::LogMessageBranch branch("Error Branch");
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, "Error Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch root("Root Branch");
        GCF::LogMessageBranch branch("Warning Branch");
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT, "Warning Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch root("Root Branch");
        GCF::LogMessageBranch branch("Debug Branch");
        GCF::Log::instance()->debug(GCF_DEFAULT_LOG_CONTEXT, "Debug Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();

    {
        GCF::LogMessageBranch root("Root Branch");
        GCF::LogMessageBranch branch("Info Branch");
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Info Log");
    }
    QVERIFY(this->logFileContents() == handler.string());
    handler.clearString();
}

void LogTest::testThreadedLogging()
{
    uint seed = (uint)(QDateTime::currentMSecsSinceEpoch());
    qsrand(seed);

    // We will create 10 threads and have them simultaneously write logs
    // to a log file. We cant test by comparing the log output. We can
    // only ensure that there are atleast ___ lines in the log output
    // and that the test program did not crash. Thats all!

    // Create threads
    // const int maxThreads = QThread::idealThreadCount();
    const int maxThreads = 100;
    QThread threads[maxThreads];
    qDebug() << "Logging simultaneously from " << maxThreads << " threads";

    // Load a logger into each thread and fire them away
    for(int i=0; i<maxThreads; i++)
    {
        Logger *logger = new Logger;

        connect(&threads[i], SIGNAL(started()), logger, SLOT(beginLogging()));
        connect(&threads[i], SIGNAL(finished()), logger, SLOT(deleteLater()));
        logger->moveToThread( &threads[i] );

        threads[i].start();
    }

    // Wait for all threads to complete
    const int maxTimeout = maxThreads*150*5;
    for(int i=0; i<maxThreads; i++)
    {
        if( !threads[i].wait(maxTimeout) )
            threads[i].terminate();
    }

    QString logText = this->logFileContents();
    int nrLinesInLogText = logText.count("\n");
    QVERIFY(nrLinesInLogText == maxThreads*5+1);
}

void LogTest::testLogModel()
{
    // FIXME: #pragma message("We need to test the GCF::Log as a QAbstractItemModel also.")
    qDebug("TODO: We need to test the GCF::Log as a QAbstractItemModel also.");
}

QString LogTest::logFileContents(bool deleteFile) const
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

QTEST_MAIN(LogTest)

#include "tst_LogTest.moc"
