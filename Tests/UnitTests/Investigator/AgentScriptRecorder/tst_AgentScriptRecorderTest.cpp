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
#include <QCoreApplication>

#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/Component>
#include <GCF3/SignalSpy>
#include <GCF3/GuiApplication>

#include <QPushButton>
#include "ui_ControlsWindow.h"
#include "ui_MainWindow.h"

static QString OutputFileName()
{
    static QString ret = QString("%1/output").arg(GCF::applicationDataDirectoryPath());
    return ret;
}

class AgentScriptRecorderTest : public QObject
{
    Q_OBJECT
    
public:
    AgentScriptRecorderTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testComponentLoad();
    void testObjectPathEvaluation();

private:
    QString logFileContents(bool deleteFile=true) const;

private:
    bool m_agentIsAvailable;
    GCF::Component *m_agentComponent;
};

AgentScriptRecorderTest::AgentScriptRecorderTest()
{
}

void AgentScriptRecorderTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void AgentScriptRecorderTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void AgentScriptRecorderTest::cleanup()
{
    if(m_agentComponent)
        m_agentComponent->unload();
    m_agentComponent = 0;

    QFile::remove( GCF::Log::instance()->logFileName() );
}

void AgentScriptRecorderTest::testComponentLoad()
{
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);
    QVERIFY(m_agentComponent->metaObject()->indexOfMethod("objectPath(QObject*)") >= 0);
    QVERIFY(m_agentComponent->metaObject()->indexOfMethod("object(QString)") >= 0);
    m_agentComponent->unload();
    m_agentComponent = 0;

    m_agentIsAvailable = true;
}

void AgentScriptRecorderTest::testObjectPathEvaluation()
{
    QVERIFY(m_agentIsAvailable);

    QWidget window;
    Ui::ControlsWindow ui;
    ui.setupUi(&window);
    window.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&window);
#else
    QTest::qWaitForWindowShown(&window);
#endif

    new GCF::ObjectTreeNode(gAppService->objectTree()->rootNode(), "Window", &window);

    // Load the agent component
    m_agentComponent = gApp->loadComponent("Investigator/Agent");

    // Get a list of all children
    QObjectList children = window.findChildren<QObject*>();
    Q_FOREACH(QObject *child, children)
    {
        QString childPath;
        bool success = QMetaObject::invokeMethod(m_agentComponent, "objectPath",
                                                 Q_RETURN_ARG(QString,childPath),
                                                 Q_ARG(QObject*,child));
        QVERIFY(success);
        QVERIFY(!childPath.isEmpty());

        QObject *object = 0;
        success = QMetaObject::invokeMethod(m_agentComponent, "object",
                                            Q_RETURN_ARG(QObject*,object),
                                            Q_ARG(QString,childPath));

        QVERIFY(success);
        QVERIFY(object == child);
    }

    delete gAppService->objectTree()->node("Application.Window");
}

QString AgentScriptRecorderTest::logFileContents(bool deleteFile) const
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
    GCF::GuiApplication app(argc, argv);

    QStringList args;
    args << QString("--outputFile:") + ::OutputFileName();
    args << QString("--testMode");
    app.processArguments(args);

    AgentScriptRecorderTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_AgentScriptRecorderTest.moc"
