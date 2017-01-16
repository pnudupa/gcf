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

#include "AgentComponent.h"
#include "Agent.h"

#include <QFile>
#include <QTimer>
#include <QStack>
#include <QFileInfo>
#include <QTextStream>

#include "AgentScriptRunner.h"
#include "AgentScriptRecorder.h"

struct AgentComponentData
{
    AgentComponentData() : recorder(0) { }

    QFile outputFile;
    QString scriptFile;
    QStack<QString> logBranchStack;
    AgentScriptRecorder *recorder;
};

AgentComponent::AgentComponent(QObject *parent)
    :GCF::Component(parent)
{
    d = new AgentComponentData;
}

AgentComponent::~AgentComponent()
{
    delete d;
}

GCF::Result AgentComponent::runScript(const QString &script)
{
    AgentScriptRunner runner(this);
    this->log( QString("Running script '%1'").arg(script) );
    GCF::Result result = runner.run(script);
    return result;
}

void AgentComponent::log(const QString &msg)
{
    QTextStream ts(&d->outputFile);
    ts << QString(d->logBranchStack.size()*4, ' ') << msg << "\n";
}

void AgentComponent::pushLogBranch(const QString &name)
{
    this->log(QString("%1 {").arg(name));
    d->logBranchStack.push(name);
}

void AgentComponent::popLogBranch(const QString &msg)
{
    if( d->logBranchStack.size() )
    {
        d->logBranchStack.pop();
        if(msg.isEmpty())
            this->log("}");
        else
            this->log(QString("} %1").arg(msg));
    }
}

QString AgentComponent::objectPath(QObject *object) const
{
    return AgentScriptRecorder::objectPath(object);
}

QObject *AgentComponent::object(const QString &objectPath) const
{
    return AgentScriptRunner::findObject(objectPath);
}

void AgentComponent::initializeEvent(GCF::InitializeEvent *e)
{
    if(e->isPreInitialize())
        return;

    GCF::ObjectTreeNode *node = gAppService->objectTree()->node(this);
    node->writableInfo()["allowmetaaccess"] = true;

    // Process application arguments
    QString outputFileName = gAppService->argumentsMap().value("--outputFile").toString();
    if(outputFileName.isEmpty())
        d->outputFile.open(stdout, QFile::WriteOnly);
    else
    {
        d->outputFile.setFileName(outputFileName);
        d->outputFile.open(QFile::WriteOnly);
    }

    if(gAppService->argumentsMap().contains("--record"))
    {
        d->recorder = new AgentScriptRecorder(this);
        d->recorder->startRecording();
        this->log("Record started");
        return;
    }

    this->log( QString("********* Start testing of %1 *********").arg(gApp->applicationName()) );
    this->log( QString("Using GCF %1 on Qt %2").arg(GCF::version()).arg(QT_VERSION_STR) );

    if(gAppService->argumentsMap().contains("--testMode"))
        return;

    d->scriptFile = gAppService->argumentsMap().value("--testScript").toString();
    int startupDelay = gAppService->argumentsMap().value("--testStartDelay", 0).toInt();

    if(d->scriptFile.isEmpty())
    {
        this->log("No test-script provided");
        QTimer::singleShot(1000, qApp, SLOT(quit()));
    }
    else
    {
        this->log( QString("Scheduling test-script '%1' for execution after %2 seconds")
                   .arg(d->scriptFile).arg(startupDelay) );
        QTimer::singleShot(startupDelay*1000, this, SLOT(runScript()));
    }
}

void AgentComponent::finalizeEvent(GCF::FinalizeEvent *e)
{
    if(e->isPostFinalize())
    {
        if(d->recorder)
        {
            d->recorder->stopRecording();
            this->log("Record completed");

            QString code = d->recorder->recordedTestCaseCode();
            QString fileName = gAppService->argumentsMap().value("--record").toString();

            QFile file;
            if(fileName.isEmpty())
                file.open(stdout, QFile::WriteOnly);
            else
            {
                file.setFileName(fileName);
                file.open(QFile::WriteOnly);
                this->log( QString("Code dumped to %1").arg(fileName) );
            }

            QTextStream ts(&file);
            ts << code;

            delete d->recorder;
        }
        else
            this->log( QString("********* Finished testing of %1 *********").arg(gApp->applicationName()) );
    }
}

void AgentComponent::runScript()
{
    GCF::Result result = this->runScript(d->scriptFile);
    if(result)
        this->log("SUCCESS: " + result.message());
    else
        this->log("FAILURE: " + result.message());

    qApp->exit(result.isSuccess() ? 0 : 1);
}

GCF_EXPORT_COMPONENT(AgentComponent)

