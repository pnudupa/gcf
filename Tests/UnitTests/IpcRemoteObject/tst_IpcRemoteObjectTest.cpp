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
#include <QString>

#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/IpcCall>
#include <GCF3/SignalSpy>
#include <GCF3/Application>
#include <GCF3/IpcRemoteObject>

#include <QPointer>
#include <QProcess>
#include <QElapsedTimer>

class IpcRemoteObjectTest;
class SenderOrReceiver : public QObject
{
    Q_OBJECT

signals:
    void integerSignal(int v);
    void booleanSignal(bool v);
    void realSignal(double v);
    void stringSignal(QString v);
    void stringListSignal(QStringList v);
    void variantSignal(QVariant v);
    void variantListSignal(QVariantList v);
    void variantMapSignal(QVariantMap v);
    void byteArraySignal(QByteArray v);
    void allSignal(int intVal, bool boolVal, double doubleVal,
                   const QString &strVal, const QStringList &strListVal,
                   const QVariant &varVal, const QVariantList &varList,
                   const QVariantMap &varMap, const QByteArray &byteArr);

    void allSignal2(const QString &strVal, const QStringList &strListVal,
                    int intVal, bool boolVal, double doubleVal,
                    const QVariantMap &varMap, const QByteArray &byteArr,
                    const QVariant &varVal, const QVariantList &varList);
    void dateSignal(QDate v);

public:
    GCF::Result result;

public slots:
    void propertyNotificationWrong(const QVariant &) { }
    void propertyNotification(const QVariant &r, bool success, const QString &error) {
        this->result = GCF::Result(success, QString(), error, r);
    }
    void callNotification(const QVariant &r, bool success, const QString &error) {
        this->result = GCF::Result(success, QString(), error, r);
    }
    void callNotificationWrong(bool, const QVariant &, const QString &) { }

private:
    friend class IpcRemoteObjectTest;
};

const int ServerPort = 49772;
const int MaxSpinWaitTime = 10000; // 10 seconds

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

class IpcRemoteObjectTest : public QObject
{
    Q_OBJECT
    
public:
    IpcRemoteObjectTest() : m_remoteApp(0) { this->setupIpcServer(); }
    ~IpcRemoteObjectTest() { this->killIpcServer(); }

public slots:
    void onServerReadyRead() {
        qDebug() << m_remoteApp->readAllStandardError();
        qDebug() << m_remoteApp->readAllStandardOutput();
    }
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testActivation();
    void testMetaInformation();
    void testChangeAndUpdatePropertyMethods_data();
    void testChangeAndUpdatePropertyMethods();
    void testChangeAndUpdatePropertyErrors();
    void testConnect();
    void testConnect2();
    void testConnectErrors();
    void testCalls_data();
    void testCalls();
    void testCallErrors();
    void testDeactivation();
    void testDeactivation2();
    void testConnectionToNonExistingServer();

private:
    void setupIpcServer();
    void killIpcServer();
    QDir helperDirectory() const;
    QString logFileContents(bool deleteFile=true) const;

private:
    QProcess *m_remoteApp;
};

void IpcRemoteObjectTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void IpcRemoteObjectTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
    // qDebug() << "\n" << this->logFileContents();
}

void IpcRemoteObjectTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void IpcRemoteObjectTest::testActivation()
{
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    QVERIFY(remoteObject.isActivated() == false);
    QVERIFY(remoteObject.isActivated() == false);
    QVERIFY(remoteObject.properties().isEmpty());
    QVERIFY(remoteObject.signalMethods().isEmpty());
    QVERIFY(remoteObject.invokableMethods().isEmpty());

    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);

    QVERIFY(spy.count() == 1);
    QVERIFY(remoteObject.isActivated() == true);
    QVERIFY(!remoteObject.properties().isEmpty());
    QVERIFY(!remoteObject.signalMethods().isEmpty());
    QVERIFY(!remoteObject.invokableMethods().isEmpty());
}

