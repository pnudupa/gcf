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

#include "DriveExplorerWindow.h"
#include "ui_DriveExplorerWindow.h"
#include "ui_AddSharePermissionsDialog.h"
#include "ui_UpdateSharePermissionsDialog.h"
#include "ui_RenameDialog.h"
#include "ui_DescribeDialog.h"
#include "ui_TargetFolderSelectionDialog.h"
#include "DriveFolderModel.h"

#include <QClipboard>
#include <QNetworkRequest>
#include <QPointer>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QPushButton>
#include <QInputDialog>

#include <GCF3/IGDriveLite>
#include <GCF3/IGDriveLiteContentModel>

#if QT_VERSION >= 0x050000
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QStandardPaths>
#else
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#endif

struct DriveExplorerWindowData
{
    DriveExplorerWindowData() : titleShowsUserInfo(false) { }

    GCF::IGDriveLite *gDrive;
    Ui::DriveExplorerWindow ui;
    bool titleShowsUserInfo;

#if QT_VERSION >= 0x050000
    QPointer<QQuickView> jobsWindow;
#else
    QPointer<QDeclarativeView> jobsWindow;
#endif

    void createValueItem(QTreeWidget *parent, QTreeWidgetItem *parentItem,
                         const QString &name, const QVariant &value) {
        if(name.startsWith('_'))
            return;

        QTreeWidgetItem *item = 0;
        if(parentItem)
            item = new QTreeWidgetItem(parentItem);
        else
            item = new QTreeWidgetItem(parent);

        item->setText(0, name);

        QFont italicFont = item->font(1);
        italicFont.setItalic(true);

        if(value.type() == QVariant::List) {
            QVariantList list = value.toList();
            item->setText(1, QString("Array of %1 values").arg(list.count()));
            item->setFont(1, italicFont);
            for(int i=0; i<list.count(); i++)
                this->createValueItem(0, item, QString("[%1]").arg(i), list.at(i));
        } else if(value.type() == QVariant::Map) {
            QVariantMap map = value.toMap();
            item->setText(1, "Object");
            item->setFont(1, italicFont);
            QVariantMap::const_iterator it = map.constBegin();
            QVariantMap::const_iterator end = map.constEnd();
            while(it != end) {
                this->createValueItem(0, item, it.key(), it.value());
                ++it;
            }
        } else
            item->setText(1, value.toString());
    }

    bool isInMyDrive(const QModelIndex &index) const {
        QModelIndex rootIndex = this->gDrive->contentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive);
        return this->isIn(index, rootIndex);
    }

    bool isInTrash(const QModelIndex &index) const {
        QModelIndex rootIndex = this->gDrive->contentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::Trash);
        return this->isIn(index, rootIndex);
    }

    bool isIn(const QModelIndex &index, const QModelIndex &rootIndex) const {
        QModelIndex idx = index;
        while(idx.isValid()) {
            if(idx == rootIndex)
                return true;
            idx = idx.parent();
        }
        return false;
    }
};

DriveExplorerWindow::DriveExplorerWindow(GCF::IGDriveLite *gDrive, QWidget *parent) :
    QWidget(parent)
{
    d = new DriveExplorerWindowData;
    d->gDrive = gDrive;
    d->ui.setupUi(this);
    d->ui.driveContentTree->viewport()->installEventFilter(this);
    d->ui.gridView->viewport()->installEventFilter(this);

    GCF::IGDriveLiteContentModel *contentModel = d->gDrive->contentModel();
    contentModel->setContentVisibility(GCF::IGDriveLiteContentModel::ShowAll);
    contentModel->setColumns(QList<GCF::IGDriveLiteContentModel::Field>() <<
                             GCF::IGDriveLiteContentModel::Title);
    d->ui.driveContentTree->setModel(contentModel);
    d->ui.gridView->setModel(contentModel);

    connect(contentModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(modelDataChanged(QModelIndex)));

    this->clear();

#if 0
    connect(d->gDrive, SIGNAL(authenticationChanged()),
            this, SLOT(onGDriveLiteAuthenticationChanged()));
#else
    d->ui.cmdRefreshAuthToken->hide();
#endif
}

DriveExplorerWindow::~DriveExplorerWindow()
{
    delete d;
}

void DriveExplorerWindow::showFileLocation(const QString &file)
{
    QFileInfo fi(file);
    if(!fi.exists())
    {
        QMessageBox::information(this, "File not found",
                                 QString("The file '%1' doesnt exist at the said location anymore")
                                 .arg(fi.fileName()));
        return;
    }

    QString location = fi.absolutePath();
    QDesktopServices::openUrl( QUrl::fromLocalFile(location) );
}

