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
#include <QProcess>
#include <QThread>
#include <QNetworkInterface>

#include <GCF3/Application>
#include <GCF3/Version>
#include <GCF3/Log>
#include <GCF3/IpcCall>
#include <GCF3/SignalSpy>
#include <GCF3/IpcServerDiscovery>

struct ServerInfo
{
    ServerInfo() : Port(0) { }

    QProcess *Process;
    quint16 Port;
};

class IpcServerDiscoveryTest : public QObject
{
    Q_OBJECT
    
public:
    IpcServerDiscoveryTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testConstructor();
    void testDiscovery();
    void testStartAgain();
    void testStopAgain();

private:
    QDir helperDirectory() const;
    quint16 launchServer(quint16 port=0);
    void killAllServers();

private:
    QList<ServerInfo> m_servers;
};

IpcServerDiscoveryTest::IpcServerDiscoveryTest()
{
}

void IpcServerDiscoveryTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void IpcServerDiscoveryTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
    // qDebug() << "\n" << this->logFileContents();
}

void IpcServerDiscoveryTest::cleanup()
{
    this->killAllServers();
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void IpcServerDiscoveryTest::testConstructor()
{
    GCF::IpcServerDiscovery discovery;
    qDebug() << discovery.user();
    QVERIFY(discovery.user().isEmpty() == false);

    // Ensure that no servers are discovered, when no server is running.
    GCF::SignalSpy spy(&discovery, SIGNAL(foundServer(GCF::IpcServerInfo)));
    discovery.start();
    spy.wait(2000);
    QVERIFY(spy.count() == 0);
    QVERIFY(discovery.foundServers().count() == 0);
}

void IpcServerDiscoveryTest::testDiscovery()
{
    // Launch some 10 server processes
    QList<quint16> serverPorts;
    for(int i=0; i<10; i++)
    {
        quint16 port = quint16(12345+i);
        if( this->launchServer(port) == port )
            serverPorts.append(port);
    }
    QVERIFY2(serverPorts.count() > 0, "Could not even launch a single server process");

    // Lets see if discovery discovers all the new server processes
    GCF::IpcServerDiscovery discovery;
    GCF::Result result = discovery.start();
    if(result.isSuccess() == false)
        qDebug() << result;
    QVERIFY(result.isSuccess() == true);

    GCF::SignalSpy spy(&discovery, SIGNAL(foundServer(GCF::IpcServerInfo)));
    QTest::qWait(5000);

    QVERIFY2(spy.count() >= serverPorts.count(), "Servers were not discovered");

    QList<quint16> foundPorts;
    QList<GCF::IpcServerInfo> serverInfoSet;
    for(int i=0; i<spy.count(); i++)
    {
        QVariantList args = spy.at(i);
        QVERIFY(args.count() == 1);

        GCF::IpcServerInfo info = args.first().value<GCF::IpcServerInfo>();
        QVERIFY(info.Port > 0);
        if(!foundPorts.contains(info.Port))
            foundPorts.append(info.Port);

        QVERIFY(info.User == discovery.user());
        QVERIFY(QNetworkInterface::allAddresses().contains(info.Address));
        QVERIFY(serverPorts.contains(info.Port));

        if(serverInfoSet.contains(info))
            qDebug() << "Duplicate: " << info;
        else
            serverInfoSet.append(info);
    }

    // This ensures that we discovered server ports of only those servers
    // that we explicitly created in this program.
    qSort(foundPorts);
    QVERIFY2(serverPorts == foundPorts, "Not all servers were discovered");

    // This verifies that foundServer() did not emit twice for the same
    // server-info
    QVERIFY2(serverInfoSet.count() == spy.count(), "Duplicate foundServer() emissions detected");
}

void IpcServerDiscoveryTest::testStartAgain()
{
    GCF::IpcServerDiscovery discovery;
    QVERIFY(discovery.start().isSuccess() == true);

    GCF::Result result = discovery.start();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Discovery has already been started");
}

void IpcServerDiscoveryTest::testStopAgain()
{
    GCF::IpcServerDiscovery discovery;
    GCF::Result result;

    result = discovery.stop();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Discovery cannot be stopped as it has not yet been started");

    result = discovery.start();
    QVERIFY(result.isSuccess() == true);

    result = discovery.stop();
    QVERIFY(result.isSuccess() == true);

    result = discovery.stop();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Discovery cannot be stopped as it has not yet been started");
}

QDir IpcServerDiscoveryTest::helperDirectory() const
{
    QDir dir = QDir::current();
    dir.cdUp();
    dir.cd("Helpers");
    return dir;
}

quint16 IpcServerDiscoveryTest::launchServer(quint16 port)
{
    // Check if a server has already been launched on this port.
    for(int i=0; i<this->m_servers.count(); i++)
    {
        ServerInfo info = this->m_servers.at(i);
        if(info.Port == port)
        {
            if(info.Process->state() == QProcess::Running)
                return port;

            delete info.Process;
            this->m_servers.removeAt(i);
            break;
        }
    }

#ifdef Q_OS_WIN
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp.exe");
#else
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp");
#endif

    if( !QFile::exists(appFile) )
        return false;

    QProcess *proc = new QProcess(this);
    QStringList args;
    args << "--serverPort:" + QString::number(port);
    args << "--enableDiscovery";

    proc->start(appFile, args);
    if(proc->waitForStarted())
    {
        QTest::qWait(100); // for the server process to start listening.
        if(proc->state() != QProcess::Running)
        {
            qDebug() << proc->readAllStandardError().simplified();
            delete proc;
            proc = 0;
        }
    }
    else
    {
        delete proc;
        proc = 0;
    }

    if(!proc)
        return 0;

    ServerInfo info;
    info.Process = proc;
    info.Port = port;
    this->m_servers.append(info);

    return port;
}

void IpcServerDiscoveryTest::killAllServers()
{
    while(this->m_servers.count())
    {
        ServerInfo info = this->m_servers.takeFirst();
        GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, info.Port,
                                              "Application.TestService", "terminate",
                                              QVariantList());
        GCF::SignalSpy spy(call, SIGNAL(done(bool)));
        if(spy.wait())
        {
            if( !info.Process->waitForFinished() )
                info.Process->terminate();
        }
        else
            info.Process->terminate();

        delete info.Process;
    }

    this->m_servers.clear();
}

int main(int argc, char *argv[])
{
    GCF::Application app(argc, argv);
    IpcServerDiscoveryTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_IpcServerDiscoveryTest.moc"
