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

/**
\class GCF::AbstractJob AbstractJob.h <GCF3/Job>
\brief This class offers an abstraction for representing and managing time-consuming
jobs in your GCF applications.
\ingroup gcf_core

Typically jobs like downloads, uploads, copy, load and save are time-consuming jobs.
You might want to offer the ability to showcase their job progress and also the ability
to pause, resume or even cancel such jobs.

The job can be started using \ref start(). The \ref suspend() and \ref resume() methods
help with suspend and resume of the job. You can call the \ref cancel() method to
cancel the job entirely. Once cancelled the job can be retried using \ref retry().

Methods \ref kind(), \ref title(), \ref description(), \ref icon(), \ref iconUrl()
\ref status(), \ref progress() return various attributes of the job. Status query
methods \ref isSuspended(), \ref isStarted(), \ref isCompleted() and \ref isRunning()
help you to determine the state of the job. If an error was encountered during the
execution, the \ref hasError() method would return true and the \ref error() method
will return a string description of the error.

Instances of this class are automatically registered with a global \ref GCF::JobListModel "job-list"
accessible from \ref GCF::ApplicationServices::jobs(). You can include jobs into your
own instance of \ref GCF::JobListModel if you want. In which case the job will be available
in both the global list and your local list.

If you are subclassing from this class, then you will need to keep the following in mind

\li reimplement the \ref startJob() method to trigger the start of your job.
\li optionally reimplement \ref suspendJob() and \ref resumeJob() to support suspension
and resume of your jobs.
\li optionally reimplement \ref cancelJob() to offer means to cancel the job entirely
\li optionally reimplement \ref retryJob() to offer means to customize the way
in which a job gets retried after a cancel.
\li once your job is started (after call to \ref startJob() ), you must notify progress
updates by calling \ref setProgress() regularly. You must notify job completion by
calling the \ref done() method. Jobs can be abruptly aborted using the \ref abort()
method. You can also notify non-critical error messages using the \ref setError() method.

*/

#include "AbstractJob.h"
#include "Application.h"

namespace GCF
{

struct AbstractJobData
{
    AbstractJobData(const QString &k) : kind(k),
        progress(0), started(false),
        suspended(false), complete(false),
        hasError(false) { }

    const QString kind;
    QString title, description, iconUrl, status, error;
    QVariant icon;
    int progress;
    bool started, suspended, complete, hasError;
};

}

/**
Constructor. It takes the kind of task as parameter. \c kind is like
 category of the task. For example kind may be "Download" or "Upload".

\param kind categorization of job.
*/
GCF::AbstractJob::AbstractJob(const QString &kind, QObject *parent)
    : QObject(parent)
{
    d = new AbstractJobData(kind);
    gAppService->jobs()->addJob(this);
}

/**
Destructor.
*/
GCF::AbstractJob::~AbstractJob()
{
    delete d;
}

/**
\return the kind of job. Kind is the category of the task. For example
kind may be "Download" or "Upload". By default it is an empty string.
*/
QString GCF::AbstractJob::kind() const { return d->kind; }

/**
\return the title of job. By default it is an empty string.
*/
QString GCF::AbstractJob::title() const { return d->title; }

/**
\return description of job. By default it is an empty string.
*/
QString GCF::AbstractJob::description() const { return d->description; }

/**
\return the icon of job. By default it returns QIcon().
*/
QVariant GCF::AbstractJob::icon() const { return d->icon; }

/**
\return the icon url of job. By default it is an empty string.
*/
QString GCF::AbstractJob::iconUrl() const { return d->iconUrl; }

/**
\return the status of job. By default it is an empty string.
*/
QString GCF::AbstractJob::status() const { return d->status; }

/**
\return the current progress information of job. By default it is 0.
*/
int GCF::AbstractJob::progress() const { return d->progress; }

/**
\return true if job has been started otherwise returns false.
*/
bool GCF::AbstractJob::isStarted() const { return d->started; }

/**
\return whether the job is suspended or not. Job will be in suspended state
when user has suspended the job by calling the method \ref GCF::AbstractJob::suspend().
*/
bool GCF::AbstractJob::isSuspended() const { return d->suspended; }

/**
\return true if the job has completed otherwise returns false. Job is
completed when the \ref GCF::AbstractJob::done() method is called.
*/
bool GCF::AbstractJob::isComplete() const { return d->complete; }

