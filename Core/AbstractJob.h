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

#ifndef JOB_H
#define JOB_H

#include "GCFGlobal.h"
#include "ObjectList.h"

#include <QObject>
#include <QAbstractListModel>

namespace GCF
{

struct AbstractJobData;
class GCF_EXPORT AbstractJob : public QObject
{
    Q_OBJECT

public:
    AbstractJob(const QString &kind, QObject *parent=nullptr);
    ~AbstractJob();

    Q_PROPERTY(QString kind READ kind CONSTANT)
    QString kind() const;

    Q_PROPERTY(QString title READ title NOTIFY updated)
    QString title() const;

    Q_PROPERTY(QString description READ description NOTIFY updated)
    QString description() const;

    Q_PROPERTY(QVariant icon READ icon NOTIFY updated)
    QVariant icon() const;

    Q_PROPERTY(QString iconUrl READ iconUrl NOTIFY updated)
    QString iconUrl() const;

    Q_PROPERTY(QString status READ status NOTIFY updated)
    QString status() const;

    Q_PROPERTY(int progress READ progress NOTIFY updated)
    int progress() const;

    Q_PROPERTY(bool started READ isStarted NOTIFY updated)
    bool isStarted() const;

    Q_PROPERTY(bool suspended READ isSuspended NOTIFY updated)
    bool isSuspended() const;

    Q_PROPERTY(bool complete READ isComplete NOTIFY updated)
    bool isComplete() const;

    Q_PROPERTY(bool running READ isRunning NOTIFY updated)
    bool isRunning() const { return this->isStarted() && !this->isComplete(); }

    Q_PROPERTY(bool hasError READ hasError NOTIFY updated)
    bool hasError() const;

    Q_PROPERTY(QString error READ error NOTIFY updated)
    QString error() const;

    GCF::Result start();
    GCF::Result cancel();
    GCF::Result suspend();
    GCF::Result resume();
    GCF::Result retry();

signals:
    void updated(GCF::AbstractJob *job);
    void started(GCF::AbstractJob *job);
    void suspended(GCF::AbstractJob *job);
    void resumed(GCF::AbstractJob *job);
    void completed(GCF::AbstractJob *job);

protected:
    virtual GCF::Result startJob() = 0;
    virtual GCF::Result cancelJob() { return false; }
    virtual GCF::Result suspendJob() { return false; }
    virtual GCF::Result resumeJob() { return false; }
    virtual GCF::Result retryJob() { return this->startJob(); }

    void setTitle(const QString &title);
    void setDescription(const QString &desc);
    void setIconUrl(const QString &iconUrl);
    void setIcon(const QVariant &icon);
    void setStatus(const QString &statusMsg);
    void setProgress(int val, const QString &msg=QString());
    void setError(const QString &errMsg, bool abort=false);
    void clearError();
    void abort(const QString &msg);
    void done();

private:
    AbstractJobData *d;
};

struct JobsListModelData;
class GCF_EXPORT JobListModel : public QAbstractListModel,
                            public GCF::ObjectListEventListener
{
    Q_OBJECT

public:
    JobListModel(QObject *parent=0);
    ~JobListModel();

    virtual bool addJob(AbstractJob *job);
    void removeJob(AbstractJob *job);

    Q_PROPERTY(int jobCount READ jobCount NOTIFY jobCountChanged)
    int jobCount() const;
    AbstractJob *jobAt(int index) const;

    Q_INVOKABLE bool containsJob(GCF::AbstractJob *job) const;
    Q_INVOKABLE int indexOfJob(GCF::AbstractJob *job) const;
    Q_INVOKABLE QObject *jobObjectAt(int index) const; // purely for access from QML/Script

    enum Field
    {
        Kind = Qt::UserRole+1,
        Title,
        Description,
        Icon,
        IconUrl,
        Progress,
        Status,
        IsStarted,
        IsSuspended,
        IsComplete,
        IsRunning,
        HasError,
        Error,
        Object
    };
    void setColumns(const QList<int> &columns);
    QList<int> columns() const;

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QHash<int,QByteArray> roleNames() const;

    Q_INVOKABLE void cancelAllJobs();
    Q_INVOKABLE void clearCompletedJobs();
    Q_INVOKABLE bool cancelJobAt(int index);
    Q_INVOKABLE bool retryJobAt(int index);
    Q_INVOKABLE bool suspendJobAt(int index);
    Q_INVOKABLE bool resumeJobAt(int index);

signals:
    void jobCountChanged();
    void allJobsComplete();
    void jobUpdated(GCF::AbstractJob *job);
    void jobStarted(GCF::AbstractJob *job);
    void jobSuspended(GCF::AbstractJob *job);
    void jobResumed(GCF::AbstractJob *job);
    void jobCompleted(GCF::AbstractJob *job);

private:
    void objectRemoved(int index, QObject *obj);
    Q_SLOT void endRemove();
    Q_SLOT void onJobUpdated();

private:
    JobsListModelData *d;
};

}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::AbstractJob*)

#endif // JOB_H
