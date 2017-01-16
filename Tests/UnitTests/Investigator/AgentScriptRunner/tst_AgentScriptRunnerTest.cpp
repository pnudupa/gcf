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
#include <QBasicTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QCalendarWidget>
#include <QDial>
#include <QGraphicsProxyWidget>

#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/Component>
#include <GCF3/SignalSpy>
#include <GCF3/GuiApplication>

#include "../Investigator/Agent/Agent.h"
#include "ui_MainWindow.h"
#include "ui_ControlsWindow.h"
#include "GraphicsView.h"
#include "ItemViewsForm.h"

static QString OutputFileName()
{
    static QString ret = QString("%1/output").arg(GCF::applicationDataDirectoryPath());
    return ret;
}

static void AppendString(QStringList &list, const QString &str)
{
    if(list.count())
    {
        if(list.last() != str)
            list.append(str);
    }
    else
        list.append(str);
}

class ControlsWindow : public QWidget
{
    Q_OBJECT

private:
    Ui::ControlsWindow m_ui;
    QStringList m_strings;

public:
    ControlsWindow(QWidget *parent=0) : QWidget(parent) {
        m_ui.setupUi(this);
        ::AppendString(m_strings, "ControlsWindow");
    }
    Q_INVOKABLE void addString(const QString &str) { ::AppendString(m_strings, str); }
    QStringList strings() { return m_strings; }
    void showEvent(QShowEvent *) { ::AppendString(m_strings, "showEvent"); }
};

class SignalEmitter : public QObject
{
    Q_OBJECT

public:
    SignalEmitter(QObject *parent=0) : QObject(parent) { }
    Q_INVOKABLE void emitSignal(int after, const QString &value) {
        m_string = value;
        m_timer.stop();
        if(after > 0)
            m_timer.start(after, this);
        else {
            emit signal(m_string);
            m_string.clear();
        }
    }

    Q_PROPERTY(QString string READ string WRITE setString NOTIFY stringChanged)
    void setString(const QString &str) {
        m_string = str;
        QMetaObject::invokeMethod(this, "stringChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString,m_string));
    }
    QString string() const { return m_string; }
    Q_SIGNAL void stringChanged(const QString &string);

    // Property without notification
    Q_PROPERTY(int integer READ integer WRITE setInteger)
    void setInteger(int val) { m_integer = val; }
    int integer() const { return m_integer; }

signals:
    void signal(const QString &value);

private:
    void timerEvent(QTimerEvent *) {
        m_timer.stop();
        emit signal(m_string);
        m_string.clear();
    }

private:
    QString m_string;
    int m_integer;
    QBasicTimer m_timer;
};

static QString QDebugMessage;
#if QT_VERSION >= 0x050000
void qDebugMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    if(msg.isEmpty()) return;
    QDebugMessage = msg.trimmed();
}
#else
void qDebugMessageHandler(QtMsgType, const char *msg)
{
    if(!msg || *msg == '\0') return;
    QDebugMessage = QString::fromLatin1(msg).trimmed();
}
#endif

class Widget : public QWidget
{
public:
    Widget(QWidget *parent=0) : QWidget(parent) {
        QComboBox *comboBox = new QComboBox(this);
        comboBox->move(5, 5);
        for(int i=0; i<10; i++)
            comboBox->addItem( QString("item %1").arg(i) );
    }
    ~Widget() { }

    QStringList events() const { return m_events; }
    void clearEvents() { m_events.clear(); }

protected:
    bool event(QEvent *e) {
#if QT_VERSION >= 0x050000
        QtMessageHandler oldHandler = qInstallMessageHandler(qDebugMessageHandler);
#else
        QtMsgHandler oldHandler = qInstallMsgHandler(qDebugMessageHandler);
#endif
        QDebugMessage.clear();
        bool success = QWidget::event(e);
        if(!QDebugMessage.isEmpty())
            ::AppendString(m_events, ::QDebugMessage);
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(oldHandler);
#else
        qInstallMsgHandler(oldHandler);
#endif
        return success;
    }