void IpcRemoteObjectTest::testMetaInformation()
{
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    QVariantMap properties;
    properties["integer"] = 20;
    properties["boolean"] = false;
    properties["real"] = 123.45;
    properties["string"] = "Hello World";
    properties["stringList"] = QStringList() << "A" << "B" << "C" << "D";
    properties["variantList"] = QVariantList() << 10 << false << 123.45 << QDate::currentDate();
    properties["variant"] = QVariant(QDate::currentDate());
    QVariantMap map;
    map["January"] = 31;
    map["February"] = 28;
    map["March"] = 31;
    map["April"] = 30;
    map["May"] = 31;
    properties["variantMap"] = map;
    properties["byteArray"] = QByteArray(1024, 'A');
    properties["objectName"] = QString();

    QVERIFY(properties.keys() == remoteObject.properties().keys());
    QVariantMap::const_iterator it = properties.begin();
    QVariantMap::const_iterator end = properties.end();
    while(it != end)
    {
        QString msg = QString("Property value for %1 doesnt match").arg(it.key());
        QByteArray msgBa = msg.toLatin1();
        QVERIFY2(it.value() == remoteObject.properties().value(it.key()), msgBa);
        ++it;
    }

    QStringList signalMethods;
    signalMethods << "destroyed()";
#if QT_VERSION >= 0x050000
    signalMethods << "objectNameChanged(QString)";
#endif
    signalMethods << "integerSignal(int)";
    signalMethods << "booleanSignal(bool)";
    signalMethods << "realSignal(double)";
    signalMethods << "stringSignal(QString)";
    signalMethods << "stringListSignal(QStringList)";
    signalMethods << "variantSignal(QVariant)";
    signalMethods << "variantListSignal(QVariantList)";
    signalMethods << "variantMapSignal(QVariantMap)";
    signalMethods << "byteArraySignal(QByteArray)";
    signalMethods << "allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)";

    QVERIFY(remoteObject.signalMethods().count() == signalMethods.count());
    for(int i=0; i<signalMethods.count(); i++)
    {
        QString msg = QString("%1: Expected '%2' but found '%3'")
                .arg(i).arg(signalMethods.at(i)).arg(remoteObject.signalMethods().at(i));
        QByteArray msgBa = msg.toLatin1();
        QVERIFY2(remoteObject.signalMethods().at(i) == signalMethods.at(i), msgBa);
    }

    QStringList invokableMethods;
    invokableMethods << "destroyed()";
#if QT_VERSION >= 0x050000
    invokableMethods << "objectNameChanged(QString)";
#endif
    invokableMethods << "deleteLater()";
    invokableMethods << "integerSignal(int)";
    invokableMethods << "booleanSignal(bool)";
    invokableMethods << "realSignal(double)";
    invokableMethods << "stringSignal(QString)";
    invokableMethods << "stringListSignal(QStringList)";
    invokableMethods << "variantSignal(QVariant)";
    invokableMethods << "variantListSignal(QVariantList)";
    invokableMethods << "variantMapSignal(QVariantMap)";
    invokableMethods << "byteArraySignal(QByteArray)";
    invokableMethods << "allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)";
    invokableMethods << "setInteger(int)";
    invokableMethods << "setBoolean(bool)";
    invokableMethods << "setReal(double)";
    invokableMethods << "setString(QString)";
    invokableMethods << "setStringList(QStringList)";
    invokableMethods << "setVariant(QVariant)";
    invokableMethods << "setVariantList(QVariantList)";
    invokableMethods << "setVariantMap(QVariantMap)";
    invokableMethods << "setByteArray(QByteArray)";
    invokableMethods << "setAll(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)";
    invokableMethods << "noParams()";
    invokableMethods << "integer(int)";
    invokableMethods << "boolean(bool)";
    invokableMethods << "real(double)";
    invokableMethods << "string(QString)";
    invokableMethods << "stringList(QStringList)";
    invokableMethods << "variant(QVariant)";
    invokableMethods << "variantList(QVariantList)";
    invokableMethods << "variantMap(QVariantMap)";
    invokableMethods << "byteArray(QByteArray)";
    invokableMethods << "allParams(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)";
    invokableMethods << "functionWith2Parameters(QVariantMap,QString)";
    invokableMethods << "testResultFunction(bool)";
    invokableMethods << "longFunction(int)";
    invokableMethods << "largeParameter(QByteArray)";
    invokableMethods << "largeReturnType(int)";
    invokableMethods << "largeParameterAndReturnType(QByteArray)";
    invokableMethods << "terminate()";
    invokableMethods << "terminateNow()";
    invokableMethods << "emitSignals()";

    QVERIFY(invokableMethods == remoteObject.invokableMethods());
}

