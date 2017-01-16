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

#include "GDriveLiteHelper.h"
#include "GDriveLiteContentListModel.h"
#include "ThumbnailImageProvider.h"
#include "QmlDialog.h"

#include <QNetworkRequest>
#include <QPointer>
#include <QFileInfo>
#include <QEventLoop>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include <GCF3/IGDriveLite>
#include <GCF3/IGDriveLiteContentModel>
#include <GCF3/QmlApplication>

struct GDriveLiteHelperData
{
    GDriveLiteHelperData() : titleShowsUserInfo(false)  { }

    GCF::IGDriveLite *gDrive;
    bool titleShowsUserInfo;

    QPointer<QQuickView> jobsWindow;
    QPointer<QQuickView> shareWindow;
    QPointer<QQuickView> moveWindow;

    QString curMoveItemId;
};

GDriveLiteHelper::GDriveLiteHelper(GCF::IGDriveLite *gDrive)
{
    d = new GDriveLiteHelperData;
    d->gDrive = gDrive;
}

GDriveLiteHelper::~GDriveLiteHelper()
{
    delete d;
}

GCF::IGDriveLite *GDriveLiteHelper::gDrive() const
{
    return d->gDrive;
}

void GDriveLiteHelper::addSharePermission(QObject *object,
                                          const QString &emailAddress,
                                          const bool &isNotify,
                                          const QString &msg,
                                          const int &permision)
{
    // FIXME: Using dynamic_cast because qobject_cast cannot be used here
    GCF::IGDriveLiteContentSharingModel *sharingModel
            = dynamic_cast<GCF::IGDriveLiteContentSharingModel*>(object);
    if(!sharingModel)
        return;

    GCF::GDriveContent::Item contentItem = sharingModel->contentItem();

    GCF::GDriveLiteShareRequest request;
    request.EMailAddress = emailAddress;
    request.SendNotificationEMails = isNotify;
    request.EMailMessage = msg;
    if(permision == 1)
        request.Role = GCF::IGDriveLiteContentSharingModel::Reader;
    else if(permision == 2)
        request.Role = GCF::IGDriveLiteContentSharingModel::Writer;
    else if(permision == 3)
        request.Role = GCF::IGDriveLiteContentSharingModel::Writer|
                GCF::IGDriveLiteContentSharingModel::Commenter;

    GCF::Result result = sharingModel->share(request);

    if(!result.isSuccess())
        messageBox("Error", QString("An error occured while sharing file '%1' with '%2'<br><br>%3")
                   .arg(contentItem.title()).arg(request.EMailAddress)
                   .arg(result.message()), false);
}

void GDriveLiteHelper::removeSharePermission(QObject *object, int index)
{
    // FIXME: Using dynamic_cast because qobject_cast cannot be used here
    GCF::IGDriveLiteContentSharingModel *sharingModel
            = dynamic_cast<GCF::IGDriveLiteContentSharingModel*>(object);
    if(!sharingModel)
        return;

    QModelIndex modelIndex = sharingModel->index(index, 0);
    if(!modelIndex.isValid())
        return;

    GCF::GDriveContent::Item contentItem = sharingModel->contentItem();
    QString shareUser = modelIndex.data(GCF::IGDriveLiteContentSharingModel::Title).toString();

    if( messageBox("Unshare confirmation", QString("Are you sure you want to stop sharing the file '%1' with '%2'?")
                   .arg(contentItem.title()).arg(shareUser)) )
    {
        GCF::Result result = sharingModel->unshare(modelIndex);
        if(result.isSuccess())
            return;

        messageBox("Error", QString("An error occured while removing share permission on '%1' for '%2'<br><br>%3")
                   .arg(contentItem.title()).arg(shareUser).arg(result.message()), false);
    }
}

