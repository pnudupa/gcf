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

#ifndef GDRIVELITEFILEUPLOADERLISTMODEL_H
#define GDRIVELITEFILEUPLOADERLISTMODEL_H

#include "GDriveLite.h"
#include "IGDriveLiteFileUploaderListModel.h"

#include <QObject>

class GDriveLiteFileUploaderListModel : public GCF::IGDriveLiteFileUploaderListModel
{
    Q_OBJECT
public:
    GDriveLiteFileUploaderListModel(GDriveLite *parent);
    ~GDriveLiteFileUploaderListModel();

    bool addJob(GCF::AbstractJob *job);

    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    GCF::IGDriveLiteFileUploader *createUploader(const QString &localFileName);
    GCF::IGDriveLiteFileUploader *uploader(const QString &localFileName) const;

    Q_INVOKABLE int uploaderCount() const { return this->jobCount(); }
    Q_SIGNAL void allUploadesComplete();

    Q_INVOKABLE void cancelAllUploades() { this->cancelAllJobs(); }
    Q_INVOKABLE void clearCompletedUploades() { this->clearCompletedJobs(); }
    Q_INVOKABLE bool abortUploadAt(int index) { return this->cancelJobAt(index); }
    Q_INVOKABLE bool retryUploadAt(int index) { return this->retryJobAt(index); }
    Q_INVOKABLE bool suspendUploadAt(int index) { return this->suspendJobAt(index); }
    Q_INVOKABLE bool resumeUploadAt(int index) { return this->resumeJobAt(index); }

private:
    GDriveLite *m_gDriveLite;
};

#endif // GDRIVELITEFILEUPLOADERLISTMODEL_H