GCF::IGDriveLite *DriveExplorerWindow::gDrive() const
{
    return d->gDrive;
}

void DriveExplorerWindow::on_cmdLogout_clicked()
{
    qApp->quit();
}

void DriveExplorerWindow::on_cmdRefresh_clicked()
{
    d->gDrive->contentModel()->refresh();
}

void DriveExplorerWindow::on_cmdJobs_clicked()
{
    delete d->jobsWindow;

#if QT_VERSION >= 0x050000
    QQuickView *qmlView = new QQuickView;
    qmlView->setTitle("Uploads and Downloads");
    qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
#else
    QDeclarativeView *qmlView = new QDeclarativeView;
    qmlView->setWindowTitle("Downloads");
    qmlView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
#endif

    qmlView->engine()->rootContext()->setContextProperty("jobsModel", gAppService->jobs());
    qmlView->engine()->rootContext()->setContextProperty("jobsApp", this);
#if QT_VERSION >= 0x050000
    qmlView->setSource(QUrl("qrc:///JobsWindowQt5.qml"));
#else
    qmlView->setSource(QUrl("qrc:///JobsWindowQt4.qml"));
#endif
    d->jobsWindow = qmlView;

    qmlView->show();
    qmlView->raise();
}

void DriveExplorerWindow::on_driveContentTree_clicked(const QModelIndex &index)
{
    if(index.data(GCF::IGDriveLiteContentModel::IsFolder).toBool())
        d->ui.gridView->setRootIndex(index);
    else
    {
        d->ui.gridView->setRootIndex(index.parent());
        d->ui.gridView->setCurrentIndex(index);
    }

    this->showProperties(index);
}

void DriveExplorerWindow::on_gridView_clicked(const QModelIndex &index)
{
    d->ui.driveContentTree->setCurrentIndex(index);
    this->showProperties(index);
}

void DriveExplorerWindow::on_contentProperties_itemClicked(QTreeWidgetItem *item, int column)
{
    qApp->clipboard()->setText(QString());
    if(column == 0 && item)
    {
        QString text = QString("%1=%2").arg(item->text(0)).arg(item->text(1));
        qApp->clipboard()->setText(text);
    }
}

void DriveExplorerWindow::modelDataChanged(const QModelIndex &index)
{
    if(d->ui.driveContentTree->currentIndex() == index)
        this->on_driveContentTree_clicked(index);
}

void DriveExplorerWindow::on_cmdRefreshAuthToken_clicked()
{
    d->gDrive->authenticator()->refreshAuthentication();
}

void DriveExplorerWindow::onGDriveLiteAuthenticationChanged()
{
    qDebug() << d->gDrive->accessToken();
}

void DriveExplorerWindow::showProperties(const QModelIndex &index)
{
    this->clear();

    GCF::GDriveContent::Item item = d->gDrive->contentModel()->item(index);
    if(!item.isValid() || item.id().isEmpty())
        return;

    QNetworkRequest request1( QUrl(item.thumbnailLink()) );
    QString authString = QString("Bearer %1").arg(d->gDrive->accessToken());
    request1.setRawHeader("Authorization", authString.toLatin1());
    d->ui.preview->load(request1);

    if( item.owners().count() )
    {
        GCF::GDriveContent::User owner = item.owners().first();

        QNetworkRequest request2( QUrl(owner.picture()) );
        QString authString = QString("Bearer %1").arg(d->gDrive->accessToken());
        request2.setRawHeader("Authorization", authString.toLatin1());
        d->ui.owner->load(request2);
    }

    d->ui.titleLabel->setText( item.title() );
    QVariantMap map = item.data();
    QVariantMap::const_iterator it = map.constBegin();
    QVariantMap::const_iterator end = map.constEnd();
    while(it != end)
    {
        d->createValueItem(d->ui.contentProperties, 0, it.key(), it.value());
        ++it;
    }

    d->ui.contentProperties->resizeColumnToContents(0);
}

bool DriveExplorerWindow::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::ContextMenu)
    {
        QContextMenuEvent *me = (QContextMenuEvent*)e;
        QModelIndex index;

        if(obj == d->ui.driveContentTree->viewport())
        {
            index = d->ui.driveContentTree->indexAt(me->pos());
            d->ui.driveContentTree->setCurrentIndex(index);
            this->on_driveContentTree_clicked(index);
        }
        else if(obj == d->ui.gridView->viewport())
            index = d->ui.gridView->indexAt(me->pos());

        d->ui.gridView->setCurrentIndex(index);
        this->handleContextMenu(index);
    }

    return false;
}

