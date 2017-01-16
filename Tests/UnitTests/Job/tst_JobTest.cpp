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
#include <QIcon>
#include <QVariant>

#include <GCF3/Version>
#include <GCF3/GuiApplication>
#include <GCF3/Log>
#include <GCF3/SignalSpy>
#include <GCF3/AbstractJob>

class JobTest;

class SimpleJob : public GCF::AbstractJob
{
    Q_OBJECT

public:
    SimpleJob(QObject *parent=0)
        : GCF::AbstractJob("Simple", parent),
          m_duration(100) { }
    friend class JobTest;

    void setDuration(int val) { m_duration = val; }
    int duration() const { return m_duration; }

    Q_SLOT void cancelSelf() { this->cancel(); }
    Q_SLOT void suspendSelf() { this->suspend(); }

    GCF::Result startJob() {
        m_timer.start(m_duration, this);
        return true;
    }

    GCF::Result cancelJob() {
        m_timer.stop();
        return true;
    }

    GCF::Result suspendJob() {
        m_timer.stop();
        return true;
    }

    GCF::Result resumeJob() {
        m_timer.start(m_duration, this);
        return true;
    }

private:
    void timerEvent(QTimerEvent *te) {
        if(te->timerId() == m_timer.timerId())
            this->done();
    }

private:
    QBasicTimer m_timer;
    int m_duration;
};

class JobTest : public QObject
{
    Q_OBJECT

public:
    JobTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testJobConstructor();
    void testGlobalJobsList();
    void testJobSetMethods();
    void testStart();
    void testSuspendAndResume();
    void testCancel();
    void testRetry();
};

JobTest::JobTest()
{
}

void JobTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void JobTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void JobTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void JobTest::testJobConstructor()
{
    SimpleJob *simpleJob = new SimpleJob;

    QVERIFY(simpleJob->kind() == "Simple");
    QVERIFY(simpleJob->title().isEmpty());
    QVERIFY(simpleJob->description().isEmpty());
    QVERIFY(simpleJob->icon().isValid() == false);
    QVERIFY(simpleJob->iconUrl().isEmpty());
    QVERIFY(simpleJob->status().isEmpty());
    QVERIFY(simpleJob->progress() == 0);
    QVERIFY(simpleJob->isStarted() == false);
    QVERIFY(simpleJob->isSuspended() == false);
    QVERIFY(simpleJob->isComplete() == false);
    QVERIFY(simpleJob->hasError() == false);
    QVERIFY(simpleJob->error().isEmpty());

    delete simpleJob;
}

void JobTest::testGlobalJobsList()
{
    SimpleJob *simpleJob = new SimpleJob;
    QVERIFY(gAppService->jobs()->containsJob(simpleJob));
    delete simpleJob;
    QVERIFY(gAppService->jobs()->containsJob(simpleJob) == false);
}

void JobTest::testJobSetMethods()
{
    SimpleJob *simpleJob = new SimpleJob;
    GCF::SignalSpy spy(simpleJob, SIGNAL(updated(GCF::AbstractJob*)));

    QVERIFY(simpleJob->title().isEmpty());
    simpleJob->setTitle("Custom title");
    QVERIFY(simpleJob->title() == "Custom title");
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(simpleJob->description().isEmpty());
    simpleJob->setDescription("some description string");
    QVERIFY(simpleJob->description() == "some description string");
    QVERIFY(spy.count() == 1);
    spy.clear();

    QPixmap pm;
    QVERIFY(simpleJob->icon().isValid() == false);
    simpleJob->setIcon( QVariant::fromValue(pm) );
    QVERIFY(simpleJob->icon().isValid() == false);
    QVERIFY(spy.count() == 1);
    spy.clear();

    QIcon icon;
    QVERIFY(simpleJob->icon().isValid() == false);
    simpleJob->setIcon( QVariant::fromValue(icon) );
    QVERIFY(simpleJob->icon().isValid() == true);
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(simpleJob->iconUrl().isEmpty());
    simpleJob->setIconUrl("http://www.sample.com/icon.png");
    QVERIFY(simpleJob->iconUrl() == "http://www.sample.com/icon.png");
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(simpleJob->status().isEmpty());
    simpleJob->setStatus("job status information");
    QVERIFY(simpleJob->status() == "job status information");
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(simpleJob->progress() == 0);
    simpleJob->setProgress(45, "45% of the job is now done");
    QVERIFY(simpleJob->progress() == 0);
    QVERIFY(simpleJob->status() == "job status information");
    simpleJob->start();
    spy.clear();
    simpleJob->setProgress(45, "45% of the job is now done");
    QVERIFY(simpleJob->progress() == 45);
    QVERIFY(simpleJob->status() == "45% of the job is now done");
    QVERIFY(spy.count() == 1);
    spy.clear();

    QVERIFY(simpleJob->hasError() == false);
    QVERIFY(simpleJob->error().isEmpty());
    simpleJob->setError("error message");
    QVERIFY(simpleJob->hasError() == true);
    QVERIFY(simpleJob->error() == "error message");
    QVERIFY(spy.count() == 1);
    spy.clear();

    delete simpleJob;
}

