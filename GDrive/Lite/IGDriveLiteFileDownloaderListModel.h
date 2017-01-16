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

#ifndef IGDriveLiteFileDownloaderListModel_H
#define IGDriveLiteFileDownloaderListModel_H

#include "IGDriveLiteFileDownloader.h"
#include <GCF3/JobListModel>

namespace GCF
{

class IGDriveLiteFileDownloaderListModel : public GCF::JobListModel
{
public:
    enum Field
    {
        SourceFileId = GCF::JobListModel::Object+1,
        SourceFileName,
        SourceFileIcon,
        DownloadedFileName,
        Downloader,

        // Short-hands for roles from base class
        DownloaderTitle,
        DownloaderDescription,
        DownloaderIcon,
        DownloaderIconUrl,
        DownloaderProgress,
        DownloaderStatus,
        DownloaderIsStarted,
        DownloaderIsSuspended,
        DownloaderIsComplete,
        DownloaderIsRunning,
        DownloaderHasError,
        DownloaderError
    };

    virtual GCF::IGDriveLiteFileDownloader *downloader(const QString &id) const = 0;
    GCF::IGDriveLiteFileDownloader *downloaderAt(int index) const {
        return (GCF::IGDriveLiteFileDownloader*)this->jobAt(index);
    }
    Q_INVOKABLE virtual int downloaderCount() const = 0;
    Q_SIGNAL virtual void allDownloadsComplete() = 0;

    Q_INVOKABLE virtual void cancelAllDownloads() = 0;
    Q_INVOKABLE virtual void clearCompletedDownloads() = 0;
    Q_INVOKABLE virtual bool abortDownloadAt(int index) = 0;
    Q_INVOKABLE virtual bool retryDownloadAt(int index) = 0;
    Q_INVOKABLE virtual bool suspendDownloadAt(int index) = 0;
    Q_INVOKABLE virtual bool resumeDownloadAt(int index) = 0;

protected:
    IGDriveLiteFileDownloaderListModel(QObject *parent=0) : GCF::JobListModel(parent) { }
    ~IGDriveLiteFileDownloaderListModel() { }
};

}

#endif // IGDRIVELITEFILEDOWNLOADERMODEL_H
