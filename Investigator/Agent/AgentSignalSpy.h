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

#ifndef AGENTSIGNALSPY_H
#define AGENTSIGNALSPY_H

#include "Agent.h"
#include "../Core/SignalSpy.h"
#include <QtDebug>

class AgentSignalSpy : public QObject
{
    Q_OBJECT

public:
    AgentSignalSpy(QObject *sender, const char *signal, QObject *parent=nullptr)
        : QObject(parent), m_signalSpy(sender, signal) { }
    ~AgentSignalSpy() { }

    Q_PROPERTY(int count READ count)
    int count() const { return m_signalSpy.count(); }

    Q_INVOKABLE QScriptValue at(const QScriptValue &i) const;
    Q_INVOKABLE QScriptValue takeAt(const QScriptValue &i) {
        QScriptValue ret = this->at(i);
        qint32 index = i.toInt32();
        if(index >= 0 && index < m_signalSpy.count())
            m_signalSpy.removeAt(index);
        return ret;
    }

    Q_INVOKABLE void clear() { m_signalSpy.clear(); }

    Q_INVOKABLE bool wait(int timeout=5000) {
        return m_signalSpy.wait(timeout);
    }

private:
    GCF::SignalSpy m_signalSpy;
};

#endif // AGENTSIGNALSPY_H