void JobTest::testStart()
{
    GCF::Result result;

    SimpleJob *job = new SimpleJob;
    job->setDuration(500); // the job will complete itself after 500ms
    QVERIFY(job->isStarted() == false);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());
    QVERIFY(job->progress() == 0);

    GCF::SignalSpy startedSpy(job, SIGNAL(started(GCF::AbstractJob*)));
    GCF::SignalSpy updatedSpy(job, SIGNAL(updated(GCF::AbstractJob*)));
    GCF::SignalSpy completedSpy(job, SIGNAL(completed(GCF::AbstractJob*)));

    // Attempt to retry must fail
    result = job->retry();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job was not started even once to retry");
    startedSpy.clear();
    updatedSpy.clear();
    completedSpy.clear();

    // Should succeed
    QVERIFY(job->isStarted() == false);
    result = job->start();
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isRunning() == true);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->progress() == 0);
    QVERIFY(job->error().isEmpty());
    QVERIFY(result.isSuccess() == true);
    QVERIFY(startedSpy.count() == 1);
    QVERIFY(updatedSpy.count() == 1);
    QVERIFY(completedSpy.count() == 0);
    startedSpy.clear();
    updatedSpy.clear();
    completedSpy.clear();

    // Attempt to retry should fail
    result = job->retry();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Could not retry because the job is still running!");
    startedSpy.clear();
    updatedSpy.clear();
    completedSpy.clear();

    // Should be an error: Job has already started
    result = job->start();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isRunning() == true);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());
    QVERIFY(result.message() == "Job has already started");
    QVERIFY(startedSpy.count() == 0);
    QVERIFY(updatedSpy.count() == 0);
    QVERIFY(completedSpy.count() == 0);
    startedSpy.clear();
    updatedSpy.clear();
    completedSpy.clear();

    // Wait for the job to complete
    QVERIFY(completedSpy.wait());
    QVERIFY(completedSpy.count() == 1);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(startedSpy.count() == 0);

    // Ensure that things are done
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isComplete() == true);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->progress() == 100);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();

    // Now we should be unable to start the job, since it has already been started once.
    result = job->start();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job is already complete");
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isComplete() == true);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());

    delete job;
}

