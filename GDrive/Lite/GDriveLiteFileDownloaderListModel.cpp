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

#include "GDriveLiteFileDownloaderListModel.h"
#include "GDriveLiteFileDownloader.h"

#include <GCF3/ObjectList>
#include <QThread>

GDriveLiteFileDownloaderListModel::GDriveLiteFileDownloaderListModel(GDriveLite *parent)
    : GCF::IGDriveLiteFileDownloaderListModel(parent), m_gDriveLite(parent)
{
#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    connect(this, SIGNAL(allJobsComplete()), this, SIGNAL(allDownloadsComplete()));
}

GDriveLiteFileDownloaderListModel::~GDriveLiteFileDownloaderListModel()
{
}

bool GDriveLiteFileDownloaderListModel::addJob(GCF::AbstractJob *job)
{
    GDriveLiteFileDownloader *downloader = qobject_cast<GDriveLiteFileDownloader*>(job);
    if(downloader)
        return GCF::JobListModel::addJob(job);

    return false;
}

QVariant GDriveLiteFileDownloaderListModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if( index.column() >= this->columns().count() )
            return QVariant();

        role = this->columns().at( index.column() );
    }

    GCF::IGDriveLiteFileDownloader *downloader = this->downloaderAt(index.row());
    switch(role)
    {
    case SourceFileId: return downloader->id();
    case SourceFileName: return downloader->item().title();
    case SourceFileIcon: return QVariant(); // not implemented
    case DownloadedFileName: return downloader->downloadedFileName();
    case Downloader: return QVariant::fromValue<QObject*>( (QObject*)downloader );
    case DownloaderTitle: role = GCF::JobListModel::Title; break;
    case DownloaderDescription: role = GCF::JobListModel::Description; break;
    case DownloaderIcon: role = GCF::JobListModel::Icon; break;
    case DownloaderIconUrl: role = GCF::JobListModel::IconUrl; break;
    case DownloaderProgress: role = GCF::JobListModel::Progress; break;
    case DownloaderStatus: role = GCF::JobListModel::Status; break;
    case DownloaderIsStarted: role = GCF::JobListModel::IsStarted; break;
    case DownloaderIsSuspended: role = GCF::JobListModel::IsSuspended; break;
    case DownloaderIsComplete: role = GCF::JobListModel::IsComplete; break;
    case DownloaderIsRunning: role = GCF::JobListModel::IsRunning; break;
    case DownloaderHasError: role = GCF::JobListModel::HasError; break;
    case DownloaderError: role = GCF::JobListModel::Error; break;
    default: break;
    }

    return GCF::IGDriveLiteFileDownloaderListModel::data(index, role);
}

QHash<int,QByteArray> GDriveLiteFileDownloaderListModel::roleNames() const
{
    static QHash<int,QByteArray> roles;
    if(roles.isEmpty())
    {
        roles = GCF::JobListModel::roleNames();

#define ADD_ROLE(x) roles[x] = "downloader" #x;
        ADD_ROLE(SourceFileId);
        ADD_ROLE(SourceFileName);
        ADD_ROLE(SourceFileIcon);
        ADD_ROLE(DownloadedFileName);
#undef ADD_ROLE

        roles[Downloader] = "downloader";

#define ADD_ROLE(x) roles[x] = #x; roles[x].remove(0, 1); roles[x].prepend('d')
        ADD_ROLE(DownloaderTitle);
        ADD_ROLE(DownloaderDescription);
        ADD_ROLE(DownloaderIcon);
        ADD_ROLE(DownloaderIconUrl);
        ADD_ROLE(DownloaderProgress);
        ADD_ROLE(DownloaderStatus);
        ADD_ROLE(DownloaderIsStarted);
        ADD_ROLE(DownloaderIsSuspended);
        ADD_ROLE(DownloaderIsComplete);
        ADD_ROLE(DownloaderIsRunning);
        ADD_ROLE(DownloaderHasError);
        ADD_ROLE(DownloaderError);
#undef ADD_ROLE
    }

    return roles;
}

GCF::IGDriveLiteFileDownloader *GDriveLiteFileDownloaderListModel::createDownloader(const QString &id)
{
    if(QThread::currentThread() != this->thread())
        return 0;

    if(!m_gDriveLite->isAuthenticated())
        return 0;

    GCF::IGDriveLiteFileDownloader *downloader = this->downloader(id);
    if(downloader)
        return downloader;

    downloader = new GDriveLiteFileDownloader(id, m_gDriveLite);
    this->addJob(downloader);

    return downloader;
}

GCF::IGDriveLiteFileDownloader *GDriveLiteFileDownloaderListModel::downloader(const QString &id) const
{
    for(int i=0; i<this->jobCount(); i++)
    {
        GCF::IGDriveLiteFileDownloader *downloader = this->downloaderAt(i);
        if(downloader->id() == id)
            return downloader;
    }

    return 0;
}