    void keyPressEvent(QKeyEvent *ke) { qDebug() << ke; }
    void keyReleaseEvent(QKeyEvent *ke) { qDebug() << ke; }
    void mousePressEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseMoveEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseReleaseEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseDoubleClickEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void showEvent(QShowEvent *) { qDebug() << "QShowEvent"; }
    void hideEvent(QHideEvent *) { qDebug() << "QHideEvent"; }
    void moveEvent(QMoveEvent *me) { qDebug() << "QMoveEvent" << me->pos(); }
    void resizeEvent(QResizeEvent *re) { qDebug() << "QResizeEvent" << re->size(); }
    void wheelEvent(QWheelEvent *we) { qDebug() << we->pos() << "QWheelEvent(" << we->delta() << ")"; }

private:
    QStringList m_events;
};

#if QT_VERSION >= 0x050000
class Window : public QWindow
{
public:
    Window() { }
    ~Window() { }

    QStringList events() const { return m_events; }
    void clearEvents() { m_events.clear(); }

protected:
    bool event(QEvent *e) {
#if QT_VERSION >= 0x050000
        QtMessageHandler oldHandler = qInstallMessageHandler(qDebugMessageHandler);
#else
        QtMsgHandler oldHandler = qInstallMsgHandler(qDebugMessageHandler);
#endif
        QDebugMessage.clear();
        bool success = QWindow::event(e);
        if(!QDebugMessage.isEmpty())
            ::AppendString(m_events, ::QDebugMessage);
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(oldHandler);
#else
        qInstallMsgHandler(oldHandler);
#endif
        return success;
    }

    void keyPressEvent(QKeyEvent *ke) { qDebug() << ke; }
    void keyReleaseEvent(QKeyEvent *ke) { qDebug() << ke; }
    void mousePressEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseMoveEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseReleaseEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void mouseDoubleClickEvent(QMouseEvent *me) { qDebug() << me->pos() << me; }
    void showEvent(QShowEvent *) { qDebug() << "QShowEvent"; }
    void hideEvent(QHideEvent *) { qDebug() << "QHideEvent"; }
    void moveEvent(QMoveEvent *me) { qDebug() << "QMoveEvent" << me->pos(); }
    void resizeEvent(QResizeEvent *re) { qDebug() << "QResizeEvent" << re->size(); }
    void wheelEvent(QWheelEvent *we) { qDebug() << we->pos() << "QWheelEvent(" << we->delta() << ")"; }

private:
    QStringList m_events;
};
#endif

class AgentScriptRunnerTest : public QObject
{
    Q_OBJECT
    
public:
    AgentScriptRunnerTest();
    
    Q_INVOKABLE void addString(const QString &string) {
        m_strings.append(string);
    }

    Q_INVOKABLE void mapObject(const QString &name, QObject *object) {
        m_objectMap[name] = object;
    }

    Q_INVOKABLE void waitStart() { m_timer.start(); }
    Q_INVOKABLE int waitEnd() { return m_timer.elapsed(); }

    Q_INVOKABLE QWidget *newControlsWindow() {
        m_controlWindows.append(new ControlsWindow);
        return m_controlWindows.last();
    }

    Q_INVOKABLE QObject *signalEmitter() { return &m_signalEmitter; }

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testComponentLoad();
    void testRunScriptErrors_data();
    void testRunScriptErrors();
    void testFunctionCallOrder();
    void testLog();
    void testFail();
    void testObject();
    void testInterface();
    void testWait();
    void testWaitForWindowShown();
    void testWaitForWindowActive();
    void testWaitForSignal();
    void testWaitForPropertyChange();
    void testVerify();
    void testCompare();
    void testSignalSpy();
    void testObjectWrap();
    void testWidgetWrap();
    void testComboBoxWrap();
    void testFindQmlContextProperty();
    void testFindGraphicsObject();
    void testGraphicsObjectWrap();
    void testWindowWrap();
    void testScrollAreaAndItemViewParts();

private:
    void compareObjectMap(QMap<QString,QObject*>& map1, QMap<QString,QObject*>& map2);
    QString testOutput() const;
    QString testScriptFile(const QString &fileName) const {
        QString fName = QString("../../../Tests/UnitTests/Investigator/AgentScriptRunner/");
        fName += fileName;
        return QFileInfo(fName).absoluteFilePath();
    }
    QString logFileContents(bool deleteFile=true) const;

private:
    bool m_agentIsAvailable;
    QStringList m_strings;
    GCF::Component *m_agentComponent;
    QMap<QString,QObject*> m_objectMap;
    QElapsedTimer m_timer;
    QList<ControlsWindow*> m_controlWindows;
    SignalEmitter m_signalEmitter;
};