void GDriveLiteHelper::updateSharePermisssion(QObject *object, int index, const int &permision)
{
    // FIXME: Using dynamic_cast because qobject_cast cannot be used here
    GCF::IGDriveLiteContentSharingModel *sharingModel
            = dynamic_cast<GCF::IGDriveLiteContentSharingModel*>(object);
    if(!sharingModel)
        return;

    QModelIndex modelIndex = sharingModel->index(index, 0);
    if(!modelIndex.isValid())
        return;

    GCF::GDriveContent::Item contentItem = sharingModel->contentItem();
    QString shareUser = modelIndex.data(GCF::IGDriveLiteContentSharingModel::Title).toString();

    if(messageBox("Update share permissions",  QString("Update share permissions on file '%1' for user '%2'")
                  .arg(contentItem.title()).arg(shareUser)))

    {
        int role = GCF::IGDriveLiteContentSharingModel::Unknown;
        if(permision == 1)
            role = GCF::IGDriveLiteContentSharingModel::Reader;
        else if(permision == 2)
            role = GCF::IGDriveLiteContentSharingModel::Writer;
        else if(permision == 3)
            role = GCF::IGDriveLiteContentSharingModel::Commenter;

        GCF::Result result = sharingModel->modifyShareRole(modelIndex, role);
        if(result.isSuccess())
            return;

        messageBox("Error", QString("An error occured while modifying share permission on '%1' for '%2'<br><br>%3")
                   .arg(contentItem.title()).arg(shareUser).arg(result.message()), false);
    }
}

void GDriveLiteHelper::download(const QString &id, QString path)
{
    if(path.contains("file://")) path.remove("file://");

    GCF::IGDriveLiteFileDownloader *dl = d->gDrive->createFileDownloader(id);
    dl->download(path);
    this->on_cmdJobs_clicked();
}


void GDriveLiteHelper::upload(const QString &id, QString paths)
{
    QStringList fileNames = paths.split(',');
    Q_FOREACH(QString fileName, fileNames)
    {
        if(fileName.contains("file://")) fileName.remove("file://");

        GCF::IGDriveLiteFileUploader *uploader = d->gDrive->createFileUploader(fileName);
        uploader->upload(id);
    }

    this->on_cmdJobs_clicked();
}

void GDriveLiteHelper::share(const QString &id)
{
    delete d->shareWindow;

    QQuickView *qmlView = new QQuickView;
    qmlView->setTitle("Sharing Settings");
    qmlView->setResizeMode(QQuickView::SizeRootObjectToView);

    GCF::IGDriveLiteContentSharingModel *sharingModel = d->gDrive->createSharingModel(id);
    GCF::GDriveContent::Item contentItem = d->gDrive->contentModel()->item(id);

    qmlView->engine()->rootContext()->setContextProperty("sharingModel", sharingModel);
    qmlView->engine()->rootContext()->setContextProperty("shrApp", this);
    qmlView->engine()->rootContext()->setContextProperty("sharingItemTitle", contentItem.title());
    qmlView->setSource(QUrl("qrc:///Qml/ShareWindow.qml"));

    d->shareWindow = qmlView;

    qmlView->show();
    qmlView->raise();
}

void GDriveLiteHelper::describe(const QString &id, const QString &text)
{
    showWaitCursor();
    GCF::Result result = d->gDrive->describe(id, text);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while describing file<br><br>" + result.message()), false);
    restoreCursor();
}

void GDriveLiteHelper::showMoveWindow(const QString &id)
{
    GDriveLiteContentListModel *contentListModel = new GDriveLiteContentListModel(d->gDrive->contentModel());
    contentListModel->setTopmostIndexAsMyDrive();

    GCF::GDriveContent::Item item = d->gDrive->contentModel()->item(id);
    d->curMoveItemId = id;

    delete d->moveWindow;

    QQuickView *qmlView = new QQuickView;
    qmlView->setTitle("Select a target folder to move");
    qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView->engine()->rootContext()->setContextProperty("gDriveContentListModel", contentListModel);
    qmlView->engine()->rootContext()->setContextProperty("gDriveHelper", this);
    qmlView->engine()->rootContext()->setContextProperty("fileName", item.title());
    qmlView->engine()->addImageProvider("thumbnail", new ThumbnailImageProvider(d->gDrive));
    qmlView->setSource(QUrl("qrc:///Qml/MoveOptionWindow.qml"));

    d->moveWindow = qmlView;
    qmlView->show();
}

void GDriveLiteHelper::closeMoveWindow()
{
    if(d->moveWindow != NULL)
        d->moveWindow->close();
}