void JobTest::testSuspendAndResume()
{
    GCF::Result result;

    SimpleJob *job = new SimpleJob;
    job->setDuration(1000); // the job will complete itself after 500ms

    GCF::SignalSpy startedSpy(job, SIGNAL(started(GCF::AbstractJob*)));
    GCF::SignalSpy updatedSpy(job, SIGNAL(updated(GCF::AbstractJob*)));
    GCF::SignalSpy completedSpy(job, SIGNAL(completed(GCF::AbstractJob*)));
    GCF::SignalSpy suspendedSpy(job, SIGNAL(suspended(GCF::AbstractJob*)));

    // Error on trying to suspend before starting
    result = job->suspend();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Cannot suspend the job before it is started");

    // Error on trying to resume before suspend
    result = job->resume();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job is not suspended for resume");

    // Lets restart the job and cancel it mid-way
    result = job->start();
    QVERIFY(result.isSuccess() == true);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    // Error on trying to resume before suspend
    result = job->resume();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job is not suspended for resume");

    // Suspend the job mid-way
    QTimer::singleShot(200, job, SLOT(suspendSelf()));
    QVERIFY(suspendedSpy.wait());
    QVERIFY(startedSpy.count() == 0);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(completedSpy.count() == 0);
    QVERIFY(job->isRunning() == true);
    QVERIFY(job->isSuspended() == true);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->status() == "Suspended");
    QVERIFY(job->progress() < 100);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    // Error on trying to suspend again
    result = job->suspend();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job is already suspended");

    // Resume the job
    QVERIFY( job->resume().isSuccess() == true );

    // Wait for the job to complete
    QVERIFY(completedSpy.wait());
    QVERIFY(completedSpy.count() == 1);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(startedSpy.count() == 0);

    // Ensure that things are done
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isComplete() == true);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->progress() == 100);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();

    // Error on trying to suspend a completed job
    result = job->suspend();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Completed jobs cannot be suspended");

    delete job;
}

void JobTest::testCancel()
{
    GCF::Result result;

    SimpleJob *job = new SimpleJob;
    job->setDuration(1000); // the job will complete itself after 500ms

    GCF::SignalSpy startedSpy(job, SIGNAL(started(GCF::AbstractJob*)));
    GCF::SignalSpy updatedSpy(job, SIGNAL(updated(GCF::AbstractJob*)));
    GCF::SignalSpy completedSpy(job, SIGNAL(completed(GCF::AbstractJob*)));
    GCF::SignalSpy suspendedSpy(job, SIGNAL(suspended(GCF::AbstractJob*)));

    // Error on trying to cancel before start
    result = job->cancel();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job cannot be cancelled unless started");

    // Lets restart the job and cancel it mid-way
    result = job->start();
    QVERIFY(result.isSuccess() == true);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    // Lets cancel the job midway
    QTimer::singleShot(200, job, SLOT(cancelSelf()));
    QVERIFY(completedSpy.wait());
    QVERIFY(startedSpy.count() == 0);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(suspendedSpy.count() == 0);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->isComplete() == true);
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->status() == "Cancelled");
    QVERIFY(job->error() == "Cancelled");
    QVERIFY(job->hasError() == true);
    QVERIFY(job->progress() < 100);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    // Error on trying to cancel after complete
    result = job->cancel();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Completed jobs cannot be cancelled");

    // It must be possible to cancel suspended jobs. So we restart
    // the job once again, suspend it midway and then try to cancel
    // it.
    job->retry();
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->isRunning() == true);
    QVERIFY(job->error().isEmpty());
    QVERIFY(completedSpy.count() == 0);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(startedSpy.count() == 1);
    QVERIFY(suspendedSpy.count() == 0);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    QTimer::singleShot(200, job, SLOT(suspendSelf()));
    QVERIFY(suspendedSpy.wait());
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isComplete() == false);
    QVERIFY(job->isSuspended() == true);
    QVERIFY(job->hasError() == false);
    QVERIFY(job->error().isEmpty());
    QVERIFY(completedSpy.count() == 0);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(startedSpy.count() == 0);
    QVERIFY(suspendedSpy.count() == 1);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    // Now attempt to cancel the job, it should work.
    result = job->cancel();
    QVERIFY(result.isSuccess() == true);
    QVERIFY(job->isRunning() == false);
    QVERIFY(job->isStarted() == true);
    QVERIFY(job->isComplete() == true);
    QVERIFY(job->isSuspended() == false);
    QVERIFY(job->hasError() == true);
    QVERIFY(job->error() == "Cancelled");
    QVERIFY(completedSpy.count() == 1);
    QVERIFY(updatedSpy.count() > 0);
    QVERIFY(startedSpy.count() == 0);
    QVERIFY(suspendedSpy.count() == 0);
    completedSpy.clear();
    updatedSpy.clear();
    startedSpy.clear();
    suspendedSpy.clear();

    delete job;
}

void JobTest::testRetry()
{
    // The previous test case will fail if retry() did not work.
}

int main(int argc, char *argv[])
{
    GCF::GuiApplication app(argc, argv);
    JobTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_JobTest.moc"