AgentScriptRunnerTest::AgentScriptRunnerTest()
    : m_agentIsAvailable(false), m_agentComponent(0)
{
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "tst_AgentScriptRunnerTest", this);
}

void AgentScriptRunnerTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void AgentScriptRunnerTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void AgentScriptRunnerTest::cleanup()
{
    if(m_agentComponent)
        m_agentComponent->unload();
    m_agentComponent = 0;
    m_strings.clear();
    m_objectMap.clear();
    qDeleteAll(m_controlWindows);
    m_controlWindows.clear();

    QFile::remove( GCF::Log::instance()->logFileName() );
}

void AgentScriptRunnerTest::testComponentLoad()
{
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);
    QVERIFY(m_agentComponent->metaObject()->indexOfMethod("runScript(QString)") >= 0);
    m_agentComponent->unload();
    m_agentComponent = 0;

    QString output = this->testOutput();
    QVERIFY(output.length() > 0);
    QVERIFY(output.startsWith("********* Start testing of tst_AgentScriptRunnerTest *********"));
    QVERIFY(output.endsWith("********* Finished testing of tst_AgentScriptRunnerTest *********\n"));

    m_agentIsAvailable = true;
}

void AgentScriptRunnerTest::testRunScriptErrors_data()
{
    QTest::addColumn<QString>("testScriptFile");
    QTest::addColumn<QString>("errorMessage");

    QTest::newRow("empty file name") << QString() << "FAILURE: No script file name provided";

    QString fName = this->testScriptFile("DoesntExist.js");
    QTest::newRow("non existant file") << fName << QString("FAILURE: Cannot open script file '%1' for reading").arg(fName);

    fName = this->testScriptFile("EmptyTestScript.js");
    QTest::newRow("empty script file") << fName << QString("FAILURE: Script file '%1' was empty").arg(fName);

    fName = this->testScriptFile("NoTestFunctions.js");
    QTest::newRow("script with no test functions") << fName << QString("FAILURE: No test_...() functions were found in the script");

    fName = this->testScriptFile("SyntaxErrorScript.js");
    QTest::newRow("script with syntax error") << fName << QString("FAILURE: Error at line 3: SyntaxError: Parse error");
}

void AgentScriptRunnerTest::testRunScriptErrors()
{
    QFETCH(QString, testScriptFile);
    QFETCH(QString, errorMessage);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript",
                                            QVariantList() << testScriptFile);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == errorMessage);
}

void AgentScriptRunnerTest::testFunctionCallOrder()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("FunctionCallOrderTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QVERIFY(m_strings.count() == 8);
    QVERIFY(m_strings.at(0) == "initTestCase");
    QVERIFY(m_strings.at(1) == "init");
    QVERIFY(m_strings.at(2) == "test_case1");
    QVERIFY(m_strings.at(3) == "cleanup");
    QVERIFY(m_strings.at(4) == "init");
    QVERIFY(m_strings.at(5) == "test_case2");
    QVERIFY(m_strings.at(6) == "cleanup");
    QVERIFY(m_strings.at(7) == "cleanupTestCase");
}

