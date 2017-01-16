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

#ifndef AGENTCOMPONENT_H
#define AGENTCOMPONENT_H

#include <GCF3/Component>

struct AgentComponentData;
class AgentComponent : public GCF::Component
{
    Q_OBJECT

public:
    AgentComponent(QObject *parent=0);
    ~AgentComponent();

    Q_INVOKABLE GCF::Result runScript(const QString &script);
    void log(const QString &msg);

    void pushLogBranch(const QString &name);
    void popLogBranch(const QString &msg=QString());

    // These two functions are for making Agent code testable.
    Q_INVOKABLE QString objectPath(QObject *object) const;
    Q_INVOKABLE QObject *object(const QString &objectPath) const;

protected:
    void initializeEvent(GCF::InitializeEvent *e);
    void finalizeEvent(GCF::FinalizeEvent *e);
    Q_SLOT void runScript();

private:
    AgentComponentData *d;
};

#endif // AGENTCOMPONENT_H
