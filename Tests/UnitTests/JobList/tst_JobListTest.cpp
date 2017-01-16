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
#if QT_VERSION >= 0x050000
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#else
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#endif
#include <QDialog>
#include <QHBoxLayout>

#include <GCF3/Version>
#include <GCF3/GuiApplication>
#include <GCF3/Log>
#include <GCF3/SignalSpy>
#include <GCF3/AbstractJob>
#include <GCF3/JobListModel>

class JobListTest;

class SimpleJob : public GCF::AbstractJob
{
    Q_OBJECT

public:
    SimpleJob(QObject *parent=0)
        : GCF::AbstractJob("Simple", parent),
          m_duration(100) { }
    friend class JobListTest;

    void setDuration(int val) { m_duration = val; }
    int duration() const { return m_duration; }

    Q_SLOT void cancelSelf() { this->cancel(); }
    Q_SLOT void suspendSelf() { this->suspend(); }
    Q_SLOT void resumeSelf() { this->resume(); }

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

class JobListTest : public QObject
{
    Q_OBJECT

public:
    JobListTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testConstructor();
    void testRoles();
    void testColumnNames();
    void testSignals();
    void testGlobalJobsList();
    void testAddJobOrder();
};

JobListTest::JobListTest()
{
}

void JobListTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void JobListTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void JobListTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void JobListTest::testConstructor()
{
    GCF::JobListModel jobsList;
    QVERIFY(jobsList.jobCount() == 0);

    for(int i=0; i<10; i++)
        QVERIFY(jobsList.jobAt(i) == 0);
    QVERIFY(jobsList.columns().isEmpty());

    QVERIFY(jobsList.rowCount() == 0);
    QVERIFY(jobsList.columnCount() == 1);

    QVERIFY(gAppService->jobs() != 0);
}

void JobListTest::testRoles()
{
    GCF::JobListModel jobList;

    QHash<int,QByteArray> roles;
#define ADD_ROLE(x) roles[GCF::JobListModel::x] = "job" #x;
    ADD_ROLE(Kind);
    ADD_ROLE(Title);
    ADD_ROLE(Description);
    ADD_ROLE(Icon);
    ADD_ROLE(IconUrl);
    ADD_ROLE(Progress);
    ADD_ROLE(Status);
    ADD_ROLE(IsStarted);
    ADD_ROLE(IsSuspended);
    ADD_ROLE(IsComplete);
    ADD_ROLE(IsRunning);
    ADD_ROLE(HasError);
    ADD_ROLE(Error);
    ADD_ROLE(Object);
#undef ADD_ROLE

    QVERIFY(roles == jobList.roleNames());
}

void JobListTest::testColumnNames()
{
    GCF::JobListModel jobsList;
    QHash<int,QByteArray> roles = jobsList.roleNames();

    for(int i=GCF::JobListModel::Kind; i<GCF::JobListModel::Object; i++)
    {
        QString role = QString::fromLatin1( roles.value(i) );
        role.remove(0, 3);
        jobsList.setColumns(QList<int>() << i);
        QVERIFY(role == jobsList.headerData(0,Qt::Horizontal,Qt::DisplayRole).toString());
    }
}

void JobListTest::testSignals()
{
    GCF::JobListModel jobsList;

    QMap<QByteArray,GCF::SignalSpy*> spyMap;
    spyMap["jobCountChanged"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobCountChanged()));
    spyMap["allJobsComplete"] = new GCF::SignalSpy(&jobsList, SIGNAL(allJobsComplete()));
    spyMap["jobUpdated"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobUpdated(GCF::AbstractJob*)));
    spyMap["jobStarted"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobStarted(GCF::AbstractJob*)));
    spyMap["jobSuspended"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobSuspended(GCF::AbstractJob*)));
    spyMap["jobResumed"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobResumed(GCF::AbstractJob*)));
    spyMap["jobCompleted"] = new GCF::SignalSpy(&jobsList, SIGNAL(jobCompleted(GCF::AbstractJob*)));

    // Add some jobs to the list
    QList<GCF::AbstractJob*> jobs;
    for(int i=0; i<10; i++)
    {
        SimpleJob *job = new SimpleJob;
        job->setDuration(2500+i*10);
        job->setTitle( QString("Title %1").arg(i+1) );
        job->setDescription( QString("Desc %1").arg(i+1) );
        job->setIconUrl( QString("http://www.sample.com/icon%1.png").arg(i+1) );
        job->setStatus( QString("status %1").arg(i+1) );
        jobs.append(job);
        jobsList.addJob(job);
        QTimer::singleShot(200, job, SLOT(suspendSelf()));
        QTimer::singleShot(350, job, SLOT(resumeSelf()));
        job->start();
        job->setProgress(20+3*i, QString("Progress %1").arg(i+1));
    }

#if QT_VERSION >= 0x050000
    QQuickView qmlView;
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
#else
    QDeclarativeView qmlView;
    qmlView.setResizeMode(QDeclarativeView::SizeRootObjectToView);