void AgentScriptRunnerTest::testLog()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("LogTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);
    QString output("test_log {\n"
                   "    logging from test script\n"
                   "}\n");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testFail()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("FailTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "FAILURE: 1 of 1 test case(s) failed");
    QString output("test_fail {\n"
                   "    log before fail\n"
                   "    FAILED at line 3: fail message\n"
                   "}\n");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testObject()
{
    // Create two windows
    QMainWindow mainWindow;
    Ui::MainWindow mainWindowUi;
    mainWindowUi.setupUi(&mainWindow);
    mainWindow.show();

    QWidget controlsWindow;
    Ui::ControlsWindow controlsWindowUi;
    controlsWindowUi.setupUi(&controlsWindow);
    controlsWindow.show();

#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&mainWindow);
    QTest::qWaitForWindowActive(&controlsWindow);
#else
    QTest::qWaitForWindowShown(&mainWindow);
    QTest::qWaitForWindowShown(&controlsWindow);
#endif

    // Insert the windows into the object tree
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "MainWindow", &mainWindow);
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "ControlsWindow", &controlsWindow);

    // Now perform the tests
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("ObjectTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    delete gApp->objectTree()->node("Application.MainWindow");
    delete gApp->objectTree()->node("Application.ControlsWindow");

    // Now check results
    QMap<QString,QObject*> expectedMap;
    expectedMap["Application.MainWindow"] = &mainWindow;
    expectedMap["Application.ControlsWindow"] = &controlsWindow;
    expectedMap["Application.tst_AgentScriptRunnerTest"] = this;
    expectedMap["Application.MainWindow/treeView"] = mainWindowUi.treeView;
    expectedMap["Application.MainWindow/textEdit1"] = mainWindowUi.textEdit1;
    expectedMap["Application.ControlsWindow/QCheckBox[0]"] = controlsWindowUi.checkBox;
    expectedMap["Application.MainWindow/QMenuBar[0]"] = mainWindowUi.menubar;
    expectedMap["Application.NonExistentObject"] = 0;

    this->compareObjectMap(expectedMap, m_objectMap);
}

void AgentScriptRunnerTest::testInterface()
{
    // Create two windows
    QMainWindow mainWindow;
    Ui::MainWindow mainWindowUi;
    mainWindowUi.setupUi(&mainWindow);
    mainWindow.show();

    QWidget controlsWindow;
    Ui::ControlsWindow controlsWindowUi;
    controlsWindowUi.setupUi(&controlsWindow);
    controlsWindow.show();

#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&mainWindow);
    QTest::qWaitForWindowActive(&controlsWindow);
#else
    QTest::qWaitForWindowShown(&mainWindow);
    QTest::qWaitForWindowShown(&controlsWindow);
#endif

    // Insert the windows into the object tree
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "MainWindow", &mainWindow);
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "ControlsWindow", &controlsWindow);

    // Now perform the tests
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("InterfaceTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    delete gApp->objectTree()->node("Application.MainWindow");
    delete gApp->objectTree()->node("Application.ControlsWindow");

    // Now check results
    QMap<QString,QObject*> expectedMap;
    expectedMap["QMainWindow"] = &mainWindow;
    expectedMap["QTreeView"] = 0;
    expectedMap["QCheckBox"] = 0;
    expectedMap["GCF::IGDriveLite"] = 0;

    this->compareObjectMap(expectedMap, m_objectMap);
}

void AgentScriptRunnerTest::testWait()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("WaitTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QVERIFY(m_strings.count() == 1);
    QVERIFY(m_strings.at(0).toInt() >= 500);
}

void AgentScriptRunnerTest::testWaitForWindowShown()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("WaitForWindowShownTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QVERIFY(m_controlWindows.count() == 1);

    ControlsWindow *window = m_controlWindows.first();
    QVERIFY(window->strings().count() == 4);

    QStringList expectedStrings;
    expectedStrings << "ControlsWindow";
    expectedStrings << "before window show";
    expectedStrings << "showEvent";
    expectedStrings << "after window show";

    QVERIFY(expectedStrings == window->strings());
}

void AgentScriptRunnerTest::testWaitForWindowActive()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("WaitForWindowActiveTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QVERIFY(m_controlWindows.count() == 1);

    ControlsWindow *window = m_controlWindows.first();
    QVERIFY(window->strings().count() == 4);

    QStringList expectedStrings;
    expectedStrings << "ControlsWindow";
    expectedStrings << "before window active";
    expectedStrings << "showEvent";
    expectedStrings << "after window active";

    QVERIFY(expectedStrings == window->strings());
}

void AgentScriptRunnerTest::testWaitForSignal()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    GCF::SignalSpy spy(&m_signalEmitter, SIGNAL(signal(QString)));

    QString fName = this->testScriptFile("WaitForSignalTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QVERIFY(spy.count() == 1);
    spy.wait();
    QVERIFY(spy.count() == 2);

    QVERIFY(this->testOutput().contains("Signal 'invalidSignal(QVariant)' doesnt exist"));
}

void AgentScriptRunnerTest::testWaitForPropertyChange()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("WaitForPropertyChangeTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == true);

    QString output = this->testOutput();
    QVERIFY(output.contains("Property 'unknownProperty' doesnt exist"));
    QVERIFY(output.contains("Property 'integer' doesnt have a notify signal"));
}

