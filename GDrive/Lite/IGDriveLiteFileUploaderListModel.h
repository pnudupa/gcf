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

#ifndef IGDRIVELITEFILEUPLOADERLISTMODEL_H
#define IGDRIVELITEFILEUPLOADERLISTMODEL_H

#include "IGDriveLiteFileUploader.h"
#include <GCF3/JobListModel>

namespace GCF
{

class IGDriveLiteFileUploaderListModel : public GCF::JobListModel
{
public:
    enum Field
    {
        UploadFolderId = GCF::JobListModel::Object+1,
        UploadFileName,
        Uploader,

        // Short-hands for roles from base class
        UploaderTitle,
        UploaderDescription,
        UploaderIcon,
        UploaderIconUrl,
        UploaderProgress,
        UploaderStatus,
        UploaderIsStarted,
        UploaderIsSuspended,
        UploaderIsComplete,
        UploaderIsRunning,
        UploaderHasError,
        UploaderError
    };

    virtual GCF::IGDriveLiteFileUploader *uploader(const QString &localFileName) const = 0;
    virtual GCF::IGDriveLiteFileUploader *uploaderAt(int index) const {
        return (GCF::IGDriveLiteFileUploader*)this->jobAt(index);
    }
    Q_INVOKABLE virtual int uploaderCount() const = 0;
    Q_SIGNAL virtual void allUploadesComplete() = 0;

    Q_INVOKABLE virtual void cancelAllUploades() = 0;
    Q_INVOKABLE virtual void clearCompletedUploades() = 0;
    Q_INVOKABLE virtual bool abortUploadAt(int index) = 0;
    Q_INVOKABLE virtual bool retryUploadAt(int index) = 0;
    Q_INVOKABLE virtual bool suspendUploadAt(int index) = 0;
    Q_INVOKABLE virtual bool resumeUploadAt(int index) = 0;

protected:
    IGDriveLiteFileUploaderListModel(QObject *parent=0) : GCF::JobListModel(parent) { }
    ~IGDriveLiteFileUploaderListModel() { }
};

}

#endif // IGDRIVELITEFILEUPLOADERLISTMODEL_H