/**
\return true if the job has otherwise returns false.
*/

bool GCF::AbstractJob::hasError() const { return d->hasError; }

/**
\return the error occurred while executing the job. By default it is an empty
string.
*/
QString GCF::AbstractJob::error() const { return d->error; }

/**
Starts the job. If the job is already started or completed, then returns
GCF::Result with status false and error message. Otherwise starts the job by
using virtual function \ref GCF::AbstractJob::startJob().
\return \ref GCF::Result which information about starting of job was
successful or not.
*/
GCF::Result GCF::AbstractJob::start()
{
    if(d->complete)
        return GCF::Result(false, QString(), "Job is already complete");

    if(d->started)
        return GCF::Result(false, QString(), "Job has already started");

    GCF::Result result = this->startJob();
    d->started = result.isSuccess();

    if(!d->started)
    {
        d->hasError = true;
        d->error = result.message();
    }
    else
    {
        d->progress = 0;
        d->hasError = false;
        d->error.clear();
    }

    emit updated(this);
    if(d->started)
        emit started(this);

    return result;
}

/**
Cancels the job. If the job is already complete or has not yet started, then
returns \ref GCF::Result with status false and error message. Otherwise cancels
the job by using virtual function \ref GCF::AbstractJob::cancelJob().
\return \ref GCF::Result which contains success or failure information
with error messages.
*/
GCF::Result GCF::AbstractJob::cancel()
{
    if(d->complete)
        return GCF::Result(false, QString(), "Completed jobs cannot be cancelled");

    if(!d->started)
        return GCF::Result(false, QString(), "Job cannot be cancelled unless started");

    GCF::Result result = this->cancelJob();
    if(result.isSuccess())
    {
        d->suspended = false;
        d->complete = true;
        d->hasError = true;
        d->error = "Cancelled";
        d->status = d->error;
    }
    else
    {
        d->complete = false;
        d->hasError = false;
        d->error.clear();
    }

    emit updated(this);

    if(d->complete)
        emit completed(this);

    return result;
}

/**
Suspends the job. If the job is already suspended, completed or has not yet
started, then returns \c GCF::Result with status false and error message otherwise
suspends the job by calling virtual function \ref GCF::AbstractJob::suspendJob().

\return \ref GCF::Result which contains success or failure information
with error messages.
*/
GCF::Result GCF::AbstractJob::suspend()
{
    if(d->suspended)
        return GCF::Result(false, QString(), "Job is already suspended");

    if(!d->started)
        return GCF::Result(false, QString(), "Cannot suspend the job before it is started");

    if(d->complete)
        return GCF::Result(false, QString(), "Completed jobs cannot be suspended");

    GCF::Result result = this->suspendJob();
    if(result.isSuccess())
    {
        d->suspended = true;
        d->status = "Suspended";
        emit updated(this);
        emit suspended(this);
    }

    return result;
}

/**
Resumes the job. If the job is not already in suspended state, then it returns
\c GCF::Result with status false and with proper error message. Otherwise
resumes the job by calling virtual function \ref GCF::AbstractJob::resumeJob().

\return \ref GCF::Result which contains success or failure information
with proper error messages.
*/
GCF::Result GCF::AbstractJob::resume()
{
    if(!d->suspended)
        return GCF::Result(false, QString(), "Job is not suspended for resume");

    GCF::Result result = this->resumeJob();

    if(result.isSuccess())
    {
        d->suspended = false;
        d->status = "Resumed";
        emit updated(this);
        emit resumed(this);
    }

    return result;
}

/**
Retries to complete the job by calling virtual function
\ref GCF::AbstractJob::start() once
again. If the job was not even started once, then it returns
\c GCF::Result with status false and with proper messages.

\return \ref GCF::Result which contains success or failure information
with proper error messages.
*/
GCF::Result GCF::AbstractJob::retry()
{
    if(!d->started)
        return GCF::Result(false, QString(), "Job was not started even once to retry");

    if(d->started && d->complete)
    {
        d->started = false;
        d->complete = false;
        d->error.clear();
        d->hasError = false;
        d->progress = 0;
        d->status.clear();
        emit updated(this);

        GCF::Result result = this->retryJob();
        d->started = result.isSuccess();

        if(!d->started)
        {
            d->hasError = true;
            d->error = result.message();
        }
        else
        {
            d->hasError = false;
            d->error.clear();
        }

        emit updated(this);
        if(d->started)
            emit started(this);

        return result;
    }

    return GCF::Result(false, QString(), "Could not retry because the job is still running!");
}