void AgentScriptRunnerTest::testVerify()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("VerifyTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == false);

    QString output("test_verify {\n"
                   "    verify #1\n"
                   "    verify #2\n"
                   "    FAILED at line 5: verify() failed\n"
                   "}");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testCompare()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("CompareTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    QVERIFY(result.isSuccess() == false);

    QString output("test_compare {\n"
                   "    compare #1\n"
                   "    compare #2\n"
                   "    FAILED at line 5: compare() failed\n"
                   "}");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testSignalSpy()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("SignalSpyTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);

    QVERIFY(result.isSuccess() == true);
    QString output("test_signalSpy {\n"
                   "    Signal 'invalidSignal(QVariant)' doesnt exist\n"
                   "}");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testObjectWrap()
{
    QWidget controlsWindow;
    Ui::ControlsWindow controlsWindowUi;
    controlsWindowUi.setupUi(&controlsWindow);
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "ControlsWindow", &controlsWindow);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("ObjectWrapTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.ControlsWindow");

    QVERIFY(result.isSuccess() == true);
    QString output("test_objectWrap {\n"
                   "    calendarWidget\n"
                   "    tab2\n"
                   "    qt_tabwidget_stackedwidget\n"
                   "    tabWidget\n"
                   "    calendarWidget\n"
                   "}");
    QVERIFY(this->testOutput().contains(output));
}

void AgentScriptRunnerTest::testWidgetWrap()
{
    Widget window;
    window.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&window);
#else
    QTest::qWaitForWindowShown(&window);
#endif
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "Window", &window);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    window.clearEvents();

    QString fName = this->testScriptFile("WidgetWrapTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.Window");

    QStringList expectedEvents;
    expectedEvents << "QResizeEvent QSize(320, 240)";
    expectedEvents << "QResizeEvent QSize(640, 480)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonPress, 1, 1, 2000000)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonRelease, 1, 0, 2000000)";
    expectedEvents << "QPoint(10,10) QMouseEvent(MouseButtonPress, 1, 1, 0)";
    expectedEvents << "QPoint(40,20) QMouseEvent(MouseButtonRelease, 1, 0, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonPress, 1, 1, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonRelease, 1, 0, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonDblClick, 1, 1, 0)";
    expectedEvents << "QPoint(100,50) QMouseEvent(MouseButtonDblClick, 1, 1, 8000000)";
    expectedEvents << "QKeyEvent(KeyPress, 41, 0, \"\"a\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 41, 0, \"\"a\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 42, 0, \"\"b\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 42, 0, \"\"b\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 43, 0, \"\"c\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 43, 0, \"\"c\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 44, 0, \"\"d\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 44, 0, \"\"d\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 41, 2000000, \"\"a\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 41, 2000000, \"\"a\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 42, 2000000, \"\"b\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 42, 2000000, \"\"b\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 43, 2000000, \"\"c\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 43, 2000000, \"\"c\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 44, 2000000, \"\"d\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 44, 2000000, \"\"d\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 45, 2000000, \"\"e\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 45, 2000000, \"\"e\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyPress, 46, 2000000, \"\"f\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 46, 2000000, \"\"f\"\", false, 1)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, 1)";
    expectedEvents << "QPoint(319,239) QWheelEvent( -1 )";
    expectedEvents << "QPoint(50,100) QWheelEvent( 1 )";

    QVERIFY(result.isSuccess() == true);

    bool success = true;
    QStringList windowEvents = window.events();
    while(expectedEvents.count())
    {
        QString evt = expectedEvents.takeFirst();
        while(windowEvents.count())
        {
            if(windowEvents.first() == evt)
                break;
            windowEvents.takeFirst();
        }
        if(windowEvents.count() && windowEvents.first() == evt)
        {
            windowEvents.takeFirst();
            continue;
        }

        success = false;
        break;
    }

    QVERIFY(success);
}

void AgentScriptRunnerTest::testComboBoxWrap()
{
    Widget window;
    window.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&window);
#else
    QTest::qWaitForWindowShown(&window);
#endif
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "Window", &window);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("ComboBoxTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.Window");

    QVERIFY(result.isSuccess() == true);

    QStringList expectedStrings;
    expectedStrings << "10" << "item 3" << "8";
    QVERIFY(expectedStrings == m_strings);
}

void AgentScriptRunnerTest::testFindQmlContextProperty()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");

#if QT_VERSION >= 0x050000
    QQmlEngine qmlEngine;
#else
    QDeclarativeEngine qmlEngine;
