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

#include "GDriveLiteFileUploaderListModel.h"

#include "GDriveLiteFileUploader.h"

#include <QThread>

GDriveLiteFileUploaderListModel::GDriveLiteFileUploaderListModel(GDriveLite *parent) :
    IGDriveLiteFileUploaderListModel(parent), m_gDriveLite(parent)
{
#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    connect(this, SIGNAL(allJobsComplete()), this, SIGNAL(allUploadesComplete()));
}

GDriveLiteFileUploaderListModel::~GDriveLiteFileUploaderListModel()
{
}

bool GDriveLiteFileUploaderListModel::addJob(GCF::AbstractJob *job)
{
    GDriveLiteFileUploader *uploader = qobject_cast<GDriveLiteFileUploader*>(job);
    if(uploader)
        return GCF::JobListModel::addJob(job);

    return false;
}

QVariant GDriveLiteFileUploaderListModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if( index.column() >= this->columns().count() )
            return QVariant();

        role = this->columns().at( index.column() );
    }

    GCF::IGDriveLiteFileUploader *uploader = this->uploaderAt(index.row());
    switch(role)
    {
    case UploadFolderId: return uploader->uploadFolderId();
    case UploadFileName: return uploader->uploadFileName();
    case Uploader: return QVariant::fromValue<QObject*>( (QObject*)uploader );
    case UploaderTitle: role = GCF::JobListModel::Title; break;
    case UploaderDescription: role = GCF::JobListModel::Description; break;
    case UploaderIcon: role = GCF::JobListModel::Icon; break;
    case UploaderIconUrl: role = GCF::JobListModel::IconUrl; break;
    case UploaderProgress: role = GCF::JobListModel::Progress; break;
    case UploaderStatus: role = GCF::JobListModel::Status; break;
    case UploaderIsStarted: role = GCF::JobListModel::IsStarted; break;
    case UploaderIsSuspended: role = GCF::JobListModel::IsSuspended; break;
    case UploaderIsComplete: role = GCF::JobListModel::IsComplete; break;
    case UploaderIsRunning: role = GCF::JobListModel::IsRunning; break;
    case UploaderHasError: role = GCF::JobListModel::HasError; break;
    case UploaderError: role = GCF::JobListModel::Error; break;
    default: break;
    }

    return GCF::IGDriveLiteFileUploaderListModel::data(index, role);
}

QHash<int,QByteArray> GDriveLiteFileUploaderListModel::roleNames() const
{
    static QHash<int,QByteArray> roles;
    if(roles.isEmpty())
    {
        roles = GCF::JobListModel::roleNames();

#define ADD_ROLE(x) roles[x] = "uploader" #x;
        ADD_ROLE(UploadFolderId);
        ADD_ROLE(UploadFileName);
#undef ADD_ROLE

        roles[Uploader] = "uploader";

#define ADD_ROLE(x) roles[x] = #x; roles[x].remove(0, 1); roles[x].prepend('u')
        ADD_ROLE(UploaderTitle);
        ADD_ROLE(UploaderDescription);
        ADD_ROLE(UploaderIcon);
        ADD_ROLE(UploaderIconUrl);
        ADD_ROLE(UploaderProgress);
        ADD_ROLE(UploaderStatus);
        ADD_ROLE(UploaderIsStarted);
        ADD_ROLE(UploaderIsSuspended);
        ADD_ROLE(UploaderIsComplete);
        ADD_ROLE(UploaderIsRunning);
        ADD_ROLE(UploaderHasError);
        ADD_ROLE(UploaderError);
#undef ADD_ROLE
    }

    return roles;
}

GCF::IGDriveLiteFileUploader *GDriveLiteFileUploaderListModel::createUploader(const QString &localFileName)
{
    if(QThread::currentThread() != this->thread())
        return 0;

    if(!m_gDriveLite->isAuthenticated())
        return 0;

    GCF::IGDriveLiteFileUploader *uploader = this->uploader(localFileName);
    if(uploader)
        return uploader;

    uploader = new GDriveLiteFileUploader(localFileName, m_gDriveLite);
    this->addJob(uploader);

    return uploader;
}

GCF::IGDriveLiteFileUploader *GDriveLiteFileUploaderListModel::uploader(const QString &localFileName) const
{
    for(int i=0; i<this->jobCount(); i++)
    {
        GCF::IGDriveLiteFileUploader *uploader = this->uploaderAt(i);
        if(uploader->uploadFileName() == localFileName)
            return uploader;
    }

    return 0;
}