/**
\fn GCF::AbstractJob::startJob() = 0;
This is a pure virtual function. All jobs inheriting from \c AbstractJob
should reimplement this function and  start the processing of their task inside
this method.
*/
/**
\fn GCF::AbstractJob::cancelJob();
Classes inheriting from \c AbstractJob should reimplement this function
to provide the cancelling option to their task.
*/
/**
\fn GCF::AbstractJob::suspendJob() = 0;
Classes inheriting from \c AbstractJob should reimplement this function
to provide the suspending option to their task.
*/
/**
\fn GCF::AbstractJob::resumeJob() = 0;
Classes inheriting from \c AbstractJob should reimplement this function
to provide the resume option to their task.
*/

/**
Sets the title of the job as \c title.
\param title title of the job.
*/
void GCF::AbstractJob::setTitle(const QString &title)
{
    d->title = title;
    emit updated(this);
}

/**
Sets the description of the job as \c desc.
\param desc description of the job.
*/
void GCF::AbstractJob::setDescription(const QString &desc)
{
    d->description = desc;
    emit updated(this);
}

/**
Sets the icon url of job as \c iconUrl.
\param iconUrl icon url of the job.
*/
void GCF::AbstractJob::setIconUrl(const QString &iconUrl)
{
    d->iconUrl = iconUrl;
    emit updated(this);
}

/**
Sets the icon of job as \c icon.
\param icon icon of the job.
*/
void GCF::AbstractJob::setIcon(const QVariant &icon)
{
    // We can accept only QIcon
    if( !qstrcmp(icon.typeName(), "QIcon") )
        d->icon = icon;
    else
        d->icon = QVariant();

    emit updated(this);
}

/**
Sets the status message of job as \c statusMsg.
\param statusMsg status message of the job.
*/
void GCF::AbstractJob::setStatus(const QString &statusMsg)
{
    d->status = statusMsg;
    emit updated(this);
}

/**
Sets the progress of job to value \c val and progress message 'msg'.
\param val progress value of job in percentage .
\param msg progress message of job.
*/
void GCF::AbstractJob::setProgress(int val, const QString &msg)
{
    if(d->started && !d->suspended && !d->complete)
    {
        d->progress = val;

        if(!msg.isNull()) // We are checking isNull() [not isEmpty()] on purpose
            d->status = msg;

        emit updated(this);
    }
}

/**
Sets the error message of job as \c errMsg . In case if job was aborted, this
method will also set the job to be complete.
\param errMsg error message of job.
\param abort whether the job was aborted.
*/
void GCF::AbstractJob::setError(const QString &errMsg, bool abort)
{
    d->hasError = true;
    d->error = errMsg;

    if(abort && d->started)
        d->complete = true;

    emit updated(this);

    if(d->complete && d->started)
        emit completed(this);
}

/**
Clears the error message of job.
*/
void GCF::AbstractJob::clearError()
{
    d->hasError = false;
    d->error.clear();
    emit updated(this);
}

/**
Aborts the job and sets the error message as \c msg.

\param msg error message that needs to be set while aborting.
*/
void GCF::AbstractJob::abort(const QString &msg)
{
    this->setError(msg, true);
}

/**
Sets the job as completed with progress information to 100 percent. It will also
clear the errors. This method should be called when the job has been completed.
*/
void GCF::AbstractJob::done()
{
    if(!d->started || d->suspended || d->complete)
        return;

    d->error.clear();
    d->hasError = false;
    d->complete = true;
    d->progress = 100;
    emit updated(this);
    emit completed(this);
}

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::JobListModel AbstractJob.h <GCF3/Job>
\brief This model provides a model representation for a configured
 list of \ref GCF::AbstractJob.
\ingroup gcf_core


It emits signals on behalf of the jobs stored in it. For example when
a job added to job list model gets completed, job list model will emit the signal
\ref GCF::AbstractJob::jobCompleted(AbstractJob *job) with parameter as
completed job pointer.