#endif
    qmlView.engine()->rootContext()->setContextProperty("jobsList", &jobsList);
#if QT_VERSION >= 0x050000
    qmlView.setSource( QUrl("qrc:///Qml/JobsListViewQt5.qml") );
#else
    qmlView.setSource( QUrl("qrc:///Qml/JobsListViewQt4.qml") );
#endif
    qmlView.resize(800, 600);
    qmlView.show();

#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&qmlView);
#else
    QTest::qWaitForWindowShown(&qmlView);
#endif

    connect(&jobsList, SIGNAL(allJobsComplete()), &qmlView, SLOT(close()));

    // Check if fields are coming properly
    QVERIFY(jobsList.rowCount() == jobs.count());
    QVERIFY(jobsList.columnCount() == 1);
    for(int i=0; i<jobs.count(); i++)
    {
        GCF::AbstractJob *job = jobs.at(jobs.count()-i-1);
        QModelIndex index = jobsList.index(i, 0);
        QVERIFY( index.data(GCF::JobListModel::Kind).toString() == job->kind() );
        QVERIFY( index.data(GCF::JobListModel::Title).toString() == job->title() );
        QVERIFY( index.data(GCF::JobListModel::Description).toString() == job->description() );
        QVERIFY( index.data(GCF::JobListModel::IconUrl).toString() == job->iconUrl() );
        QVERIFY( index.data(GCF::JobListModel::Progress).toInt() == job->progress() );
        QVERIFY( index.data(GCF::JobListModel::Status).toString() == job->status() );
        QVERIFY( index.data(GCF::JobListModel::IsStarted).toBool() == job->isStarted() );
        QVERIFY( index.data(GCF::JobListModel::IsSuspended).toBool() == job->isSuspended() );
        QVERIFY( index.data(GCF::JobListModel::IsComplete).toBool() == job->isComplete() );
        QVERIFY( index.data(GCF::JobListModel::HasError).toBool() == job->hasError() );
        QVERIFY( index.data(GCF::JobListModel::Error).toString() == job->error() );
        QVERIFY( index.data(GCF::JobListModel::Object).value<QObject*>() == job );
    }

    // Wait for suspend to happen
    QTest::qWait(250);

    // Check if fields are coming properly
    QVERIFY(jobsList.rowCount() == jobs.count());
    QVERIFY(jobsList.columnCount() == 1);
    for(int i=0; i<jobs.count(); i++)
    {
        GCF::AbstractJob *job = jobs.at(jobs.count()-i-1);
        QModelIndex index = jobsList.index(i, 0);
        QVERIFY( index.data(GCF::JobListModel::Kind).toString() == job->kind() );
        QVERIFY( index.data(GCF::JobListModel::Title).toString() == job->title() );
        QVERIFY( index.data(GCF::JobListModel::Description).toString() == job->description() );
        QVERIFY( index.data(GCF::JobListModel::IconUrl).toString() == job->iconUrl() );
        QVERIFY( index.data(GCF::JobListModel::Progress).toInt() == job->progress() );
        QVERIFY( index.data(GCF::JobListModel::Status).toString() == job->status() );
        QVERIFY( index.data(GCF::JobListModel::IsStarted).toBool() == job->isStarted() );
        QVERIFY( index.data(GCF::JobListModel::IsSuspended).toBool() == job->isSuspended() );
        QVERIFY( index.data(GCF::JobListModel::IsComplete).toBool() == job->isComplete() );
        QVERIFY( index.data(GCF::JobListModel::HasError).toBool() == job->hasError() );
        QVERIFY( index.data(GCF::JobListModel::Error).toString() == job->error() );
        QVERIFY( index.data(GCF::JobListModel::Object).value<QObject*>() == job );
    }

    // Wait for all jobs in the list to complete
    QVERIFY(spyMap["allJobsComplete"]->wait());

    // Check if fields are coming properly
    QVERIFY(jobsList.rowCount() == jobs.count());
    QVERIFY(jobsList.columnCount() == 1);
    for(int i=0; i<jobs.count(); i++)
    {
        GCF::AbstractJob *job = jobs.at(jobs.count()-i-1);
        QModelIndex index = jobsList.index(i, 0);
        QVERIFY( index.data(GCF::JobListModel::Kind).toString() == job->kind() );
        QVERIFY( index.data(GCF::JobListModel::Title).toString() == job->title() );
        QVERIFY( index.data(GCF::JobListModel::Description).toString() == job->description() );
        QVERIFY( index.data(GCF::JobListModel::IconUrl).toString() == job->iconUrl() );
        QVERIFY( index.data(GCF::JobListModel::Progress).toInt() == job->progress() );
        QVERIFY( index.data(GCF::JobListModel::Status).toString() == job->status() );
        QVERIFY( index.data(GCF::JobListModel::IsStarted).toBool() == job->isStarted() );
        QVERIFY( index.data(GCF::JobListModel::IsSuspended).toBool() == job->isSuspended() );
        QVERIFY( index.data(GCF::JobListModel::IsComplete).toBool() == job->isComplete() );
        QVERIFY( index.data(GCF::JobListModel::HasError).toBool() == job->hasError() );
        QVERIFY( index.data(GCF::JobListModel::Error).toString() == job->error() );
        QVERIFY( index.data(GCF::JobListModel::Object).value<QObject*>() == job );
    }

    // We should have received 10 of jobCountChanged(), jobStarted(), jobCompleted()
    // jobSuspended() and jobResumed() signals.
    //
    // And 20+ jobUpdated() signals.
    QVERIFY( spyMap["jobCountChanged"]->count() == 10 );
    QVERIFY( spyMap["jobStarted"]->count() == 10 );
    QVERIFY( spyMap["jobCompleted"]->count() == 10 );
    QVERIFY( spyMap["jobSuspended"]->count() == 10 );
    QVERIFY( spyMap["jobResumed"]->count() == 10 );
    QVERIFY( spyMap["jobUpdated"]->count() >= 20 );

    qDeleteAll(jobs);

    QVERIFY(jobsList.rowCount() == 0);
    qDeleteAll(spyMap.values());
}