#endif

    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "QmlEngine", &qmlEngine);

    QObjectList objectList;
    for(int i=0; i<5; i++)
        objectList.append(new QObject(&qmlEngine));

    qmlEngine.rootContext()->setContextProperty("Object1", objectList.at(0));
    qmlEngine.rootContext()->setContextProperty("Object2", objectList.at(1));
    qmlEngine.rootContext()->setContextProperty("Object3", objectList.at(2));
    qmlEngine.rootContext()->setContextProperty("Object4", objectList.at(3));
    qmlEngine.rootContext()->setContextProperty("Object5", objectList.at(4));

    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("QmlContextPropertyTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.QmlEngine");

    QVERIFY(result.isSuccess() == true);

    QMap<QString,QObject*> expectedMap;
    expectedMap["Application.QmlEngine"] = &qmlEngine;
    expectedMap["Application.QmlEngine/Object1"] = objectList.at(0);
    expectedMap["Application.QmlEngine/Object2"] = objectList.at(1);
    expectedMap["Application.QmlEngine/Object3"] = objectList.at(2);
    expectedMap["Application.QmlEngine/Object4"] = objectList.at(3);
    expectedMap["Application.QmlEngine/Object5"] = objectList.at(4);

    this->compareObjectMap(expectedMap, m_objectMap);
}

void AgentScriptRunnerTest::testFindGraphicsObject()
{
    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");

    QGraphicsView gView;
    QGraphicsScene gScene;
    gView.setScene(&gScene);

    QList<QGraphicsObject*> gObjectList;

    QGraphicsObject *gObject1 = gScene.addWidget(new QCalendarWidget);
    gObject1->setObjectName("Calendar");
    gObjectList.append( gObject1 );

    QGraphicsObject *gObject2 = gScene.addWidget(new QDial);
    gObject2->setObjectName("Dial");
    gObjectList.append( gObject2 );

    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "GraphicsView", &gView);
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "GraphicsScene", &gScene);

    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("GraphicsObjectTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.GraphicsView");
    delete gApp->objectTree()->node("Application.GraphicsScene");

    QVERIFY(result.isSuccess() == true);

    QMap<QString,QObject*> expectedMap;
    expectedMap["Application.GraphicsView"] = &gView;
    expectedMap["Application.GraphicsScene"] = &gScene;
    expectedMap["Application.GraphicsView/Calendar"] = gObject1;
    expectedMap["Application.GraphicsView/Dial"] = gObject2;
    expectedMap["Application.GraphicsScene/Calendar"] = gObject1;
    expectedMap["Application.GraphicsScene/Dial"] = gObject2;

    this->compareObjectMap(expectedMap, m_objectMap);
}

void AgentScriptRunnerTest::testGraphicsObjectWrap()
{
#if 0
    GraphicsView gView;
    gView.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&gView);
#else
    QTest::qWaitForWindowShown(&gView);
#endif

    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "GraphicsView", &gView);

    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("GraphicsObjectWrapTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.GraphicsView");

    QVERIFY(result.isSuccess() == true);
#else
    // FIXME: #pragma warning("Fix 'testGraphicsObjectWrap()' test case! ")
#endif
}

void AgentScriptRunnerTest::testWindowWrap()
{
#if QT_VERSION >= 0x050000
    Window window;
    // To avoid "Mouse event occurs outside of target window." warning from QTest::mouseEvent()
    // We need to set the position explicitly.
    window.setPosition(QPoint(0,0));
    window.show();
    QTest::qWaitForWindowActive(&window);
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "Window", &window);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    window.clearEvents();

    QString fName = this->testScriptFile("WindowWrapTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.Window");

    QStringList expectedEvents;
#if 0
    expectedEvents << "QResizeEvent QSize(320, 240)";
    expectedEvents << "QResizeEvent QSize(640, 480)";