You can add jobs to job list model using
\ref GCF::JobListModel::addJob(AbstractJob *job) and remove a job from the model
using \ref GCF::JobListModel::removeJob(AbstractJob *job).

JobListModel can be queried for data using the
\ref GCF::JobListModel::data(const QModelIndex &index, int role) method by providing
an appropriate \c index and corresponding \ref GCF::JobListModel::Field for the
required information.

You can control the columns available in the model, and its order, by making use of the
\ref GCF::JobListModel::setColumns(const QList<int> &columns) method.

When the JobListModel is deleted it will cancel and delete all the jobs in it.
*/

namespace GCF
{

struct JobsListModelData
{
    GCF::ObjectList jobs;
    QList<int> columns;

    int insertIndex(const GCF::AbstractJob *newJob) const {
        for(int i=0; i<jobs.count(); i++) {
            const GCF::AbstractJob *job = (GCF::AbstractJob*)jobs.at(i);
            if(job->kind() == newJob->kind())
                return i;
        }
        return jobs.count();
    }
};

}

/**
Constructor.
*/
GCF::JobListModel::JobListModel(QObject *parent)
    :QAbstractListModel(parent)
{
    d = new JobsListModelData;

#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    d->jobs.setEventListener(this);
}

/**
Destructor.
*/
GCF::JobListModel::~JobListModel()
{
    while(d->jobs.count())
    {
        GCF::AbstractJob *job = (GCF::AbstractJob *)d->jobs.first();
        job->cancel();
        job->deleteLater();
    }

    delete d;
}

/**
Adds a job to the job list model.
\param job \ref GCF::AbstractJob that needs to be added.
*/
bool GCF::JobListModel::addJob(AbstractJob *job)
{
    if(d->jobs.contains(job))
        return false;

    int index = d->insertIndex(job);

    this->beginInsertRows(QModelIndex(), index, index);
    connect(job, SIGNAL(updated(GCF::AbstractJob*)), this, SLOT(onJobUpdated()));
    connect(job, SIGNAL(updated(GCF::AbstractJob*)), this, SIGNAL(jobUpdated(GCF::AbstractJob*)));
    connect(job, SIGNAL(started(GCF::AbstractJob*)), this, SIGNAL(jobStarted(GCF::AbstractJob*)));
    connect(job, SIGNAL(suspended(GCF::AbstractJob*)), this, SIGNAL(jobSuspended(GCF::AbstractJob*)));
    connect(job, SIGNAL(resumed(GCF::AbstractJob*)), this, SIGNAL(jobResumed(GCF::AbstractJob*)));
    connect(job, SIGNAL(completed(GCF::AbstractJob*)), this, SIGNAL(jobCompleted(GCF::AbstractJob*)));
    d->jobs.insert(index, job);
    this->endInsertRows();

    emit jobCountChanged();

    return true;
}

/**
Removes a job from the job list model.
\param job \ref GCF::AbstractJob that needs to be removed.
*/
void GCF::JobListModel::removeJob(GCF::AbstractJob *job)
{
    d->jobs.remove(job);
}

/**
\return the count of all jobs in the model.
*/
int GCF::JobListModel::jobCount() const
{
    return d->jobs.count();
}

/**
\return the job at the index in the model.
*/
GCF::AbstractJob *GCF::JobListModel::jobAt(int index) const
{
    if(index < 0 || index >= d->jobs.count())
        return 0;

    return (GCF::AbstractJob*)(d->jobs.at(index));
}

/**
\return true if the model contains the \c job otherwise false.
*/
bool GCF::JobListModel::containsJob(GCF::AbstractJob *job) const
{
    return d->jobs.contains(job);
}

/**
\return the index of job in the model.
*/
int GCF::JobListModel::indexOfJob(GCF::AbstractJob *job) const
{
    return d->jobs.indexOf(job);
}

/**
\return the job at the specified \c index in the model.
*/
QObject *GCF::JobListModel::jobObjectAt(int index) const
{
    return d->jobs.at(index);
}