void DriveExplorerWindow::handleContextMenu(const QModelIndex &index)
{
    if(!index.isValid() || !(index.flags()&Qt::ItemIsEnabled))
        return;

    QString itemId = index.data(GCF::IGDriveLiteContentModel::Id).toString();
    QMenu menu;
    QAction *download = 0, *upload = 0, *refresh = 0, *share = 0;
    QAction *trash = 0, *trashForEver = 0, *restore = 0;
    QAction *rename = 0, *describe = 0, *star = 0, *unstar = 0, *move = 0;
    QAction *mkdir = 0;

    // Download + Upload
    if(index.data(GCF::IGDriveLiteContentModel::IsFile).toBool())
        download = menu.addAction("Download");
    else if(index.data(GCF::IGDriveLiteContentModel::IsFolder).toBool() && d->isInMyDrive(index))
        upload = menu.addAction("Upload");
    menu.addSeparator();

    // Meta-data-update actions
    if(d->isInMyDrive(index) && index != d->gDrive->contentModel()->specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive))
    {
        rename = menu.addAction("Rename");
        describe = menu.addAction("Describe");
        if(index.data(GCF::IGDriveLiteContentModel::IsStarred).toBool())
            unstar = menu.addAction("Unstar");
        else
            star = menu.addAction("Star");
        move = menu.addAction("Move");
    }
    if(d->isInMyDrive(index) && index.data(GCF::IGDriveLiteContentModel::IsFolder).toBool())
        mkdir = menu.addAction("Create directory");
    menu.addSeparator();

    // Sharing
    share = menu.addAction("Share settings");
    menu.addSeparator();

    // Delete, undelete
    if(d->isInTrash(index))
    {
        trashForEver = menu.addAction("Delete");
        restore = menu.addAction("Restore");
    }
    else
        trash = menu.addAction("Trash");
    menu.addSeparator();

    // Refresh
    refresh = menu.addAction("Refresh");

    QAction *result = menu.exec(QCursor::pos());
    if(download && result == download)
        this->download(itemId);
    else if(upload && result == upload)
        this->upload(itemId);
    else if(result == share)
        this->share(itemId);
    else if(trash && result == trash)
        this->trash(itemId);
    else if(trashForEver && result == trashForEver)
        this->deletePermanently(itemId);
    else if(restore && result == restore)
        this->restore(itemId);
    else if(rename && rename == result)
        this->rename(itemId);
    else if(describe && describe == result)
        this->describe(itemId);
    else if(star && star == result)
        this->star(itemId);
    else if(unstar && unstar == result)
        this->unstar(itemId);
    else if(move && move == result)
        this->move(itemId);
    else if(mkdir && mkdir == result)
        this->mkdir(itemId);
    else if(result == refresh)
    {
        GCF::Result result = d->gDrive->contentModel()->refresh(index);
        if(!result.isSuccess())
            QMessageBox::information(this, "Error", result.message());
    }
}

void DriveExplorerWindow::clear()
{
    if(d->titleShowsUserInfo == false)
    {
        if( d->gDrive->authenticatedUser().isValid() )
        {
            QString title = QString("Google Drive Explorer - %1")
                    .arg(d->gDrive->authenticatedUser().name());
            this->setWindowTitle(title);
            d->titleShowsUserInfo = true;
        }
    }

    d->ui.titleLabel->clear();
    d->ui.preview->setHtml(QString());
    d->ui.owner->setHtml(QString());
    d->ui.contentProperties->clear();
}

void DriveExplorerWindow::download(const QString &id)
{
#if QT_VERSION >= 0x050000
    QString downloadsFolder = QStandardPaths::writableLocation( QStandardPaths::DownloadLocation );
#else
    QString downloadsFolder = QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation );
#endif

    QString path = QFileDialog::getExistingDirectory(this, "Select a folder to download into", downloadsFolder);
    qDebug() << "DriveExplorerWindow " << path;
    GCF::IGDriveLiteFileDownloader *dl = d->gDrive->createFileDownloader(id);
    dl->download(path);
    this->on_cmdJobs_clicked();
}

class ModalEventLoop : public QEventLoop
{
    QObject *m_windowObject;

public:
    ModalEventLoop(QObject *windowObj) : m_windowObject(windowObj) {
        m_windowObject->installEventFilter(this);
    }
    ~ModalEventLoop() { }

    bool eventFilter(QObject *obj, QEvent *e) {
        if(obj == m_windowObject && e->type() == QEvent::Close)
            this->quit();
        return false;
    }
};