void IpcRemoteObjectTest::testChangeAndUpdatePropertyMethods_data()
{
    QTest::addColumn<QString>("propertyName");
    QTest::addColumn<QVariant>("propertyValue");
    QTest::addColumn<QString>("notifySignal");

    QTest::newRow("integer") << "integer" << QVariant(500) << "integerSignal(int)";
    QTest::newRow("boolean") << "boolean" << QVariant(true) << "booleanSignal(bool)";
    QTest::newRow("real") << "real" << QVariant(987.654) << "realSignal(double)";
    QTest::newRow("string") << "string" << QVariant("Modified Signal") << "stringSignal(QString)";
    QStringList strList = QStringList() << "M" << "N" << "O" << "P";
    QTest::newRow("stringList") << "stringList" << QVariant(strList) << "stringListSignal(QStringList)";
    QTest::newRow("variant") << "variant" << QVariant(QDate::currentDate()) << "variantSignal(QVariant)";
    QVariantList varList = QVariantList() << 50 << true << 234.234 << QTime::currentTime();
    QTest::newRow("variantList") << "variantList" << QVariant(varList) << "variantListSignal(QVariantList)";
    QVariantMap varMap;
    varMap["June"] = 31;
    varMap["July"] = 31;
    varMap["August"] = 31;
    varMap["September"] = 30;
    varMap["October"] = 31;
    QTest::newRow("variantMap") << "variantMap" << QVariant(varMap) << "variantMapSignal(QVariantMap)";
    QTest::newRow("byteArray") << "byteArray" << QVariant(QByteArray("C", 200)) << "byteArraySignal(QByteArray)";
}

void IpcRemoteObjectTest::testChangeAndUpdatePropertyMethods()
{
    QFETCH(QString, propertyName);
    QFETCH(QVariant, propertyValue);
    QFETCH(QString, notifySignal);

    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    // Connect the notifySignal in the remote object to a local signal in Receiver
    // This way we can monitor if the notification signal was emitted properly or not.
    QByteArray sig = QString("2" + notifySignal).toLatin1();
    SenderOrReceiver receiver;
    bool success = GCF::ipcConnect(&remoteObject, sig, &receiver, sig);
    QVERIFY2(success==true, "Could not connect to notifySignal");

    GCF::SignalSpy receiverSpy(&receiver, sig);
    QVERIFY2(receiverSpy.isValid(), "Cannot spy on notifySignal in receiver");

    // Spy on the propertyUpdated() signal to make sure that it is delivered.
    GCF::SignalSpy propertyUpdatedSpy(&remoteObject, SIGNAL(propertyUpdated(QString,QVariant)));
    QVERIFY2(propertyUpdatedSpy.isValid(), "Cannot spy on propertyUpdated in remoteObject");

    // Trigger for change of property
    success = remoteObject.changeProperty(propertyName, propertyValue, &receiver,
                                          SLOT(propertyNotification(QVariant,bool,QString)));
    QVERIFY2(success, "GCF::IpcRemoteObject::changeProperty() returned false!");

    // Wait for the change to be done.
    propertyUpdatedSpy.wait(::MaxSpinWaitTime);

    // Check everything
    QVERIFY(receiver.result.isSuccess() == true);
    QVERIFY(receiver.result.data() == propertyValue);
    QVERIFY2(propertyUpdatedSpy.count() == 1, "GCF::IpcRemoteObject::propertyUpdated(QString,QVariant) was not emitted");
    QVERIFY2(propertyUpdatedSpy.first().first().toString() == propertyName, "GCF::IpcRemoteObject::propertyUpdated() - 1st parameter error");
    QVERIFY2(propertyUpdatedSpy.first().last() == propertyValue, "GCF::IpcRemoteObject::propertyUpdated() - 2nd parameter error");

    // Wait for notification signal to be delivered.
    if(receiverSpy.isEmpty())
        receiverSpy.wait(::MaxSpinWaitTime);
    QVERIFY2(receiverSpy.count() == 1, "Notification signal was not received");
    QVERIFY2(receiverSpy.first().first() == propertyValue, "Notification signal property was incorrect");

    QVERIFY2(remoteObject.properties().value(propertyName) == propertyValue, "Properties map on the IpcRemoteObject not updated");

    // Now fetch the new property from the remote process
    receiverSpy.clear();
    propertyUpdatedSpy.clear();
    receiver.result = GCF::Result();
    success = remoteObject.updateProperty(propertyName, &receiver, SLOT(propertyNotification(QVariant,bool,QString)));
    QVERIFY2(success, "GCF::IpcRemoteObject::updateProperty() returned false!");

    // Wait for the change to be done.
    propertyUpdatedSpy.wait(::MaxSpinWaitTime);

    // Check everything
    QVERIFY(receiver.result.isSuccess() == true);
    QVERIFY(receiver.result.data() == propertyValue);
    QVERIFY2(propertyUpdatedSpy.count() == 1, "GCF::IpcRemoteObject::propertyUpdated(QString,QVariant) was not emitted");
    QVERIFY2(propertyUpdatedSpy.first().first().toString() == propertyName, "GCF::IpcRemoteObject::propertyUpdated() - 1st parameter error");
    QVERIFY2(propertyUpdatedSpy.first().last() == propertyValue, "GCF::IpcRemoteObject::propertyUpdated() - 2nd parameter error");
}