/*! \enum GCF::JobListModel::Field
The enumeration values of Field represents a specific information about
\ref GCF::AbstractJob. This enumeration is
mainly used to fetch data about \ref GCF::AbstractJob stored inside the model.
Also this is used to construct columns of this model.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Kind
 * kind of job. Accessible using role name jobKind.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Title
 * title of job. Accessible using role name jobTitle.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Description
 * description of job. Accessible using role name jobDescription.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Icon
 * icon of job. Accessible using role name jobIcon.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::IconUrl
 * icon url of job. Accessible using role name jobIconUrl.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Progress
 * current progress value of job in percentage. Accessible using role name
 * jobProgress.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Status
 * status message of job. Accessible using role name jobStatus.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::IsStarted
 * job has started or not.  Accessible using role name jobIsStarted.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::IsSuspended
 * job is suspended or not. Accessible using role name jobIsSuspended.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::IsComplete
 * job is complete or not. Accessible using role name jobIsComplete.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::IsRunning
 * job is running or not. Accessible using role name jobIsRunning.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::HasError
 * job has error while executing or not. Accessible using role name jobHasError.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Error
 * the error set on job. Accessible using role name jobError.
 */

/*! \var GCF::JobListModel::Field GCF::JobListModel::Object
 * represents the job object. Accessible using role name jobObject.
 */

/**
Sets the columns of this according to \c fields.

\param fields list of enumeration values of \ref GCF::JobListModel::Field based
upon which model columns will be constructed.
*/
void GCF::JobListModel::setColumns(const QList<int> &fields)
{
    this->beginResetModel();
    d->columns = fields;
    this->endResetModel();
}

/**
Returns the columns of job list model.
\return \c QList<int> list of enumeration values of
\ref GCF::JobListModel::Field.
*/
QList<int> GCF::JobListModel::columns() const
{
    return d->columns;
}

/**
Reimplementation of \c QAbstractItemModel::rowCount(const QModelIndex
&parent) const.

\return the number of jobs in the model.
*/
int GCF::JobListModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return d->jobs.count();
}

/**
Reimplementation of \c QAbstractItemModel::columnCount(const QModelIndex
&parent) const.
*/
int GCF::JobListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->columns.count() ? d->columns.count() : 1;
}

/**
Reimplementation of \c QAbstractItemModel::data(const QModelIndex &index,
int role) const

Returns information about \ref GCF::AbstractJob based on the \c role
provided with the \c index parameter. Here \c role is an enumeration value in
\ref GCF::JobListModel::Field.

\param index index of the job
\param role \ref GCF::JobListModel::Field which represents specific data
about \ref GCF::AbstractJob.
*/
QVariant GCF::JobListModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        if(d->columns.count())
            role = d->columns.at( index.column() );
        else
            role = Title;
    }

    GCF::AbstractJob *job = this->jobAt(index.row());
    switch(role)
    {
    case Kind: return job->kind();
    case Title: return job->title();
    case Description: return job->description();
    case Icon: return job->icon();
    case IconUrl: return job->iconUrl();
    case Progress: return job->progress();
    case Status: return job->status();
    case IsStarted: return job->isStarted();
    case IsSuspended: return job->isSuspended();
    case IsComplete: return job->isComplete();
    case IsRunning: return job->isRunning();
    case HasError: return job->hasError();
    case Error: return job->error();
    case Object: return QVariant::fromValue<QObject*>( (QObject*)job );
    default: break;
    }

    return QVariant();
}

/**
Reimplementation of \c QAbstractItemModel::headerData(const QModelIndex &index,
int role) const
*/
QVariant GCF::JobListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if(section < 0 || section >= d->columns.count())
            return QVariant();

        int field = Title;
        if(d->columns.count())
            field = d->columns.at(section);

        QHash<int,QByteArray> roles = this->roleNames();
        QString header = QString::fromLatin1( roles.value(field,"jobUnknown") );
        header = header.remove(0, 3);
        return header;
    }

    return QString();
}

/**
Reimplementation of \c QAbstractItemModel::roleNames() const
*/
QHash<int,QByteArray> GCF::JobListModel::roleNames() const
{
    static QHash<int,QByteArray> roles;
    if(roles.isEmpty())
    {
#define ADD_ROLE(x) roles[x] = "job" #x;
        ADD_ROLE(Kind);
        ADD_ROLE(Title);
        ADD_ROLE(Description);
        ADD_ROLE(Icon);
        ADD_ROLE(IconUrl);
        ADD_ROLE(Progress);
        ADD_ROLE(Status);
        ADD_ROLE(IsStarted);
        ADD_ROLE(IsSuspended);
        ADD_ROLE(IsComplete);
        ADD_ROLE(IsRunning);
        ADD_ROLE(HasError);
        ADD_ROLE(Error);
        ADD_ROLE(Object);
#undef ADD_ROLE
    }

    return roles;
}