void DriveExplorerWindow::share(const QString &id)
{
#if QT_VERSION >= 0x050000
    QQuickView qmlView;
    qmlView.setTitle("Sharing Settings");
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
#else
    QDeclarativeView qmlView;
    qmlView.setWindowTitle("Sharing Settings");
    qmlView.setResizeMode(QDeclarativeView::SizeRootObjectToView);
#endif

    GCF::IGDriveLiteContentSharingModel *sharingModel = d->gDrive->createSharingModel(id);
    GCF::GDriveContent::Item contentItem = d->gDrive->contentModel()->item(id);

    qmlView.engine()->rootContext()->setContextProperty("sharingModel", sharingModel);
    qmlView.engine()->rootContext()->setContextProperty("shrApp", this);
    qmlView.engine()->rootContext()->setContextProperty("sharingItemTitle", contentItem.title());
#if QT_VERSION >= 0x050000
    qmlView.setSource(QUrl("qrc:///ShareWindowQt5.qml"));
    qmlView.setModality(Qt::ApplicationModal);
#else
    qmlView.setSource(QUrl("qrc:///ShareWindowQt4.qml"));
    qmlView.setWindowModality(Qt::ApplicationModal);
#endif

    qmlView.show();

    ModalEventLoop eventLoop(&qmlView);
    eventLoop.exec();

    delete sharingModel;
}

void DriveExplorerWindow::upload(const QString &id)
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select file(s) to upload", QDir::homePath());
    if(fileNames.isEmpty())
        return;

    Q_FOREACH(QString fileName, fileNames)
    {
        GCF::IGDriveLiteFileUploader *uploader = d->gDrive->createFileUploader(fileName);
        uploader->upload(id);
    }

    this->on_cmdJobs_clicked();
}