void IpcRemoteObjectTest::testChangeAndUpdatePropertyErrors()
{
    // Launch the remote object
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");

    // At this point, the remote object is not activated. So even valid property names should fail!
    GCF::Result result = remoteObject.updateProperty("integer");
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Property 'integer' doesnt exist in this object");

    // Wait for the remote object to activate
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    result = remoteObject.updateProperty("integer");
    QVERIFY(result.isSuccess() == true);

    result = remoteObject.updateProperty("unknown");
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Property 'unknown' doesnt exist in this object");

    // Provide receiver, but not method.
    SenderOrReceiver receiver;
    result = remoteObject.updateProperty("integer", &receiver);
    QVERIFY(result.isSuccess() == true);

    // Provide invalid member
    result = remoteObject.updateProperty("integer", &receiver, SLOT(propertyNotificationWrong(QVariant)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Receiver's member function's parameter list should be (QVariant,bool,QString).");
}

void IpcRemoteObjectTest::testConnect()
{
    /*
     * We wont test any of the single parameter signals here, because they have already
     * been tested in the last test-case-function. In this function we will test multi-parameter
     * signal in a later test-function we will test a whole lot of error scenarios.
     */

    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    GCF::Result result;

    // Lets test a successful connection first
    SenderOrReceiver sender;
    result = GCF::ipcConnect(&sender,
                             SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)),
                             &remoteObject,
                             SLOT(setAll(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QVERIFY(result.isSuccess() == true);

    // Lets connect to a remote signal next
    SenderOrReceiver receiver;
    GCF::SignalSpy requestFinishedSpy(&remoteObject, SIGNAL(requestFinished(int)));
    result = GCF::ipcConnect(&remoteObject,
                             SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)),
                             &receiver,
                             SLOT(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QVERIFY(result.isSuccess() == true);

    // Wait for the IPC connection to get fully established
    requestFinishedSpy.wait(::MaxSpinWaitTime);
    QVERIFY2(requestFinishedSpy.count() == 1, "remoteObject::allSignal() to receiver2::allSignal() connection was not confirmed as established");
    QVERIFY(requestFinishedSpy.first().first() == result.data());

    // To check if signalOccurance() signal got emitted or not
    GCF::SignalSpy signalOccuranceSpy(&remoteObject, SIGNAL(signalOccurance(QString,QVariantList)));

    // Emit a signal from receiver1 and expect to receive notification in receiver2
    GCF::SignalSpy receiverSpy(&receiver, SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QVERIFY(receiverSpy.isValid());

    // Send the signal from the local process to the remote process
    sender.allSignal(30, false, 234.345, "Om Shanthi Om",
                     QStringList() << "DDLJ" << "OSO" << "KKHH" << "K3G" << "RaONE",
                     QVariant(QDate::currentDate()),
                     QVariantList(), QVariantMap(), QByteArray('X', 4));

    // Wait for the remote process to send a bounce back signal. If it comes; then
    // both local-to-remote and remote-to-local connections worked!
    receiverSpy.wait(::MaxSpinWaitTime);

    QVERIFY(receiverSpy.count() == 1);

    if(!signalOccuranceSpy.count())
        signalOccuranceSpy.wait(::MaxSpinWaitTime);
    QVERIFY(signalOccuranceSpy.count() == 1);

    // Check each and every parameter.
    QVariantList signalArgs = receiverSpy.first();
    QVERIFY(signalOccuranceSpy.first().first().toString() == "allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)");
    QVERIFY(signalArgs == signalOccuranceSpy.first().last().toList());

    QVERIFY(signalArgs.at(0) == QVariant(30));
    QVERIFY(signalArgs.at(1) == QVariant(false));
    QVERIFY(signalArgs.at(2) == QVariant(234.345));
    QVERIFY(signalArgs.at(3) == QVariant("Om Shanthi Om"));
    QVERIFY(signalArgs.at(4).toStringList() == QStringList() << "DDLJ" << "OSO" << "KKHH" << "K3G" << "RaONE");
    QVERIFY(signalArgs.at(5) == QVariant(QDate::currentDate()));
    QVERIFY(signalArgs.at(6).toList() == QVariantList());
    QVERIFY(signalArgs.at(7).toMap() == QVariantMap());
    QVERIFY(signalArgs.at(8).toByteArray() == QByteArray('X', 4));
}

void IpcRemoteObjectTest::testConnect2()
{
    /*
      There is only a small differnece between this test function and the previous.
      In this test function we make use of

      GCF::ipcConnect(QObject *sender, const char *signal, const QHostAddress &addr,
                quint16 port, const QString &object, const QString &method);

      to make local-signal-to-remote-method connection instead of GCF::IpcRemoteObject::ipcConnect()
      */

    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    GCF::Result result;

    // Lets test a successful connection first
    SenderOrReceiver sender;
    result = GCF::ipcConnect(&sender,
                             SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)),
                             QHostAddress::LocalHost,
                             ::ServerPort,
                             "Application.TestService",
                             "setAll");
    QVERIFY(result.isSuccess() == true);

    // Lets connect to a remote signal next
    SenderOrReceiver receiver;
    GCF::SignalSpy requestFinishedSpy(&remoteObject, SIGNAL(requestFinished(int)));
    result = GCF::ipcConnect(&remoteObject,
                             SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)),
                             &receiver,
                             SLOT(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QVERIFY(result.isSuccess() == true);

    // Wait for the IPC connection to get fully established
    requestFinishedSpy.wait(::MaxSpinWaitTime);
    QVERIFY2(requestFinishedSpy.count() == 1, "remoteObject::allSignal() to receiver2::allSignal() connection was not confirmed as established");
    QVERIFY(requestFinishedSpy.first().first() == result.data());

    // To check if signalOccurance() signal got emitted or not
    GCF::SignalSpy signalOccuranceSpy(&remoteObject, SIGNAL(signalOccurance(QString,QVariantList)));

    // Emit a signal from receiver1 and expect to receive notification in receiver2
    GCF::SignalSpy receiverSpy(&receiver, SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)));
    QVERIFY(receiverSpy.isValid());

    // Send the signal from the local process to the remote process
    sender.allSignal(30, false, 234.345, "Om Shanthi Om",
                     QStringList() << "DDLJ" << "OSO" << "KKHH" << "K3G" << "RaONE",
                     QVariant(QDate::currentDate()),
                     QVariantList(), QVariantMap(), QByteArray('X', 4));

    // Wait for the remote process to send a bounce back signal. If it comes; then
    // both local-to-remote and remote-to-local connections worked!
    receiverSpy.wait(::MaxSpinWaitTime);

    QVERIFY(receiverSpy.count() == 1);

    if(!signalOccuranceSpy.count())
        signalOccuranceSpy.wait(::MaxSpinWaitTime);
    QVERIFY(signalOccuranceSpy.count() == 1);

    // Check each and every parameter.
    QVariantList signalArgs = receiverSpy.first();
    QVERIFY(signalOccuranceSpy.first().first().toString() == "allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)");
    QVERIFY(signalArgs == signalOccuranceSpy.first().last().toList());

    QVERIFY(signalArgs.at(0) == QVariant(30));
    QVERIFY(signalArgs.at(1) == QVariant(false));
    QVERIFY(signalArgs.at(2) == QVariant(234.345));
    QVERIFY(signalArgs.at(3) == QVariant("Om Shanthi Om"));
    QVERIFY(signalArgs.at(4).toStringList() == QStringList() << "DDLJ" << "OSO" << "KKHH" << "K3G" << "RaONE");
    QVERIFY(signalArgs.at(5) == QVariant(QDate::currentDate()));
    QVERIFY(signalArgs.at(6).toList() == QVariantList());
    QVERIFY(signalArgs.at(7).toMap() == QVariantMap());
    QVERIFY(signalArgs.at(8).toByteArray() == QByteArray('X', 4));
}