#endif
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonPress, 1, 1, 2000000)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonRelease, 1, 0, 2000000)";
    expectedEvents << "QPoint(10,10) QMouseEvent(MouseButtonPress, 1, 1, 0)";
    expectedEvents << "QPoint(40,20) QMouseEvent(MouseButtonRelease, 1, 0, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonPress, 1, 1, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonRelease, 1, 0, 0)";
    expectedEvents << "QPoint(319,239) QMouseEvent(MouseButtonDblClick, 1, 1, 0)";
    expectedEvents << "QPoint(100,50) QMouseEvent(MouseButtonDblClick, 1, 1, 8000000)";
    expectedEvents << "QKeyEvent(KeyPress, 41, 0, \"\"a\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 41, 0, \"\"a\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 42, 0, \"\"b\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 42, 0, \"\"b\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 43, 0, \"\"c\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 43, 0, \"\"c\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 44, 0, \"\"d\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 44, 0, \"\"d\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 41, 2000000, \"\"a\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 41, 2000000, \"\"a\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 42, 2000000, \"\"b\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 42, 2000000, \"\"b\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 43, 2000000, \"\"c\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 43, 2000000, \"\"c\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 44, 2000000, \"\"d\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 44, 2000000, \"\"d\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 45, 2000000, \"\"e\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 45, 2000000, \"\"e\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 1000020, 2000000, \"\"\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyPress, 46, 2000000, \"\"f\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 46, 2000000, \"\"f\"\", false, ffff)";
    expectedEvents << "QKeyEvent(KeyRelease, 1000020, 0, \"\"\"\", false, ffff)";
    expectedEvents << "QPoint(319,239) QWheelEvent( -1 )";
    expectedEvents << "QPoint(50,100) QWheelEvent( 1 )";

    QVERIFY(result.isSuccess() == true);

    bool success = true;
    QStringList windowEvents = window.events();
    while(expectedEvents.count())
    {
        QString evt = expectedEvents.takeFirst();
        while(windowEvents.count())
        {
            if(windowEvents.first() == evt)
                break;
            windowEvents.takeFirst();
        }
        if(windowEvents.count() && windowEvents.first() == evt)
        {
            windowEvents.takeFirst();
            continue;
        }

        success = false;
        break;
    }

    QVERIFY(success);
#endif
}

void AgentScriptRunnerTest::testScrollAreaAndItemViewParts()
{
    ItemViewsForm itemViewsForm;
    itemViewsForm.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&itemViewsForm);
#else
    QTest::qWaitForWindowShown(&itemViewsForm);
#endif
    new GCF::ObjectTreeNode(gApp->objectTree()->rootNode(), "ItemViewsForm", &itemViewsForm);

    QVERIFY2(m_agentIsAvailable, "Agent component not available for testing");
    m_agentComponent = gApp->loadComponent("Investigator/Agent");
    QVERIFY(m_agentComponent != 0);

    QString fName = this->testScriptFile("ScrollAreaAndItemViewPartsTest.js");
    GCF::Result result = gApp->invokeMethod(m_agentComponent, "runScript", QVariantList() << fName);
    delete gApp->objectTree()->node("Application.ItemViewsForm");

    if(!result)
        qDebug() << "\n" << this->testOutput();

    QVERIFY(result.isSuccess() == true);
}

void AgentScriptRunnerTest::compareObjectMap(QMap<QString, QObject *> &map1, QMap<QString, QObject *> &map2)
{
    QVERIFY2(map1.count() == map2.count(), "ObjectTest.js and this test-function are not in sync");
    QMap<QString,QObject*>::const_iterator it = map1.begin();
    QMap<QString,QObject*>::const_iterator end = map1.end();
    int passCount = 0;
    while(it != end)
    {
        QObject *expectedObject = it.value();
        QObject *haveObject = map2.value(it.key(), 0);
        if(expectedObject == haveObject)
            ++passCount;
        else
        {
            if(expectedObject && haveObject)
                qDebug() << "For " << it.key() << ", expected: " << expectedObject << " but we have " << haveObject;
            else if(expectedObject && !haveObject)
                qDebug() << "For " << it.key() << ", expected: " << expectedObject << " but we have NULL";
            else if(!expectedObject && haveObject)
                qDebug() << "For " << it.key() << ", expected: NULL but we have " << haveObject;
        }
        ++it;
    }

    QVERIFY(passCount == map1.count());
}

QString AgentScriptRunnerTest::testOutput() const
{
    QFile file(::OutputFileName());
    if( !file.exists() )
        return QString();

    file.open(QFile::ReadOnly);
    QString bytes = file.readAll();
    file.close();

    QFile::remove(::OutputFileName());

    return bytes;
}

QString AgentScriptRunnerTest::logFileContents(bool deleteFile) const
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

    AgentScriptRunnerTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_AgentScriptRunnerTest.moc"