/**
Cancels all jobs stored in the model.
*/
void GCF::JobListModel::cancelAllJobs()
{
    for(int i=0; i<d->jobs.count(); i++)
    {
        GCF::AbstractJob *job = (GCF::AbstractJob*)d->jobs.at(i);
        if(job->isRunning())
            job->cancel();
    }
}

/**
Removes all completed jobs from the model.
*/
void GCF::JobListModel::clearCompletedJobs()
{
    for(int i=d->jobs.count()-1; i>=0; i--)
    {
        GCF::AbstractJob *job = (GCF::AbstractJob*)d->jobs.at(i);
        if(job->isRunning())
            continue;

        this->beginRemoveRows(QModelIndex(), i, i);
        d->jobs.setEventListener(0);
        d->jobs.remove(job);
        d->jobs.setEventListener(this);
        job->deleteLater();
        this->endRemoveRows();
    }
}

/**
Cancels job at specified index.
\return true if job was cancelled successfully.
*/
bool GCF::JobListModel::cancelJobAt(int index)
{
    GCF::AbstractJob *job = this->jobAt(index);
    if(job)
        return job->cancel();

    return false;
}

/**
Retries to complete the job at specified index.
\return true is retrying job was successful.
*/
bool GCF::JobListModel::retryJobAt(int index)
{
    GCF::AbstractJob *job = this->jobAt(index);
    if(job)
        return job->retry();

    return false;
}

/**
Suspends the job at specified index.
\return true is suspending job was successful.
*/
bool GCF::JobListModel::suspendJobAt(int index)
{
    GCF::AbstractJob *job = this->jobAt(index);
    if(job)
        return job->suspend();

    return false;
}

/**
Resumes the job at specified index.
\return true is resuming job was successful.
*/
bool GCF::JobListModel::resumeJobAt(int index)
{
    GCF::AbstractJob *job = this->jobAt(index);
    if(job)
        return job->resume();

    return false;
}

/**
\fn GCF::JobListModel::jobCountChanged()
This signal is emitted when a job is added or removed to or from the model.
*/

/**
\fn GCF::JobListModel::allJobsComplete()
This signal is emitted when all jobs in the model have completed execution.
*/

/**
\fn GCF::JobListModel::jobUpdated(GCF::AbstractJob *job)
This signal is emitted when a job updates its state.
*/

/**
\fn GCF::JobListModel::jobStarted(GCF::AbstractJob *job)
This signal is emitted when a job in the model has started execution.
*/

/**
\fn GCF::JobListModel::jobSuspended(GCF::AbstractJob *job)
This signal is emitted when a job in the model has been suspended.
*/

/**
\fn GCF::JobListModel::jobResumed(GCF::AbstractJob *job)
This signal is emitted when a job in the model has been resumed.
*/

/**
\fn GCF::JobListModel::jobCompleted(GCF::AbstractJob *job)
This signal is emitted when a job in the model got completed.
*/

void GCF::JobListModel::objectRemoved(int index, QObject *obj)
{
    disconnect(obj, 0, this, 0);
    this->beginRemoveRows(QModelIndex(), index, index);
    QMetaObject::invokeMethod(this, "endRemove", Qt::QueuedConnection);
}

void GCF::JobListModel::endRemove()
{
    this->endRemoveRows();
    emit jobCountChanged();
}

void GCF::JobListModel::onJobUpdated()
{
    int index = d->jobs.indexOf(this->sender());
    if(index < 0)
        return;

    QModelIndex start = this->index(index, 0);
    QModelIndex end = this->index(index, d->columns.count() ? d->columns.count()-1 : 0);
    emit dataChanged(start, end);

    for(int i=0; i<d->jobs.count(); i++)
    {
        GCF::AbstractJob *job = (GCF::AbstractJob *)d->jobs.at(i);
        if(!job->isComplete())
            return;
    }

    emit allJobsComplete();
}