void IpcRemoteObjectTest::testConnectErrors()
{
    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    /*
     * The following error scenarios can happen in local-signal-to-remote-member connections
     *
     *  1. Local signal doesnt exist
     *  2. Local signal has parameters that are not supported. See below for supported types.
     *      [int, bool, double, QString, QStringList, QVariant, QVariantList, QVariantMap, QByteArray]
     *  3. Remote member doesnt exist
     *  4. Parameters from signal and those from slot are not equal
     */
    SenderOrReceiver sender;

    GCF::Result result = GCF::ipcConnect(&sender, SIGNAL(nonExistingSignal(int)),
                                         &remoteObject, SLOT(setInteger(int)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Signal doesnt exist in the sender");

    result = GCF::ipcConnect(&sender, SIGNAL(dateSignal(QDate)), QHostAddress::LocalHost, ::ServerPort,
                             "Application.TestService", "setVariant");
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Parameter type 'QDate' not supported");

    result = GCF::ipcConnect(&sender, SIGNAL(booleanSignal(bool)), &remoteObject, SLOT(unknown(bool)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Member doesnt exist in remote object");

    result = GCF::ipcConnect(&sender, SIGNAL(dateSignal(QDate)), &remoteObject, SLOT(setVariant(QVariant)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Parameters accepted by the member must be same as those emitted by the signal");

    /*
     * The following error scenarios can happen in remote-signal-to-local-member connections
     *
     *  1. Remote signal doesnt exist
     *  2. Local member doesnt exist
     *  3. Parameters from signal and those from slot are not equal
     *
     * We dont have to check for parameter-type support because check #3 will take care of that.
     * And we never connect to a signal that doesnt exist in the remote object. Plus we are
     * guaranteed that remote obejct sends signal list with only those signals whose parameters
     * are supported.
     */
    SenderOrReceiver receiver;

    result = GCF::ipcConnect(&remoteObject, SIGNAL(unknown(QVariant)), &receiver, SIGNAL(variantSignal(QVariant)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Signal doesnt exist in the remote object");

    result = GCF::ipcConnect(&remoteObject, SIGNAL(variantSignal(QVariant)), &receiver, SLOT(unknown(QVariant)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Member doesnt exist in the receiver");

    result = GCF::ipcConnect(&remoteObject, SIGNAL(allSignal(int,bool,double,QString,QStringList,QVariant,QVariantList,QVariantMap,QByteArray)),
                             &receiver, SIGNAL(allSignal2(QString,QStringList,int,bool,double,QVariantMap,QByteArray,QVariant,QVariantList)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Parameters accepted by the member must be same as those emitted by the signal");

    /*
     * Other errors
     *
     * 1. Sender not specified
     * 2. Receiver not specified
     */
    result = GCF::ipcConnect((QObject*)0, 0, (GCF::IpcRemoteObject*)0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Receiver not specified");

    result = GCF::ipcConnect((QObject*)0, 0, &remoteObject, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "sender, signal and member must be provided as parameters to this function");

    result = GCF::ipcConnect((QObject*)0, 0, &remoteObject, SLOT(setInteger(int)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "sender, signal and member must be provided as parameters to this function");

    result = GCF::ipcConnect(&receiver, 0, &remoteObject, SLOT(setInteger(int)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "sender, signal and member must be provided as parameters to this function");

    result = GCF::ipcConnect((GCF::IpcRemoteObject*)0, 0, (QObject*)0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Sender not specified");

    result = GCF::ipcConnect(&remoteObject, 0, (QObject*)0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "signal, receiver and member must be provided as parameters to this function");

    result = GCF::ipcConnect(&remoteObject, SIGNAL(integerSignal(int)), (QObject*)0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "signal, receiver and member must be provided as parameters to this function");

    result = GCF::ipcConnect(&remoteObject, SIGNAL(integerSignal(int)), &receiver, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "signal, receiver and member must be provided as parameters to this function");
}

/**
 * GCF::IpcRemoteObject::call() internally uses GCF::IpcCall. We have an
 * exhaustive test suite for testing GCF::IpcCall; so we dont need to redo
 * all of that testing here.
 *
 * What we need to test here is that valid calls work and that error messages
 * are returned appropriately.
 *
 * For valid-calls test; we repeat data-driven-testing from tst_IpcTest.cpp
 */
void IpcRemoteObjectTest::testCalls_data()
{
    QTest::addColumn<QString>("function");
    QTest::addColumn<QVariantList>("args");

    QTest::newRow("noParams") << "noParams" << QVariantList();
    QTest::newRow("integer") << "integer" << (QVariantList() << 10);
    QTest::newRow("boolean") << "boolean" << (QVariantList() << false);
    QTest::newRow("real") << "real" << (QVariantList() << 123.456);
    QTest::newRow("string") << "string" << (QVariantList() << "Hello World");

    QStringList strList;
    for(int i=0; i<10; i++)
        strList << QString("String %1").arg(i+1);
    QTest::newRow("stringList") << "stringList" << (QVariantList() << strList);

    QTest::newRow("variant") << "variant" << (QVariantList() << QDate::currentDate());

    QVariantList varList;
    varList.append( QDate::currentDate() );
    varList.append( QString("this is a simple string") );
    varList.append( true );
    varList.append( 69123.123 );

    QTest::newRow("variantList") << "variantList" << (QVariantList() << QVariant(varList));

    QVariantMap varMap;
    varMap["january"] = 31;
    varMap["february"] = 28;
    varMap["march"] = 31;
    varMap["april"] = 30;
    varMap["may"] = 31;

    QTest::newRow("variantMap") << "variantMap" << (QVariantList() << QVariant(varMap));
    QTest::newRow("byteArray") << "byteArray" << (QVariantList() << QByteArray(4096, 'A'));

    QVariantList allParamsArgs;
    {
        allParamsArgs.append(10);
        allParamsArgs.append(false);
        allParamsArgs.append(123.34);
        allParamsArgs.append(QString("string value"));
        allParamsArgs.append(QStringList() << "a" << "b" << "C");
        allParamsArgs.append(QVariant(QDate::currentDate()));

        QVariantList varList;
        varList.append( QDate::currentDate() );
        varList.append( QString("this is a simple string") );
        varList.append( true );
        varList.append( 69123.123 );
        allParamsArgs.append( QVariant(varList) );

        QVariantMap varMap;
        varMap["january"] = 31;
        varMap["february"] = 28;
        varMap["march"] = 31;
        varMap["april"] = 30;
        varMap["may"] = 31;

        allParamsArgs.append( QVariant(varMap) );

        QByteArray bytes(4096, 'A');
        allParamsArgs.append(bytes);
    }

    QTest::newRow("allParams") << "allParams" << allParamsArgs;
}

void IpcRemoteObjectTest::testCalls()
{
    QFETCH(QString, function);
    QFETCH(QVariantList, args);

    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    GCF::SignalSpy requestFinishedSpy(&remoteObject, SIGNAL(requestFinished(int)));

    SenderOrReceiver receiver;
    remoteObject.call(function, args, &receiver, SLOT(callNotification(QVariant,bool,QString)));

    requestFinishedSpy.wait(::MaxSpinWaitTime);

    QVERIFY(receiver.result.isSuccess() == true);
    if(args.count())
    {
        if(args.count() == 1)
            QVERIFY(receiver.result.data() == args.first());
        else
            QVERIFY(receiver.result.data().toList() == args);
    }
}

void IpcRemoteObjectTest::testCallErrors()
{
    /*
      Possible error scenarios

      1. Method doesnt exist in the remote object
      2. Notification member doesnt exist in the receiver object
      3. Receiver's member function's parameter list should be (QVariant,bool,QString)
      4. Error messages from server must be received in the receiver's notification slot
      */

    // Launch the remote object
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");

    GCF::Result result = remoteObject.call("setInteger", QVariantList() << 10, 0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Method doesnt exist in the remote object");

    // Wait for it to activate
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    result = remoteObject.call("unknownFunction", QVariantList() << 10, 0, 0);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Method doesnt exist in the remote object");

    SenderOrReceiver receiver;

    result = remoteObject.call("setInteger", QVariantList() << 10,
                                           &receiver, SLOT(callNotification1(QVariant,bool,QString)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Notification member doesnt exist in the receiver object");

    result = remoteObject.call("setInteger", QVariantList() << 10,
                               &receiver, SLOT(callNotificationWrong(bool,QVariant,QString)));
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Receiver's member function's parameter list should be (QVariant,bool,QString)");

    GCF::SignalSpy requestFinishedSpy(&remoteObject, SIGNAL(requestFinished(int)));
    result = remoteObject.call("setInteger", QVariantList() << QVariantMap(),
                               &receiver, SLOT(callNotification(QVariant,bool,QString)));
    QVERIFY(result.isSuccess() == true);
    requestFinishedSpy.wait(::MaxSpinWaitTime);

    QVERIFY(receiver.result.isSuccess() == false);
    QVERIFY(receiver.result.message() == "Invalid parameter type. Expecting 'int' but found 'QVariantMap'");
}

void IpcRemoteObjectTest::testDeactivation()
{
    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    GCF::SignalSpy deactivationSpy(&remoteObject, SIGNAL(deactivated()));
    this->killIpcServer();

    if(!deactivationSpy.count())
        deactivationSpy.wait(::MaxSpinWaitTime);
    QVERIFY(deactivationSpy.count() == 1);
    QVERIFY(remoteObject.isActivated() == false);
    QVERIFY(remoteObject.properties().isEmpty());
    QVERIFY(remoteObject.signalMethods().isEmpty());
    QVERIFY(remoteObject.invokableMethods().isEmpty());

    // Launch the server again
    this->setupIpcServer();

    // Reactivate the remote object and see if that works.
    spy.clear();
    remoteObject.reactivate();
    if(!spy.count())
        spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    QVERIFY(remoteObject.isActivated() == true);
    QVERIFY(!remoteObject.properties().isEmpty());
    QVERIFY(!remoteObject.signalMethods().isEmpty());
    QVERIFY(!remoteObject.invokableMethods().isEmpty());
}

void IpcRemoteObjectTest::testDeactivation2()
{
    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(activated()));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);

    // Request for a property
    QList<SenderOrReceiver*> receiverList;
    GCF::Result result;

    for(int i=0; i<1000; i++)
    {
        receiverList.append( new SenderOrReceiver );
        result = remoteObject.updateProperty("byteArray", receiverList.last(),
                                             SLOT(propertyNotification(QVariant,bool,QString)));
        QVERIFY(result.isSuccess() == true);
    }

    // Now kill the server
    GCF::SignalSpy deactivationSpy(&remoteObject, SIGNAL(deactivated()));
    this->killIpcServer();
    if(!deactivationSpy.count())
        deactivationSpy.wait(::MaxSpinWaitTime);

    // We should have received a error in receiver
    int successCounter = 0;
    Q_FOREACH(SenderOrReceiver *receiver, receiverList)
    {
        if(receiver->result.isSuccess())
            ++successCounter;
        else
            QVERIFY(receiver->result.message() == "Connection with remote object was lost before any response could be received");
    }

    qDebug() << "Of the " << receiverList.count() << " updateProperty requests, "
             << successCounter << " requests successful. Others were failures";
}

void IpcRemoteObjectTest::testConnectionToNonExistingServer()
{
    QVERIFY(m_remoteApp == 0);

    // Launch the remote object and wait for it to activate
    GCF::IpcRemoteObject remoteObject(QHostAddress::LocalHost, ::ServerPort, "Application.TestService");
    GCF::SignalSpy spy(&remoteObject, SIGNAL(couldNotActivate(QString)));
    spy.wait(::MaxSpinWaitTime);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().first().toString() == "Connection refused");
}

void IpcRemoteObjectTest::setupIpcServer()
{
    QElapsedTimer timer;
    if(m_remoteApp)
        return;

#ifdef Q_OS_WIN
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp.exe");
#else
    QString appFile = this->helperDirectory().absoluteFilePath("IpcServerApp");
#endif

    QVERIFY( QFile::exists(appFile) );

    m_remoteApp = new QProcess(this);
    GCF::SignalSpy startedSpy(m_remoteApp, SIGNAL(started()));

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
    else
    {
        connect(m_remoteApp, SIGNAL(readyReadStandardError()),
                this, SLOT(onServerReadyRead()));
        connect(m_remoteApp, SIGNAL(readyReadStandardOutput()),
                this, SLOT(onServerReadyRead()));
    }
}

void IpcRemoteObjectTest::killIpcServer()
{
    QElapsedTimer timer;
    if(!m_remoteApp)
        return;

    qDebug() << "Terminating IpcServerApp[" << m_remoteApp->pid() << "]";

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, ::ServerPort,
                                          "Application.TestService", "terminate",
                                          QVariantList());

    GCF::SignalSpy doneSpy(call, SIGNAL(done(bool)));
    GCF::SignalSpy procSpy(m_remoteApp, SIGNAL(finished(int)));

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

    qDebug() << "    DONE!";
}

QDir IpcRemoteObjectTest::helperDirectory() const
{
    QDir dir = QDir::current();
    dir.cdUp();
    dir.cd("Helpers");
    return dir;
}

QString IpcRemoteObjectTest::logFileContents(bool deleteFile) const
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
    IpcRemoteObjectTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_IpcRemoteObjectTest.moc"
