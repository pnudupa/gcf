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

#ifndef GDRIVELITEFILEDOWNLOADERLISTMODEL_H
#define GDRIVELITEFILEDOWNLOADERLISTMODEL_H

#include "IGDriveLiteFileDownloaderListModel.h"
#include "GDriveLite.h"
#include <GCF3/ObjectList>

class GDriveLiteFileDownloaderListModel : public GCF::IGDriveLiteFileDownloaderListModel
{
    Q_OBJECT

public:
    GDriveLiteFileDownloaderListModel(GDriveLite *parent=0);
    ~GDriveLiteFileDownloaderListModel();

    bool addJob(GCF::AbstractJob *job);

    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    GCF::IGDriveLiteFileDownloader *createDownloader(const QString &id);

    GCF::IGDriveLiteFileDownloader *downloader(const QString &id) const;
    Q_INVOKABLE int downloaderCount() const { return this->jobCount(); }
    Q_SIGNAL void allDownloadsComplete();

    Q_INVOKABLE void cancelAllDownloads() { this->cancelAllJobs(); }
    Q_INVOKABLE void clearCompletedDownloads() { this->clearCompletedJobs(); }
    Q_INVOKABLE bool abortDownloadAt(int index) { return this->cancelJobAt(index); }
    Q_INVOKABLE bool retryDownloadAt(int index) { return this->retryJobAt(index); }
    Q_INVOKABLE bool suspendDownloadAt(int index) { return this->suspendJobAt(index); }
    Q_INVOKABLE bool resumeDownloadAt(int index) { return this->resumeJobAt(index); }

private:
    GDriveLite *m_gDriveLite;
};

#endif // GDriveLiteFileDownloaderListModel_H
