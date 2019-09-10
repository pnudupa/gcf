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

#include "AgentScriptRunner.h"
#include "AgentSignalSpy.h"

#include <QFile>
#include <QtTest>

struct AgentScriptRunnerData
{
    AgentScriptRunnerData() : engine(nullptr), failCount(0), fail(false) { }

    AgentComponent *agentComponent;
    QScriptEngine *engine;
    int failCount;
    int fail; // This is actually bool, but we use a int for proper padding.

    QList<AgentSignalSpy*> spyList;

    int lineNumber(QScriptEngine *engine) const;
    bool call(QScriptValue &function, const QString &functionName=QString());
    static QScriptValue createValue(QScriptEngine *engine, const QVariant& value);
};

QScriptValue AgentScriptRunner_log(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_fail(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_object(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_interface(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_wait(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_waitForWindowShown(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_waitForWindowActive(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_waitForSignal(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_waitForPropertyChange(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_compare(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_verify(QScriptContext *context, QScriptEngine *engine);
QScriptValue AgentScriptRunner_createSignalSpy(QScriptContext *context, QScriptEngine *engine);

AgentScriptRunner::AgentScriptRunner(AgentComponent *parent) :
    QObject(parent)
{
    d = new AgentScriptRunnerData;
    d->agentComponent = parent;
}

AgentScriptRunner::~AgentScriptRunner()
{
    delete d;
}

GCF::Result AgentScriptRunner::run(const QString &scriptFile)
{
    if(scriptFile.isEmpty())
        return GCF::Result(false, QString(), QString("No script file name provided"));

    QFile file(scriptFile);
    if(!file.open(QFile::ReadOnly))
        return GCF::Result(false, QString(),
                           QString("Cannot open script file '%1' for reading").arg(scriptFile));

    QString script = file.readAll();
    if(script.isEmpty())
        return GCF::Result(false, QString(),
                           QString("Script file '%1' was empty").arg(scriptFile));

    return this->runCode(script, scriptFile);
}

GCF::Result AgentScriptRunner::runCode(const QString &script, const QString &scriptFile)
{
    QScriptEngine engine;
    d->engine = &engine;
    engine.globalObject().setProperty("Qt", engine.newQMetaObject(&QObject::staticQtMetaObject));
    engine.globalObject().setProperty("investigator", engine.newQObject(this));
    engine.globalObject().setProperty("log", engine.newFunction(AgentScriptRunner_log));
    engine.globalObject().setProperty("fail", engine.newFunction(AgentScriptRunner_fail));
    engine.globalObject().setProperty("object", engine.newFunction(AgentScriptRunner_object));
    engine.globalObject().setProperty("interface", engine.newFunction(AgentScriptRunner_interface));
    engine.globalObject().setProperty("wait", engine.newFunction(AgentScriptRunner_wait));
    engine.globalObject().setProperty("waitForWindowShown", engine.newFunction(AgentScriptRunner_waitForWindowShown));
    engine.globalObject().setProperty("waitForWindowActive", engine.newFunction(AgentScriptRunner_waitForWindowActive));
    engine.globalObject().setProperty("waitForSignal", engine.newFunction(AgentScriptRunner_waitForSignal));
    engine.globalObject().setProperty("waitForPropertyChange", engine.newFunction(AgentScriptRunner_waitForPropertyChange));
    engine.globalObject().setProperty("compare", engine.newFunction(AgentScriptRunner_compare));
    engine.globalObject().setProperty("verify", engine.newFunction(AgentScriptRunner_verify));
    engine.globalObject().setProperty("createSignalSpy", engine.newFunction(AgentScriptRunner_createSignalSpy));

    QScriptValue scriptValue = engine.evaluate(script, scriptFile);
    if(scriptValue.isError() || engine.hasUncaughtException())
    {
        int lineNr = -1;
        QString errMsg("Unknown error");
        if(engine.hasUncaughtException())
        {
            lineNr = engine.uncaughtExceptionLineNumber();
            errMsg = engine.uncaughtException().toString();
        }

        return GCF::Result(false, QString(),
                           QString("Error at line %1: %2").arg(lineNr).arg(errMsg));
    }

    // Now look for functions of the form test_xxx()
    QList<QScriptValue> testFunctions;
    QStringList testFunctionNames;
    QScriptValue initTestCaseFn;
    QScriptValue cleanupTestCaseFn;
    QScriptValue initFn;
    QScriptValue cleanupFn;
    QScriptValueIterator scriptIterator(engine.globalObject());

    while(scriptIterator.hasNext())
    {
        scriptIterator.next();

        QScriptValue item = scriptIterator.value();
        if(item.isFunction())
        {
            QString fnName = scriptIterator.name();
            if(fnName == "initTestCase")
                initTestCaseFn = item;
            else if(fnName == "cleanupTestCase")
                cleanupTestCaseFn = item;
            else if(fnName == "init")
                initFn = item;
            else if(fnName == "cleanup")
                cleanupFn = item;
            else if(fnName.startsWith("test_"))
            {
                testFunctionNames.append(fnName);
                testFunctions.append(item);
            }
        }
    }

    if(testFunctions.isEmpty())
        return GCF::Result(false, QString(), QString("No test_...() functions were found in the script"));

    // Now execute the test functions
    if(initTestCaseFn.isValid())
        d->call(initTestCaseFn, "initTestCase");

    d->failCount = 0;
    d->fail = false;

    for(int i=0; i<testFunctions.count(); i++)
    {
        QString testFunctionName = testFunctionNames.at(i);
        QScriptValue testFunction = testFunctions.at(i);
        d->agentComponent->pushLogBranch(testFunctionName);

        d->call(initFn);
        if(d->fail)
            --d->failCount;
        d->fail = 0;

        /**
          Signal spies created in initTestCase() and/or init() are
          to be deleted at the end of the test-script. Not before that.
          So we call clear() here; but not qDeleteAll(d->spyList);
          */
        d->spyList.clear();

        d->call(testFunction);

        qDeleteAll(d->spyList);
        d->spyList.clear();

        d->fail = false;

        d->call(cleanupFn);
        d->fail = false;

        d->agentComponent->popLogBranch();
    }

    if(cleanupTestCaseFn.isValid())
        d->call(cleanupTestCaseFn, "cleanupTestCase");

    // Delete all AgentSignalSpy instances
    d->spyList = this->findChildren<AgentSignalSpy*>();
    qDeleteAll(d->spyList);
    d->spyList.clear();

    if(d->failCount)
        return GCF::Result(false, QString(), QString("%1 of %2 test case(s) failed").arg(d->failCount).arg(testFunctions.count()));

    return GCF::Result(true, QString(), QString("%1 test case(s) passed").arg(testFunctions.count()));
}

void AgentScriptRunner::log(const QScriptValue &msg)
{
    d->agentComponent->log(msg.toString());
}

void AgentScriptRunner::fail(const QScriptValue &msg)
{
    int linrNr = d->lineNumber(msg.engine());
    QString msg2 = QString("FAILED at line %1: %2")
            .arg(linrNr)
            .arg(msg.toString());
    this->log(msg2);
    d->fail = true;
    ++d->failCount;
    msg.engine()->abortEvaluation();
}

QScriptValue AgentScriptRunner::object(const QScriptValue &path)
{
    QObject *object = this->findObject(path.toString());
    if(object == nullptr)
    {
        int lineNr = d->lineNumber(path.engine());
        d->agentComponent->log(QString("Error at line %1: Object '%2' not found").arg(lineNr).arg(path.toString()));
        return QScriptValue(path.engine(), QScriptValue::NullValue);
    }

    QScriptValue objectValue = path.engine()->newQObject(object);
    this->wrap(objectValue);
    return objectValue;
}

void AgentScriptRunner::wait(int ms)
{
    QTest::qWait(ms);
}

void AgentScriptRunner::waitForWindowShown(const QScriptValue &window, int timeout)
{
    QObject *windowObject = this->objectFromScriptValue(window);
    if(!windowObject)
        return;

    if(!windowObject->isWidgetType())
    {
        QString msg = QString("ERROR %1: Widget is not a window").arg(d->lineNumber(window.engine()));
        this->log(QScriptValue(window.engine(), msg));
        return;
    }

    QWidget *windowPointer = qobject_cast<QWidget*>(windowObject);

#if QT_VERSION >= 0x050000
    const bool success = QTest::qWaitForWindowActive(windowPointer, timeout);
#else
    Q_UNUSED(timeout);
    const bool success = QTest::qWaitForWindowShown(windowPointer);
#endif

    Q_UNUSED(success)
}

void AgentScriptRunner::waitForWindowActive(const QScriptValue &window, int timeout)
{
    this->waitForWindowShown(window, timeout);
}

QScriptValue AgentScriptRunner::waitForSignal(const QScriptValue &object, const QString &signal, int timeout)
{
    QObject *qObject = this->objectFromScriptValue(object);
    if(!qObject)
        return QScriptValue(false);

    QByteArray signalBa = signal.toLatin1();
    signalBa = QMetaObject::normalizedSignature(signalBa);
    int signalIndex = qObject->metaObject()->indexOfMethod(signalBa);
    if( signalIndex < 0 )
    {
        QScriptValue msg(object.engine(), QString("Signal '%1' doesnt exist").arg(signal));
        this->log(msg);
        return QScriptValue(false);
    }

    signalBa.prepend('0' + QSIGNAL_CODE);
    GCF::SignalSpy spy(qObject, signalBa);
    if(!spy.isValid())
    {
        QScriptValue msg(object.engine(), QString("Cannot spy on signal '%1'").arg(signal));
        this->log(msg);
        return QScriptValue(false);
    }

    if(spy.wait(timeout))
    {
        QVariantList args = spy.last();
        QMetaMethod method = qObject->metaObject()->method(signalIndex);

        QScriptValue retValue = object.engine()->newObject();
        for(int i=0; i<method.parameterNames().count(); i++)
        {
            QString paramName = QString::fromLatin1( method.parameterNames().at(i) );
            if(paramName.isEmpty())
                paramName = QString("p%1").arg(i);

            QScriptValue paramValue = d->createValue(object.engine(), args.at(i));
            retValue.setProperty(paramName, paramValue);
        }

        return retValue;
    }

    return QScriptValue(false);
}

QScriptValue AgentScriptRunner::waitForPropertyChange(const QScriptValue &object, const QString &propertyName, int timeout)
{
    QObject *qObject = this->objectFromScriptValue(object);
    if(!qObject)
        return QScriptValue(false);

    QByteArray propertyNameBa = propertyName.toLatin1();
    int propertyIndex = qObject->metaObject()->indexOfProperty(propertyNameBa);
    if(propertyIndex < 0)
    {
        QScriptValue msg(object.engine(), QString("Property '%1' doesnt exist").arg(propertyName));
        this->log(msg);
        return QScriptValue(false);
    }

    QMetaProperty property = qObject->metaObject()->property(propertyIndex);
    if(property.notifySignalIndex() < 0)
    {
        QScriptValue msg(object.engine(), QString("Property '%1' doesnt have a notify signal").arg(propertyName));
        this->log(msg);
        return QScriptValue(false);
    }

#if QT_VERSION >= 0x050000
    QByteArray notifySignal( property.notifySignal().methodSignature() );
#else
    QByteArray notifySignal( property.notifySignal().signature() );
#endif
    notifySignal.prepend('0' + QSIGNAL_CODE);

    GCF::SignalSpy spy(qObject, notifySignal);
    if(!spy.isValid())
    {
        QScriptValue msg(object.engine(), QString("Cannot spy on property '%1'").arg(propertyName));
        this->log(msg);
        return QScriptValue(false);
    }

    if( spy.wait(timeout) )
    {
        QVariant value = property.read(qObject);
        return d->createValue(object.engine(), value);
    }

    return QScriptValue(false);
}

void AgentScriptRunner::compare(const QScriptValue &value1, const QScriptValue &value2, const QScriptValue &msg)
{
    bool equals = false;

    if(value1.isQObject() && value2.isQObject())
        equals = (value1.toQObject() == value2.toQObject());
    else if(value1.isQMetaObject() && value2.isQMetaObject())
        equals = (value1.toQMetaObject() == value2.toQMetaObject());
    else
        equals = value1.strictlyEquals(value2);

    if(equals)
        return;

    this->fail(msg);
}

void AgentScriptRunner::verify(const QScriptValue &condition, const QScriptValue &msg)
{
    if(condition.toBool())
        return;

    this->fail(msg);
}

QScriptValue AgentScriptRunner::createSignalSpy(const QScriptValue &object, const QString &signal)
{
    QObject *qObject = this->objectFromScriptValue(object);
    if(!qObject)
        return QScriptValue(false);

    QByteArray signalBa = signal.toLatin1();
    signalBa = QMetaObject::normalizedSignature(signalBa);
    int signalIndex = qObject->metaObject()->indexOfMethod(signalBa);
    if( signalIndex < 0 )
    {
        QScriptValue msg(object.engine(), QString("Signal '%1' doesnt exist").arg(signal));
        this->log(msg);
        return QScriptValue(false);
    }

    signalBa.prepend('0' + QSIGNAL_CODE);
    AgentSignalSpy *spy = new AgentSignalSpy(qObject, signalBa, this);
    d->spyList.append(spy);

    return object.engine()->newQObject(spy);
}

QObject *AgentScriptRunner::objectFromScriptValue(const QScriptValue &object)
{
    QObject *qObject = nullptr;

    if(object.isQObject())
        qObject = object.toQObject();
    else
        qObject = this->findObject(object.toString());

    if(!qObject)
    {
        QString msgStr = QString("ERROR %1: Object not found").arg(d->lineNumber(object.engine()));
        QScriptValue msg(object.engine(), msgStr);
        this->log(msg);
    }

    return qObject;
}

///////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////

QScriptValue AgentScriptRunner_log(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    runner->log(arg1);

    return QScriptValue();
}

QScriptValue AgentScriptRunner_fail(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue("Reason unknown");
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    runner->fail(arg1);

    return QScriptValue();
}

QScriptValue AgentScriptRunner_object(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    return runner->object(arg1);
}

QScriptValue AgentScriptRunner_interface(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ?
                context->argument(0) : QScriptValue();

    GCF::ObjectTreeNode *node = gAppService->objectTree()->findObjectNode(arg1.toString());
    QObject *object = node ? node->object() : nullptr;

    QScriptValue objectValue = engine->newQObject(object);
    AgentScriptRunner::wrap(objectValue);
    return objectValue;
}

QScriptValue AgentScriptRunner_wait(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    runner->wait(arg1.toInt32());
    return QScriptValue();
}

QScriptValue AgentScriptRunner_waitForWindowShown(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    runner->waitForWindowShown(arg1, arg2.toInt32());
    return QScriptValue();
}

QScriptValue AgentScriptRunner_waitForWindowActive(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    runner->waitForWindowActive(arg1, arg2.toInt32());
    return QScriptValue();
}

QScriptValue AgentScriptRunner_waitForSignal(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QScriptValue arg3 = context->argumentCount() >= 3 ? context->argument(3) : QScriptValue(5000);
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    return runner->waitForSignal(arg1, arg2.toString(), arg3.toInt32());
}

QScriptValue AgentScriptRunner_waitForPropertyChange(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QScriptValue arg3 = context->argumentCount() >= 3 ? context->argument(3) : QScriptValue(5000);
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    return runner->waitForPropertyChange(arg1, arg2.toString(), arg3.toInt32());
}

QScriptValue AgentScriptRunner_compare(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QScriptValue arg3 = context->argumentCount() >= 3 ? context->argument(2) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    if(!arg3.isValid())
        arg3 = QScriptValue(engine, "compare() failed");
    runner->compare(arg1, arg2, arg3);
    return QScriptValue();
}

QScriptValue AgentScriptRunner_verify(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    if(!arg2.isValid())
        arg2 = QScriptValue(engine, "verify() failed");
    runner->verify(arg1, arg2);
    return QScriptValue();
}

QScriptValue AgentScriptRunner_createSignalSpy(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue arg1 = context->argumentCount() >= 1 ? context->argument(0) : QScriptValue();
    QScriptValue arg2 = context->argumentCount() >= 2 ? context->argument(1) : QScriptValue();
    QObject *investigatorObject = engine->globalObject().property("investigator").toQObject();
    AgentScriptRunner *runner = qobject_cast<AgentScriptRunner*>(investigatorObject);
    return runner->createSignalSpy(arg1, arg2.toString());
}

///////////////////////////////////////////////////////////////////////////////

int AgentScriptRunnerData::lineNumber(QScriptEngine *engine) const
{
    int lineNr = 0;
    QString fileName;
    QScriptContext *context = engine->currentContext();

    while(context)
    {
        QScriptContextInfo contextInfo(context);
        fileName = contextInfo.fileName();
        lineNr = contextInfo.lineNumber();
        if(!fileName.isEmpty())
            break;
        context = context->parentContext();
    }

    return lineNr;
}

bool AgentScriptRunnerData::call(QScriptValue &function, const QString &functionName)
{
    if(!functionName.isEmpty())
        this->agentComponent->pushLogBranch(functionName);

    QScriptValue callResult = function.call();
    QString msg;

    if(callResult.isError())
    {
        if(callResult.engine()->hasUncaughtException())
            msg = QString("Error at line %1: %2")
                    .arg(callResult.engine()->uncaughtExceptionLineNumber())
                    .arg(callResult.engine()->uncaughtException().toString());
        else
            msg = QString("Error at line %1: Unknown error").arg(this->lineNumber(callResult.engine()));
    }

    if(!msg.isEmpty())
    {
        if(functionName.isEmpty())
            this->agentComponent->log(msg);
        else
            this->agentComponent->popLogBranch(msg);
    }

    return callResult.isError() == false;
}

QScriptValue AgentScriptRunnerData::createValue(QScriptEngine *engine, const QVariant& value)
{
    if(value.type() == QVariant::Map)
    {
        QScriptValue obj = engine->newObject();

        QVariantMap map = value.toMap();
        QVariantMap::const_iterator it = map.begin();
        QVariantMap::const_iterator end = map.end();
        while(it != end)
        {
            obj.setProperty( it.key(), createValue(engine, it.value()) );
            ++it;
        }

        return obj;
    }

    switch(value.type())
    {
    case QVariant::String:
        if(value.toString().isEmpty())
            return QScriptValue(QScriptValue::NullValue);
        return QScriptValue(value.toString());
    case QVariant::Int:
        return QScriptValue(value.toInt());
    case QVariant::UInt:
        return QScriptValue(value.toUInt());
    case QVariant::LongLong:
        return QScriptValue(qsreal(value.toLongLong()));
    case QVariant::Bool:
        return QScriptValue(value.toBool());
    case QVariant::ByteArray:
        return QScriptValue(QLatin1String(value.toByteArray()));
    case QVariant::Double:
        return QScriptValue(qsreal(value.toDouble()));
    case QVariant::StringList: {
        QStringList stringList = value.toStringList();
        QScriptValue array = engine->newArray(uint(stringList.length()));
        for(int i=0; i<stringList.count(); i++)
            array.setProperty(quint32(i), stringList.at(i));
        return array;
        }
    case QVariant::List: {
        QList<QVariant> list = value.toList();
        QScriptValue array = engine->newArray(uint(list.length()));
        for(int i=0; i<list.count(); i++)
            array.setProperty(quint32(i), createValue(engine, list.at(i)));
        return array;
        }
    default:
        break;
    }

    if(value.isNull())
        return QScriptValue(QScriptValue::NullValue);

    return engine->newVariant(value);
}

QScriptValue AgentSignalSpy::at(const QScriptValue &i) const
{
    QScriptValue array = i.engine()->newArray();
    qint32 index = i.toInt32();
    if(index < 0 || index >= m_signalSpy.count())
        return array;

    QVariantList args = m_signalSpy.at(index);
    for(int idx=0; idx<args.count(); idx++)
        array.setProperty(quint32(idx), AgentScriptRunnerData::createValue(i.engine(), args.at(idx)));

    return array;
}