void JobListTest::testGlobalJobsList()
{
    GCF::JobListModel jobsList;
    QList<GCF::AbstractJob*> jobs;
    for(int i=0; i<10; i++)
    {
        SimpleJob *job = new SimpleJob;
        job->setDuration(2500+i*10);
        job->setTitle( QString("Title %1").arg(i+1) );
        job->setDescription( QString("Desc %1").arg(i+1) );
        job->setIconUrl( QString("http://www.sample.com/icon%1.png").arg(i+1) );
        job->setStatus( QString("status %1").arg(i+1) );
        jobs.append(job);
        jobsList.addJob(job);
        QTimer::singleShot(200, job, SLOT(suspendSelf()));
        QTimer::singleShot(350, job, SLOT(resumeSelf()));
        job->start();
        job->setProgress(20+3*i, QString("Progress %1").arg(i+1));
    }

    // Contents of jobsList and the global list should be one and the same.
    GCF::JobListModel *globalJobsList = gAppService->jobs();
    QVERIFY(globalJobsList->rowCount() == jobsList.rowCount());
    QVERIFY(globalJobsList->columnCount() == jobsList.columnCount());

    for(int i=0; i<globalJobsList->rowCount(); i++)
    {
        QModelIndex gIndex = globalJobsList->index(i, 0);
        QModelIndex index = jobsList.index(i, 0);
        QVERIFY(gIndex.model()->itemData(gIndex) == index.model()->itemData(gIndex));
    }

    qDeleteAll(jobs);

    QVERIFY(jobsList.rowCount() == 0);
    QVERIFY(globalJobsList->rowCount() == jobsList.rowCount());
}

class TemplateJob : public GCF::AbstractJob
{
public:
    TemplateJob(const QString &title, const QString &kind, QObject *parent=0)
        : GCF::AbstractJob(kind, parent) {
        this->setTitle(title);
    }
    ~TemplateJob() { }

    GCF::Result startJob() {
        this->done();
        return true;
    }
};

void JobListTest::testAddJobOrder()
{
    GCF::JobListModel jobList;

    const QStringList kinds = QStringList()
            << "Fruits" << "Vegetables" << "Infation" << "Corruption";
    const QList<int> insertOrder = QList<int>()  << 0 << 1 << 2
                                                 << 3 << 0 << 2
                                                 << 4 << 6 << 0
                                                 << 3 << 6 << 9
                                                 << 0 << 4 << 8
                                                 << 12 << 0 << 5
                                                 << 10 << 15;

    GCF::SignalSpy spy(&jobList, SIGNAL(rowsInserted(QModelIndex,int,int)));
    for(int i=0; i<20; i++)
    {
        QString title = QString("Job%1").arg(i+1);
        QString kind = kinds.at( i%kinds.count() );
        TemplateJob *job = new TemplateJob(title, kind, &jobList);
        jobList.addJob(job);

        QVERIFY(spy.count() == 1);
        QVERIFY(spy.last().at(1).toInt() == insertOrder.at(i));
        QVERIFY(spy.last().at(2).toInt() == insertOrder.at(i));
        spy.clear();
    }

    const QStringList order = QStringList() << "Job17" << "Job13" << "Job9"
                                            << "Job5" << "Job1" << "Job18"
                                            << "Job14" << "Job10" << "Job6"
                                            << "Job2" << "Job19" << "Job15"
                                            << "Job11" << "Job7" << "Job3"
                                            << "Job20" << "Job16" << "Job12"
                                            << "Job8" << "Job4";
    for(int i=0; i<jobList.jobCount(); i++)
        QVERIFY(jobList.jobAt(i)->title() == order.at(i));

    jobList.clearCompletedJobs();
}

int main(int argc, char *argv[])
{
    GCF::GuiApplication app(argc, argv);
    JobListTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_JobListTest.moc"