void DriveExplorerWindow::trash(const QString &id)
{
    GCF::Result result = d->gDrive->trash(id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while moving to trash<br><br>" + result.message());
}

void DriveExplorerWindow::deletePermanently(const QString &id)
{
    GCF::Result result = d->gDrive->deletePermanently(id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while deleting file<br><br>" + result.message());
}

void DriveExplorerWindow::restore(const QString &id)
{
    GCF::Result result = d->gDrive->restore(id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while restoring file<br><br>" + result.message());
}

void DriveExplorerWindow::rename(const QString &id)
{
    QDialog dialog(this);
    Ui::RenameDialog ui;
    ui.setupUi(&dialog);

    GCF::GDriveContent::Item item = d->gDrive->contentModel()->item(id);
    ui.txtOldName->setText(item.title());
    ui.txtNewName->setText(item.title());

    if( dialog.exec() == QDialog::Rejected )
        return;

    if(ui.txtNewName->text() == ui.txtOldName->text())
        return;

    GCF::Result result = d->gDrive->rename(id, ui.txtNewName->text());
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while renaming file<br><br>" + result.message());
}

void DriveExplorerWindow::describe(const QString &id)
{
    QDialog dialog(this);
    Ui::DescribeDialog ui;
    ui.setupUi(&dialog);

    GCF::GDriveContent::Item item = d->gDrive->contentModel()->item(id);
    ui.lblHeader->setText( QString("Descripton for '%1'").arg(item.title()) );
    ui.txtDescription->setPlainText( item.description() );

    if( dialog.exec() == QDialog::Rejected )
        return;

    GCF::Result result = d->gDrive->describe(id, ui.txtDescription->toPlainText());
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while describing file<br><br>" + result.message());
}

void DriveExplorerWindow::star(const QString &id)
{
    GCF::Result result = d->gDrive->star(id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while starring file<br><br>" + result.message());
}

void DriveExplorerWindow::unstar(const QString &id)
{
    GCF::Result result = d->gDrive->unstar(id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while unstarring file<br><br>" + result.message());
}

void DriveExplorerWindow::move(const QString &id)
{
    DriveFolderModel model(d->gDrive);
    QModelIndex myDriveIndex = model.specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive);
    GCF::GDriveContent::Item item = d->gDrive->contentModel()->item(id);

    QDialog dialog(this);
    Ui::TargetFolderSelectionDialog ui;
    ui.setupUi(&dialog);
    ui.lblNote->setText( ui.lblNote->text().arg(item.title()) );
    ui.treeView->setModel(&model);
    ui.treeView->setRootIndex(myDriveIndex);
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText("Select");

    if( dialog.exec() == QDialog::Rejected )
        return;

    QString folderId = model.selectedFolderId();
    GCF::Result result = d->gDrive->move(id, folderId);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while moving file<br><br>" + result.message());
}

void DriveExplorerWindow::mkdir(const QString &id)
{
    QString name = QInputDialog::getText(this, "Create Directory", "Enter directory name");
    if(name.isEmpty())
        return;

    GCF::Result result = d->gDrive->mkdir(name, id);
    if(!result.isSuccess())
        QMessageBox::information(this, "Error", "Error while creating directory<br><br>" + result.message());
}

void DriveExplorerWindow::addSharePermission(QObject *object)
{
    // FIXME: Using dynamic_cast because qobject_cast cannot be used here
    GCF::IGDriveLiteContentSharingModel *sharingModel
            = dynamic_cast<GCF::IGDriveLiteContentSharingModel*>(object);
    if(!sharingModel)
        return;

    GCF::GDriveContent::Item contentItem = sharingModel->contentItem();

    QDialog dialog(this);
    Ui::AddSharePermissionsDialog ui;
    ui.setupUi(&dialog);
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText("Share");
    dialog.setWindowTitle( QString("Share file: %1").arg(contentItem.title()) );
    if( dialog.exec() != QDialog::Accepted )
        return;

    GCF::GDriveLiteShareRequest request;
    request.EMailAddress = ui.txtEMailAddress->text();
    request.SendNotificationEMails = ui.chkNotifyByEMail->isChecked();
    request.EMailMessage = ui.txtEMailMessage->toPlainText();
    if(ui.rbCanRead->isChecked())
        request.Role = GCF::IGDriveLiteContentSharingModel::Reader;
    else if(ui.rbCanWrite->isChecked())
        request.Role = GCF::IGDriveLiteContentSharingModel::Writer;
    else if(ui.rbCanComment->isChecked())
        request.Role = GCF::IGDriveLiteContentSharingModel::Writer|
                GCF::IGDriveLiteContentSharingModel::Commenter;

    qApp->setOverrideCursor(Qt::WaitCursor);
    GCF::Result result = sharingModel->share(request);
    qApp->restoreOverrideCursor();
    if(result.isSuccess())
        return;

    QMessageBox::information(this, "Error",
                             QString("An error occured while sharing file '%1' with '%2'<br><br>%3")
                             .arg(contentItem.title()).arg(request.EMailAddress)
                             .arg(result.message()));
}

void DriveExplorerWindow::removeSharePermission(QObject *object, int index)
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

    QMessageBox::StandardButton answer =
            QMessageBox::question(this, "Unshare confirmation",
                                  QString("Are you sure you want to stop sharing the file '%1' with '%2'?")
                                  .arg(contentItem.title()).arg(shareUser),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(answer == QMessageBox::Yes)
    {
        qApp->setOverrideCursor(Qt::WaitCursor);
        GCF::Result result = sharingModel->unshare(modelIndex);
        qApp->restoreOverrideCursor();
        if(result.isSuccess())
            return;

        QMessageBox::information(this, "Error",
                                 QString("An error occured while revoking share permission on '%1' for '%2'<br><br>%3")
                                 .arg(contentItem.title()).arg(shareUser).arg(result.message()));
    }
}

void DriveExplorerWindow::updateSharePermisssion(QObject *object, int index)
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

    QDialog dialog(this);
    Ui::UpdateSharePermissionsDialog ui;
    ui.setupUi(&dialog);
    ui.lblTitle->setText( QString("Update share permissions on file '%1' for user '%2'")
                          .arg(contentItem.title()).arg(shareUser) );
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText("Apply");
    dialog.setWindowTitle( QString("Update share permissions") );
    if( dialog.exec() != QDialog::Accepted )
        return;

    int role = GCF::IGDriveLiteContentSharingModel::Unknown;
    if(ui.rbCanRead->isChecked())
        role = GCF::IGDriveLiteContentSharingModel::Reader;
    else if(ui.rbCanWrite->isChecked())
        role = GCF::IGDriveLiteContentSharingModel::Writer;
    else if(ui.rbCanComment->isChecked())
        role = GCF::IGDriveLiteContentSharingModel::Commenter;

    qApp->setOverrideCursor(Qt::WaitCursor);
    GCF::Result result = sharingModel->modifyShareRole(modelIndex, role);
    qApp->restoreOverrideCursor();

    if(result.isSuccess())
        return;

    QMessageBox::information(this, "Error",
                             QString("An error occured while modifying share permission on '%1' for '%2'<br><br>%3")
                             .arg(contentItem.title()).arg(shareUser).arg(result.message()));
}

