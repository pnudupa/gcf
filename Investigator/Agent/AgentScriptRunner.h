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

#ifndef AGENTSCRIPTRUNNER_H
#define AGENTSCRIPTRUNNER_H

#include "Agent.h"
#include <QObject>

class AgentComponent;

struct AgentScriptRunnerData;
class AgentScriptRunner : public QObject
{
    Q_OBJECT

public:
    AgentScriptRunner(AgentComponent *parent=nullptr);
    ~AgentScriptRunner();

    GCF::Result run(const QString &scriptFile);
    GCF::Result runCode(const QString &script, const QString &scriptFile=QString("script"));

    void log(const QScriptValue &msg);
    void fail(const QScriptValue &msg);
    QScriptValue object(const QScriptValue &path);

    // QTest functions
    void wait(int ms);
    void waitForWindowShown(const QScriptValue &window, int timeout=5000);
    void waitForWindowActive(const QScriptValue &window, int timeout=5000);
    QScriptValue waitForSignal(const QScriptValue &object, const QString &signal, int timeout=5000);
    QScriptValue waitForPropertyChange(const QScriptValue &object, const QString &propertyName, int timeout=5000);
    void compare(const QScriptValue &value1, const QScriptValue &value2, const QScriptValue &msg);
    void verify(const QScriptValue &condition, const QScriptValue& msg);
    QScriptValue createSignalSpy(const QScriptValue &object, const QString &signal);

    static QObject *findObject(const QString &name);
    static QObject *findObject(QObject *parent, const QString &name);
    static void wrap(QScriptValue &value);

private:
    QObject *objectFromScriptValue(const QScriptValue &object);

private:
    friend struct AgentScriptRunnerData;
    AgentScriptRunnerData *d;
};

#endif // AGENTSCRIPTRUNNER_H