void GDriveLiteHelper::move(const QString &folderid)
{
    GCF::Result result = d->gDrive->move(d->curMoveItemId, folderid);
    closeMoveWindow();

    if(!result.isSuccess())
        messageBox("Error", QString("Error while moving file<br><br>" + result.message()), false);
}

void GDriveLiteHelper::mkdir(const QString &id)
{
    QmlInputDialog inputDialog(QString("Enter directory name"), true, true);
    inputDialog.setTitle(QString("Create Directory"));
    if(inputDialog.exec() == 1)
    {
        GCF::Result result = d->gDrive->mkdir(inputDialog.inputValue(), id);
        if(!result.isSuccess())
            messageBox("Error", QString("Error while creating directory<br><br>" + result.message()), false);
    }
}

void GDriveLiteHelper::star(const QString &id)
{
    GCF::Result result = d->gDrive->star(id);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while starring file<br><br>" + result.message()), false);
}

void GDriveLiteHelper::unstar(const QString &id)
{
    GCF::Result result = d->gDrive->unstar(id);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while unstarring file<br><br>" + result.message()), false);
}

void GDriveLiteHelper::rename(const QString &id, const QString &name)
{
    showWaitCursor();
    GCF::Result result = d->gDrive->rename(id, name);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while renaming file<br><br>" + result.message()), false);
    restoreCursor();
}

bool GDriveLiteHelper::isInMainDrive(const QString &id)
{
    QModelIndex index = d->gDrive->contentModel()->indexForId(id);
    QModelIndex rootIndex = d->gDrive->contentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive);

    return isIn(index, rootIndex);
}

bool GDriveLiteHelper::isInTrash(const QString &id)
{
    QModelIndex index = d->gDrive->contentModel()->indexForId(id);
    QModelIndex rootIndex = d->gDrive->contentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::Trash);
    return isIn(index, rootIndex);
}

void GDriveLiteHelper::trash(const QString &id)
{
    if(!messageBox("Confirmation", "Do you want, the item to be moved to trash?"))
        return;

    GCF::Result result = d->gDrive->trash(id);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while moving to trash<br><br>" + result.message()), false);
}

void GDriveLiteHelper::restore(const QString &id)
{
    GCF::Result result = d->gDrive->restore(id);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while restoring file<br><br>" + result.message()), false);
}

void GDriveLiteHelper::refresh(const QString &id)
{
    Q_UNUSED(id);
    /*
    // It causes a crash
    QModelIndex index = d->gDrive->contentModel()->indexForId(id);
    GCF::Result result = d->gDrive->contentModel()->refresh(index);
    if(!result.isSuccess())
        messageBox("Error", result.message(), false);
    */
}

void GDriveLiteHelper::deletePermanently(const QString &id)
{
    GCF::Result result = d->gDrive->deletePermanently(id);
    if(!result.isSuccess())
        messageBox("Error", QString("Error while deleting file<br><br>" + result.message()), false);
}

void GDriveLiteHelper::showWaitCursor()
{
    gApp->setOverrideCursor(Qt::WaitCursor);
}

void GDriveLiteHelper::restoreCursor()
{
    gApp->restoreOverrideCursor();
}

void GDriveLiteHelper::on_cmdJobs_clicked()
{
    delete d->jobsWindow;

    QQuickView *qmlView = new QQuickView;
    qmlView->setTitle("Uploads and Downloads");
    qmlView->setResizeMode(QQuickView::SizeRootObjectToView);

    qmlView->engine()->rootContext()->setContextProperty("jobsModel", gAppService->jobs());
    qmlView->engine()->rootContext()->setContextProperty("jobsApp", this);
    qmlView->setSource(QUrl("qrc:///Qml/JobsWindow.qml"));
    d->jobsWindow = qmlView;

    qmlView->show();
    qmlView->raise();
}

bool GDriveLiteHelper::isIn(const QModelIndex &index, const QModelIndex &rootIndex) const
{
    QModelIndex idx = index;
    while(idx.isValid())
    {
        if(idx == rootIndex)
            return true;
        idx = idx.parent();
    }

    return false;
}

bool GDriveLiteHelper::messageBox(const QString &title, const QString &content, bool showCancelButton)
{
    QmlMessageBox messageBox(content, showCancelButton);
    messageBox.setTitle(title);
    return messageBox.exec() == 1;
}
