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

#include <QString>
#include <QtTest>
#include <QSettings>
#include <QWebView>
#include <QWebFrame>
#include <QMetaType>
#include <QWebElement>
#include <QStringList>

#include <GCF3/GuiApplication>
#include <GCF3/Component>
#include <GCF3/Log>
#include <GCF3/Version>
#include <GCF3/IGDriveLite>
#include <GCF3/SignalSpy>

#include "TestSettings.h"
#include "GDriveRequestAPI.h"

class GDriveLiteTest : public QObject
{
    Q_OBJECT

public:
    GDriveLiteTest();

    bool isValid() const { return m_settings.count() >= 4; }

public slots:
    void auth(GCF::IGDriveLiteAuthenticator *) {
        m_authCalled = true;
    }

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testComponentLoad();
    void testAuthorization();
    void testContentModel();
    void testRefresh();
    void testRefreshFolder();
    void testRefreshFile();
    void testDefaultColumns();
    void testRoleNames();
    void testSetColumns();
    void testItem();
    void testDownloader();
    void testDownloaderErrors();
    void testDownloaderModel();
    void testUploaderAndDeletePermanently();
    void testUploaderModel();
    void testSharing();
    void testTrashAndRestore();
    void testRename();
    void testDescribe();
    void testMove();
    void testStarUnstar();
    void testMkdir();

private:
    QDir dataDirectory() { return m_dataDirectory; }

    void verifyContentModel();
    bool ensureContentModelIsFullyLoaded();
    bool ensureFolderIsFullyLoaded(const QModelIndex &folderIndex);
    bool verifyModelIndexFlags(const QModelIndex &index);
    QStringList dumpModel();
    QStringList dumpModelIndex(const QString &path, const QModelIndex &index);
    QStringList expectedContents();
    QString logFileContents(bool deleteFile=true) const;
    void printDownloadersStatus();
    void printUploadersStatus();

    void fetchDriveContents();
    void populateGDriveContentInformation(GCF::ObjectTreeNode *node, const QString &path=QString());
    void sortFileItemList();
    void sortFolderItemList();

    QList<GCF::ObjectTreeNode*> contentNodes(const QString &id, GCF::ObjectTree *tree) {
        if(id.isEmpty())
            return QList<GCF::ObjectTreeNode*>();
        QSet<GCF::ObjectTreeNode*> nodes = this->contentNodes( tree->rootNode(), id );
        return nodes.toList();
    }

    QSet<GCF::ObjectTreeNode*> contentNodes(GCF::ObjectTreeNode *node, const QString &id) {
        QSet<GCF::ObjectTreeNode*> nodes;
        if(id.isEmpty())
            return nodes;
        if(node->name() == id)
            nodes += node;
        Q_FOREACH(GCF::ObjectTreeNode *child, node->children()) {
            QSet<GCF::ObjectTreeNode*> childNodes = this->contentNodes(child, id);
            nodes += childNodes;
        }
        return nodes;
    }

private:
    QVariantMap m_settings;

    GCF::IGDriveLite *m_gDriveLite;
    bool m_authCalled;

    QVariantMap m_gDriveContentTitleIdMap;
    QList< GCF::GDriveContent::Item > m_folderItemList;
    QList< GCF::GDriveContent::Item > m_fileItemList;

    QDir m_dataDirectory;
};

#if QT_VERSION <= 0x050000
Q_DECLARE_METATYPE(QModelIndex)
#endif

GDriveLiteTest::GDriveLiteTest()
    : m_gDriveLite(0), m_authCalled(false)
{
#if QT_VERSION <= 0x050000
    qRegisterMetaType<QModelIndex>("QModelIndex");
#endif

    m_dataDirectory = qApp->applicationDirPath();
    m_dataDirectory.cdUp();
    m_dataDirectory.cd("TestData");
    m_dataDirectory.cd("GDriveLiteTest");

    GDriveTests::loadSettings(m_settings);
    const QStringList requiredSettings = QStringList() << "client_id"
                         << "client_secret" << "login" << "password";
    bool validEnvironment = true;
    Q_FOREACH(QString settings, requiredSettings)
    {
        if(m_settings.contains(settings))
            continue;

        validEnvironment = false;

        QByteArray var = settings.toUpper().toLatin1();
        qDebug("Please provide the GDRIVE_TEST_%s environment variable", var.constData());
    }

    if(!validEnvironment)
        m_settings.clear();
}

void GDriveLiteTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void GDriveLiteTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void GDriveLiteTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void GDriveLiteTest::testComponentLoad()
{
    QVERIFY(m_settings.count());

    GCF::Component *gDriveComp = gApp->loadComponent("GDrive/GDriveLite");
    QVERIFY2(gDriveComp != 0, "Could not load GDrive/GDriveLite component");

    GCF::IGDriveLite *gDriveLite = qobject_cast<GCF::IGDriveLite*>(gDriveComp);
    QVERIFY2(gDriveLite != 0, "GDrive/GDriveLite componenet doesnt implement GCF::IGDriveLite interface");

    QVERIFY(gDriveLite->isAuthenticated() == false);
    QVERIFY(gDriveLite->authenticatedUser().isValid() == false);
    QVERIFY(gDriveLite->accessToken().isEmpty());
    QVERIFY(gDriveLite->refreshToken().isEmpty());
    QVERIFY(gDriveLite->errorMessage().isEmpty());
    QVERIFY(gDriveLite->contentModel() == 0);

    gDriveComp->unload();
}

void GDriveLiteTest::testAuthorization()
{
    QVERIFY(m_settings.count());

    QList< QPair<QByteArray,QVariant> > props;
    props << qMakePair<QByteArray,QVariant>("clientID", m_settings.value("client_id").toString());
    props << qMakePair<QByteArray,QVariant>("clientSecret", m_settings.value("client_secret").toString());

    GCF::Component *gDriveComp = gApp->loadComponent("GDrive/GDriveLite", props);
    QVERIFY2(gDriveComp != 0, "Could not load GDrive/GDriveLite component");
    QVERIFY(gDriveComp->property("clientID").toString() == m_settings.value("client_id").toString());
    QVERIFY(gDriveComp->property("clientSecret").toString() == m_settings.value("client_secret").toString());

    m_gDriveLite = qobject_cast<GCF::IGDriveLite*>(gDriveComp);
    QVERIFY2(m_gDriveLite != 0, "GDrive/GDriveLite componenet doesnt implement GCF::IGDriveLite interface");
    QVERIFY(m_gDriveLite->contentModel() != 0);

    // Perform authorization

    // Load the authentication page in a web-view
    qDebug() << "Showing authentication URL in a web-view";
    QUrl url = m_gDriveLite->authenticator()->authenticationPageUrl();
    QWebView webView;
    webView.load(url);
    webView.show();
#if QT_VERSION >= 0x050000
    QTest::qWaitForWindowActive(&webView);
#else
    QTest::qWaitForWindowShown(&webView);
#endif

    GCF::SignalSpy spy(&webView, SIGNAL(loadFinished(bool)));
    spy.wait(10000);

    // Enter user name and password
    QWebFrame *frame = webView.page()->mainFrame();

    QWebElement email = frame->findFirstElement("#Email");
    QTest::mouseClick(&webView, Qt::LeftButton, Qt::NoModifier, email.geometry().center());
    QTest::keyClicks(&webView, m_settings.value("login").toString());
    qDebug() << "Login entered";

    QWebElement passwd = frame->findFirstElement("#Passwd");
    QTest::mouseClick(&webView, Qt::LeftButton, Qt::NoModifier, passwd.geometry().center());
    QTest::keyClicks(&webView, m_settings.value("password").toString());
    qDebug() << "Password entered";

    // Submit user-name password
    QWebElement submit = frame->findFirstElement("#signIn");
    QTest::mouseClick(&webView, Qt::LeftButton, Qt::NoModifier, submit.geometry().center());
    qDebug() << "Sign-in button clicked entered";
    spy.wait(10000);
    qDebug() << "Sign-in succeeded. We have moved to the approval page. Waiting for approve button to be enabled";
    QTest::qWait(5000); // for the button to get enabled

    // Approve access
    GCF::SignalSpy titleSpy(&webView, SIGNAL(titleChanged(QString)));
    QWebElement approve = frame->findFirstElement("#submit_approve_access");
    QTest::mouseClick(&webView, Qt::LeftButton, Qt::NoModifier, approve.geometry().center());
    qDebug() << "Clicked on Approve. Waiting for title to change";
    titleSpy.wait(10000);

    // Get new title from webview
    QString title = webView.title();
    webView.close();
    qDebug() << "Title changed. Authenticating from the title";

    // Authorize GDrive using the title
    GCF::SignalSpy authSpy(m_gDriveLite, SIGNAL(authenticationChanged()));
    GCF::SignalSpy authUserSpy(m_gDriveLite, SIGNAL(authenticatedUserChanged()));
    m_authCalled = false;
    GCF::Result result = m_gDriveLite->authenticator()->authenticateFromPageTitle(title,
                                                                                  this, SLOT(auth(GCF::IGDriveLiteAuthenticator*)));
    if(!result.isSuccess())
        qDebug() << result;
    QVERIFY(result.isSuccess() == true);

    qDebug() << "Authentication successfull";

    authSpy.wait(10000);
    QVERIFY(m_authCalled == true);
    QVERIFY(authSpy.count() == 1);
    QVERIFY(m_gDriveLite->isAuthenticated() == true);
    QVERIFY(!m_gDriveLite->accessToken().isEmpty());

    authUserSpy.wait(10000);
    QVERIFY(authUserSpy.count() == 1);
    QVERIFY(m_gDriveLite->authenticatedUser().isValid() == true);

    QTest::qWait(500); // for other parts of GDriveLite to initialize based on authorization

    // Fetch contents of the drive and make a list of files in there.
    this->fetchDriveContents();
}

void GDriveLiteTest::testContentModel()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    this->verifyContentModel();
}

void GDriveLiteTest::testRefresh()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    GCF::IGDriveLiteContentModel *model = m_gDriveLite->contentModel();
    QVERIFY(model->rowCount() != 0);
    QVERIFY(model->refresh().isSuccess() == true);

    // Attempting to refresh for the second time, shouldnt work!
    GCF::Result result = model->refresh();
    QVERIFY(result.isSuccess() == false);

    // Now verify if the refresh worked properly or not.
    this->verifyContentModel();
}

void GDriveLiteTest::testRefreshFolder()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY2(m_folderItemList.count() >= 2, "Insufficient folders in the drive for testing refreshFolder");

    GCF::GDriveContent::Item largeFolderItem = m_folderItemList.last();
    GCF::GDriveContent::Item smallFolderItem = m_folderItemList.first();
    int mostChildren = largeFolderItem.data().value("_childCount").toInt();
    int leastChildren = smallFolderItem.data().value("_childCount").toInt();

    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    QModelIndex largeFolderIndex = contentModel->indexForId(largeFolderItem.id());
    QModelIndex smallFolderIndex = contentModel->indexForId(smallFolderItem.id());

    // Verify whether the row-counts match in the content model
    QVERIFY(mostChildren == contentModel->rowCount(largeFolderIndex));
    QVERIFY(leastChildren == contentModel->rowCount(smallFolderIndex));

    // Now perform refresh
    GCF::SignalSpy folderLoadedSpy(contentModel, SIGNAL(folderItemLoaded(QModelIndex)));
    qDebug() << "Refreshing " << largeFolderItem.title();
    GCF::Result result = contentModel->refresh(largeFolderIndex);
    QVERIFY(result.isSuccess() == true);

    // While refreshing the large-folder, another folder refresh should be rejected
    result = contentModel->refresh(smallFolderIndex);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == QString("Refresh of '%1' is underway. Please try again after sometime.")
            .arg(largeFolderItem.title()));

    // Wait for the large folder to get updated
    folderLoadedSpy.wait(120000);
    QVERIFY(folderLoadedSpy.count() == 1);
    QVERIFY(folderLoadedSpy.first().first().value<QModelIndex>() == largeFolderIndex);
    QVERIFY(contentModel->rowCount(largeFolderIndex) == mostChildren);
    folderLoadedSpy.clear();

    // At this point, we will be having files within large folder being loaded
    qDebug() << " - Ensuring that all" << mostChildren << "items in the folder are loaded";
    GCF::SignalSpy fileLoadedSpy(contentModel, SIGNAL(fileItemLoaded(QModelIndex)));
    for(int i=0; i<mostChildren; i++)
    {
        QModelIndex index = contentModel->index(i, 0, largeFolderIndex);
        while(!contentModel->item(index).isValid())
            fileLoadedSpy.wait();
        qDebug() << "    [" << i << "] = " << index.data(GCF::IGDriveLiteContentModel::Title).toString();
        fileLoadedSpy.clear();
    }

    // Now all refreshes will be truly done. But we got to still wait
    // for a bit longer to truly ensure all refresh-objects are destroued
    QTest::qWait(100);

    // Now it should be possible to refresh the small folder
    qDebug() << "Refreshing " << smallFolderItem.title();
    result = contentModel->refresh(smallFolderIndex);
    QVERIFY(result.isSuccess() == true);

    // Wait for the small folder to get refreshed
    folderLoadedSpy.wait(120000);
    QVERIFY(folderLoadedSpy.count() == 1);
    QVERIFY(folderLoadedSpy.first().first().value<QModelIndex>() == smallFolderIndex);
    QVERIFY(contentModel->rowCount(smallFolderIndex) == leastChildren);
    folderLoadedSpy.clear();

    // At this point, we will be having files within small folder being loaded
    qDebug() << " - Ensuring that all" << leastChildren << "items in the folder are loaded";
    for(int i=0; i<leastChildren; i++)
    {
        QModelIndex index = contentModel->index(i, 0, smallFolderIndex);
        while(!contentModel->item(index).isValid())
            fileLoadedSpy.wait();
        qDebug() << "    [" << i << "] = " << index.data(GCF::IGDriveLiteContentModel::Title).toString();
        fileLoadedSpy.clear();
    }

    // Now all refreshes will be truly done. But we got to still wait
    // for a bit longer to truly ensure all refresh-objects are destroued
    QTest::qWait(100);
}

void GDriveLiteTest::testRefreshFile()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY2(m_fileItemList.count() >= 2, "Insufficient files in the drive for testing refreshFile");

    // Pick 2 file-ids from the middle
    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    GCF::GDriveContent::Item fileItem1 = m_fileItemList.at( m_fileItemList.count()/2 );
    GCF::GDriveContent::Item fileItem2 = m_fileItemList.at( m_fileItemList.count()/2-1 );
    QModelIndex fileIndex1 = contentModel->indexForId(fileItem1.id());
    QModelIndex fileIndex2 = contentModel->indexForId(fileItem2.id());

    // Start refreshing one fileIndex1
    GCF::SignalSpy spy(contentModel, SIGNAL(fileItemLoaded(QModelIndex)));
    qDebug() << "Refreshing file" << fileIndex1.data(GCF::IGDriveLiteContentModel::Title).toString();
    GCF::Result result = contentModel->refresh(fileIndex1);
    QVERIFY(result.isSuccess() == true);
    QVERIFY(contentModel->flags(fileIndex1) == 0);

    // Before refresh of fileIndex1, we shouldnt be able to refresh fileIndex2
    result = contentModel->refresh(fileIndex2);
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == QString("Refresh of '%1' is underway. Please try again after sometime.")
            .arg(fileItem1.title()));

    // Wait for refresh of first item to complete
    QVERIFY(spy.wait());
    QVERIFY(spy.first().first().value<QModelIndex>() == fileIndex1);
    Qt::ItemFlags flags = contentModel->flags(fileIndex1);
    QVERIFY(flags&Qt::ItemIsEnabled && flags&Qt::ItemIsSelectable);
    QVERIFY(fileIndex1.data(GCF::IGDriveLiteContentModel::Id).toString() == fileItem1.id());
    QVERIFY(fileIndex1.data(GCF::IGDriveLiteContentModel::Title).toString() == fileItem1.title());

    // Wait for some more time to ensure that the refresh is really complete
    QTest::qWait(100);
}

void GDriveLiteTest::testDefaultColumns()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    /*
     * NOTE to Developer
     *
     * By default some columns are shown by the content model.
     * This function verifies the default columns. If at any point
     * the default columns change, then we should update this test
     * case and also the corresponding documentation of the GDriveLite
     * component.
     */
    QList<GCF::IGDriveLiteContentModel::Field> defaultColumns;
    defaultColumns << GCF::IGDriveLiteContentModel::Title;
    defaultColumns << GCF::IGDriveLiteContentModel::OwnerNames;
    defaultColumns << GCF::IGDriveLiteContentModel::MimeType;
    defaultColumns << GCF::IGDriveLiteContentModel::ModifiedDate;

    QList<GCF::IGDriveLiteContentModel::Field> columns;
    columns = m_gDriveLite->contentModel()->columns();

    QVERIFY2(defaultColumns == columns, "Default columns has changed. Update this test case and documentation.");
}

void GDriveLiteTest::testRoleNames()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    GCF::IGDriveLiteContentModel *model = m_gDriveLite->contentModel();
    QHash<int,QByteArray> roles = model->roleNames();

#define VERIFY_ROLE(x) QVERIFY(roles[GCF::IGDriveLiteContentModel::x] == "content" #x)
    VERIFY_ROLE(Kind);
    VERIFY_ROLE(Id);
    VERIFY_ROLE(ETag);
    VERIFY_ROLE(SelfLink);
    VERIFY_ROLE(WebContentLink);
    VERIFY_ROLE(WebViewLink);
    VERIFY_ROLE(AlternateLink);
    VERIFY_ROLE(EmbedLink);
    VERIFY_ROLE(OpenWithLinks);
    VERIFY_ROLE(DefaultOpenWithLink);
    VERIFY_ROLE(IconLink);
    VERIFY_ROLE(ThumbnailLink);
    VERIFY_ROLE(Title);
    VERIFY_ROLE(MimeType);
    VERIFY_ROLE(Description);
    VERIFY_ROLE(IsStarred);
    VERIFY_ROLE(IsHidden);
    VERIFY_ROLE(IsTrashed);
    VERIFY_ROLE(IsRestricted);
    VERIFY_ROLE(IsViewed);
    VERIFY_ROLE(CreatedDate);
    VERIFY_ROLE(ModifiedDate);
    VERIFY_ROLE(ModifiedByMeDate);
    VERIFY_ROLE(LastViewedByMeDate);
    VERIFY_ROLE(SharedWithMeDate);
    VERIFY_ROLE(DownloadUrl);
    VERIFY_ROLE(ExportLinks);
    VERIFY_ROLE(IndexableText);
    VERIFY_ROLE(UserPermissions);
    VERIFY_ROLE(OriginalFileName);
    VERIFY_ROLE(FileExtension);
    VERIFY_ROLE(Md5Checksum);
    VERIFY_ROLE(FileSize);
    VERIFY_ROLE(QuataBytesUsed);
    VERIFY_ROLE(OwnerNames);
    VERIFY_ROLE(Owners);
    VERIFY_ROLE(LastModifyingUserName);
    VERIFY_ROLE(LastModifyingUser);
    VERIFY_ROLE(IsEditable);
    VERIFY_ROLE(IsCopyable);
    VERIFY_ROLE(CanWritersShare);
    VERIFY_ROLE(IsShared);
    VERIFY_ROLE(IsExplicitlyTrashed);
    VERIFY_ROLE(IsAppDataContents);
    VERIFY_ROLE(HeadRevisionId);
    VERIFY_ROLE(Properties);
    VERIFY_ROLE(ImageMediaMetaData);
    VERIFY_ROLE(IsFolder);
    VERIFY_ROLE(IsFile);
    VERIFY_ROLE(IsFileInfoLoaded);
    VERIFY_ROLE(IsFolderContentLoaded);
#undef VERIFY_ROLE

    QVERIFY(roles.count() == GCF::IGDriveLiteContentModel::MaxField-GCF::IGDriveLiteContentModel::MinField+1);

    QVERIFY(GCF::IGDriveLiteContentModel::MinField == GCF::IGDriveLiteContentModel::Kind);
    QVERIFY(GCF::IGDriveLiteContentModel::MaxField == GCF::IGDriveLiteContentModel::IsFolderContentLoaded);
}

void GDriveLiteTest::testSetColumns()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    GCF::IGDriveLiteContentModel *model = m_gDriveLite->contentModel();

    // When we call setColumns() with an empty list of fields,
    // the model should work with column-0 = Title only.
    QList<GCF::IGDriveLiteContentModel::Field> columns;
    model->setColumns(columns);
    QVERIFY(model->columnCount() == 1);
    QVERIFY(model->columns() == columns);
    QVERIFY(model->headerData(0,Qt::Horizontal,Qt::DisplayRole).toString() == "Title");

    // Lets show all fields in the columns
    columns.clear();
    for(int i=GCF::IGDriveLiteContentModel::MinField; i<=GCF::IGDriveLiteContentModel::MaxField; i++)
        columns << (GCF::IGDriveLiteContentModel::Field)i;
    model->setColumns(columns);
    QVERIFY(model->columnCount() == columns.count());
    QVERIFY(model->columns() == columns);
    for(int i=0; i<columns.count(); i++)
    {
        QString expectedTitle = QString::fromLatin1( model->roleNames().value( columns.at(i) ) );
        expectedTitle.remove(0, 7);
        QString givenTitle = model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
        if(expectedTitle != givenTitle)
            qDebug() << "At" << i << "expecting" << expectedTitle << ", but got" << givenTitle;
        QVERIFY(expectedTitle == givenTitle);
    }

    // Must be possible to have duplicate columns
    columns.clear();
    columns << GCF::IGDriveLiteContentModel::Title;
    columns << GCF::IGDriveLiteContentModel::Id;
    columns << GCF::IGDriveLiteContentModel::Title;
    model->setColumns(columns);
    QVERIFY(model->columnCount() == columns.count());
    QVERIFY(model->columns() == columns);
    QVERIFY(model->headerData(0,Qt::Horizontal,Qt::DisplayRole).toString() == "Title");
    QVERIFY(model->headerData(1,Qt::Horizontal,Qt::DisplayRole).toString() == "Id");
    QVERIFY(model->headerData(2,Qt::Horizontal,Qt::DisplayRole).toString() == "Title");
}

void GDriveLiteTest::testItem()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    GCF::IGDriveLiteContentModel *model = m_gDriveLite->contentModel();

    // Roots must not have any item
    for(int i=0; i<model->rowCount(); i++)
    {
        QModelIndex index = model->index(i, 0);
        GCF::GDriveContent::Item item = model->item(index);
        QVERIFY(item.isValid() == false);
    }

    // Any of the children must have items.
    // We wont check this recursively.
    QModelIndex myDriveIndex = model->specialItemIndex(GCF::IGDriveLiteContentModel::MyDrive);
    QVERIFY(myDriveIndex.isValid());
    QModelIndex myItemIndex = model->index(0, 0, myDriveIndex);
    GCF::GDriveContent::Item myItem = model->item(myItemIndex);
    QVERIFY(myItem.isValid());
}

void GDriveLiteTest::testDownloader()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    const int fileCount = qMin(m_fileItemList.count(), 5);
    for(int i=0; i<fileCount; i++)
    {
        GCF::GDriveContent::Item fileItem = m_fileItemList.at(i);
        QString fileId = fileItem.id();

        GCF::IGDriveLiteFileDownloader *downloader = m_gDriveLite->createFileDownloader(fileId);
        QVERIFY(downloader != 0);
        QVERIFY(downloader->id() == fileId);
        QVERIFY(downloader->isDownloading() == false);
        QVERIFY(downloader->isDownloadComplete() == false);
        QVERIFY(downloader->preferredDownloadMimeType().isEmpty());
        QVERIFY(downloader->usedDownloadMimeType().isEmpty());
        QVERIFY(downloader->downloadedFileName().isEmpty());
        QVERIFY(downloader->downloadProgress() == 0);
        QVERIFY(downloader->downloadCompleteResult().isSuccess() == false);

        // Try aborting download, shouldnt work
        GCF::Result result;
        result = downloader->abortDownload();
        QVERIFY(result.isSuccess() == false);
        QVERIFY(result.message() == "Job cannot be cancelled unless started");

        // Try retryDownload(), shouldnt work
        result = downloader->retryDownload();
        QVERIFY(result.isSuccess() == false);
        QVERIFY(result.message() == "Job was not started even once to retry");

        // Set preferred download to something that doesnt exist
        downloader->setPreferredDownloadMimeType("krishnan/prashanth/vcl");
        QVERIFY(downloader->preferredDownloadMimeType() == "krishnan/prashanth/vcl");
        QVERIFY(downloader->usedDownloadMimeType().isEmpty());

        // Attempt to download
        result = downloader->download();
        QVERIFY(result.isSuccess() == false);
        QVERIFY(result.message() == "No download-link available for mime-type 'krishnan/prashanth/vcl'");

        // Now reset the preferred type
        downloader->setPreferredDownloadMimeType(QString());

        // Install all the spies that we need and keep checking
        GCF::SignalSpy dlStartedSpy(downloader, SIGNAL(started(GCF::AbstractJob*)));
        GCF::SignalSpy dlCompleteSpy(downloader, SIGNAL(completed(GCF::AbstractJob*)));
        GCF::SignalSpy dlProgressSpy(downloader, SIGNAL(downloadProgressChanged(int)));

        // Begin downloading
        result = downloader->download();
        if(!result.isSuccess())
            qDebug() << result;
        QVERIFY(result.isSuccess() == true);

        // Check signal emission status right after download() function returns
        QVERIFY(downloader->isDownloading() == true);
        QVERIFY(downloader->isDownloadComplete() == false);
        QVERIFY(dlStartedSpy.count() == 1);
        QVERIFY(dlCompleteSpy.count() == 0);
        QVERIFY(dlProgressSpy.count() > 0);

        // Check download progress
        while(dlProgressSpy.wait())
        {
            int progress = downloader->downloadProgress();
            if(progress < 100)
            {
                QVERIFY(downloader->isDownloading() == true);
                QVERIFY(downloader->isDownloadComplete() == false);
            }

            if(dlProgressSpy.count())
                QVERIFY(dlProgressSpy.last().first().toInt() == progress);
            dlProgressSpy.clear();
            qDebug() << downloader->item().title() << "- downloaded" << progress << "percent";

            if(progress == 100 || dlCompleteSpy.count())
                break;
        }

        QVERIFY(downloader->downloadProgress() == 100);
        if(!downloader->isDownloadComplete())
            dlCompleteSpy.wait();

        qDebug() << "Downloaded" << downloader->item().title() << "to" << downloader->downloadedFileName();

        QVERIFY(downloader->isDownloading() == false);
        QVERIFY(downloader->isDownloadComplete() == true);
        QVERIFY(downloader->downloadCompleteResult().isSuccess() == true);

        delete downloader;
    }
}

void GDriveLiteTest::testDownloaderErrors()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());

    // Hopefully this ID doesnt exist ever!
    QString id = "abcdefghijklmnopqrstuvwxyz1234567890";
    GCF::IGDriveLiteFileDownloader *downloader = m_gDriveLite->createFileDownloader(id);
    GCF::Result result = downloader->download();
    delete downloader;
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "File not found: abcdefghijklmnopqrstuvwxyz1234567890");
}

void GDriveLiteTest::testDownloaderModel()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    QStringList ids;
    const int idsCount = qMin(m_fileItemList.count(), 5);
    for(int i=0; i<idsCount; i++)
        ids << m_fileItemList.at(i).id();

    GCF::IGDriveLiteFileDownloaderListModel *model = m_gDriveLite->downloaders();
    GCF::SignalSpy rowsInsertedSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    GCF::SignalSpy downloaderChangedSpy(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    GCF::SignalSpy allDownloadsCompleteSpy(model, SIGNAL(allDownloadsComplete()));

    QList<GCF::IGDriveLiteFileDownloader*> dlList;
    Q_FOREACH(QString id, ids)
    {
        GCF::IGDriveLiteFileDownloader *downloader = m_gDriveLite->createFileDownloader(id);
        GCF::Result result = downloader->download();
        if(!result.isSuccess())
            qDebug() << result;
        QVERIFY(result.isSuccess());
        dlList.append(downloader);
    }

    QVERIFY( model->rowCount() == ids.count() );

    int changedCount = 0;
    while(!allDownloadsCompleteSpy.wait(5000))
    {
        if(downloaderChangedSpy.count())
        {
            changedCount += downloaderChangedSpy.count();
            downloaderChangedSpy.clear();
        }

        this->printDownloadersStatus();
    }

    this->printDownloadersStatus();
    QVERIFY(allDownloadsCompleteSpy.count());
    QVERIFY( rowsInsertedSpy.count() == ids.count() );
    // QVERIFY( changedCount >= ids.count()*3 );

    // Delete all downloaded files
    Q_FOREACH(GCF::IGDriveLiteFileDownloader *dl, dlList)
        QFile::remove( dl->downloadedFileName() );

    GCF::SignalSpy rowsRemovedSpy(model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    qDeleteAll(dlList);
    QTest::qWait(100);
    QVERIFY( rowsRemovedSpy.count() == ids.count() );
    QVERIFY(model->rowCount() == 0);
}

void GDriveLiteTest::testUploaderAndDeletePermanently()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_folderItemList.count() >= 1);

    QString uploadFolderID = m_folderItemList.first().id();
    const QString uploadFileName = this->dataDirectory().absoluteFilePath("gcf3-logo.png");

    GCF::IGDriveLiteFileUploader *uploader = m_gDriveLite->createFileUploader(uploadFileName);
    QVERIFY(uploader != 0);
    QVERIFY(uploader->uploadFolderId().isEmpty());
    QVERIFY(uploader->isUploading() == false);
    QVERIFY(uploader->isUploadComplete() == false);
    QVERIFY(uploader->uploadFileName() == uploadFileName);
    QVERIFY(uploader->uploadProgress() == 0);
    QVERIFY(uploader->uploadCompleteResult().isSuccess() == false);

    // Try aborting upload, shouldnt work
    GCF::Result result;
    result = uploader->abortUpload();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job cannot be cancelled unless started");

    // Try retryUpload(), shouldnt work
    result = uploader->retryUpload();
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.message() == "Job was not started even once to retry");

    // Attempt to upload to invalid folder id
    result = uploader->upload("krishnan/prashanth/vcl");
    QVERIFY(result.isSuccess() == false);
    // QVERIFY(result.message() == "Requested folder-id is not valid");

    // Install all the spies that we need and keep checking
    GCF::SignalSpy ulStartedSpy(uploader, SIGNAL(started(GCF::AbstractJob*)));
    GCF::SignalSpy ulCompleteSpy(uploader, SIGNAL(completed(GCF::AbstractJob*)));
    GCF::SignalSpy ulProgressSpy(uploader, SIGNAL(uploadProgressChanged(int)));

    // Begin uploading
    uploader->setUploadFolderId(uploadFolderID);
    result = uploader->upload();
    if(!result.isSuccess())
        qDebug() << result;

    QVERIFY(result.isSuccess() == true);

    // Check signal emission status right after upload() function returns
    QVERIFY(uploader->isUploading() == true);
    QVERIFY(uploader->isUploadComplete() == false);
    QVERIFY(ulStartedSpy.count() == 1);
    QVERIFY(ulCompleteSpy.count() == 0);
    QVERIFY(ulProgressSpy.count() > 0);

    // Check upload progress
    while(ulProgressSpy.wait())
    {
        int progress = uploader->uploadProgress();
        if(progress < 100)
        {
            QVERIFY(uploader->isUploading() == true);
            QVERIFY(uploader->isUploadComplete() == false);
        }

        if(ulProgressSpy.count())
            QVERIFY(ulProgressSpy.last().first().toInt() == progress);
        ulProgressSpy.clear();

        if(progress == 100 || ulCompleteSpy.count())
            break;
    }

    QVERIFY(uploader->uploadProgress() == 100);
    if(!uploader->isUploadComplete())
        ulCompleteSpy.wait();

    QVERIFY(uploader->isUploading() == false);
    QVERIFY(uploader->isUploadComplete() == true);

    if(!uploader->uploadCompleteResult().isSuccess())
        qDebug() << "Upload failed. Error: " << uploader->uploadCompleteResult().message();
    QVERIFY(uploader->uploadCompleteResult().isSuccess() == true);
    GCF::GDriveContent::Item uploadedItem = uploader->uploadedItem();
    delete uploader;

    QTest::qWait(200);

    QVERIFY(uploadedItem.isValid() == true);
    QString uploadedFileId = uploadedItem.id();
    QVERIFY(uploadedItem.title() == QFileInfo(uploadFileName).fileName());

    ///////////////////////////////////////////////////////////////////////////
    /// Testing for delete permanently
    ///////////////////////////////////////////////////////////////////////////

    // Delete file
    GCF::Result deleteResult = m_gDriveLite->deletePermanently(uploadedFileId);
    QVERIFY(deleteResult.isSuccess());

    // Check that the file-id is no longer valid with the drive
    GDriveRequestAPI api(m_gDriveLite);
    result = api.get( QString("https://www.googleapis.com/drive/v2/files/%1").arg(uploadedFileId) );
    QVERIFY(result.isSuccess() == false);
    QVERIFY(result.code() == "404");

    GCF::IGDriveLiteContentModel* contentModel = m_gDriveLite->contentModel();
    QVERIFY(contentModel->item(uploadedFileId).isValid() == false);
}

void GDriveLiteTest::testUploaderModel()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated());
    QVERIFY(m_folderItemList.count() >= 1);

    GCF::GDriveContent::Item folderItem = m_folderItemList.first();
    qDebug() << "Uploading to" << folderItem.title();

    QString folderId = folderItem.id();
    GCF::IGDriveLiteFileUploaderListModel *model = m_gDriveLite->uploaders();
    GCF::SignalSpy rowsInsertedSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    GCF::SignalSpy uploaderChangedSpy(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    GCF::SignalSpy allUploadsCompleteSpy(model, SIGNAL(allUploadesComplete()));

    QDir dir = this->dataDirectory();
    QFileInfoList infoList = dir.entryInfoList(QDir::Files);
    QList<GCF::IGDriveLiteFileUploader*> ulList;
    Q_FOREACH(QFileInfo info, infoList)
    {
        GCF::IGDriveLiteFileUploader *uploader = m_gDriveLite->createFileUploader(info.absoluteFilePath());
        GCF::Result result = uploader->upload(folderId);
        if(!result.isSuccess())
            qDebug() << result;
        QVERIFY(result.isSuccess());
        ulList.append(uploader);
    }

    if(model->rowCount() != infoList.count())
        qDebug() << model->rowCount() << infoList.count();

    QVERIFY( model->rowCount() == infoList.count() );

    int changedCount = 0;
    while(!allUploadsCompleteSpy.wait(5000))
    {
        if(uploaderChangedSpy.count())
        {
            changedCount += uploaderChangedSpy.count();
            uploaderChangedSpy.clear();
        }

        this->printUploadersStatus();
    }

    this->printUploadersStatus();
    QVERIFY(allUploadsCompleteSpy.count());
    QVERIFY( rowsInsertedSpy.count() == infoList.count() );
    QVERIFY( changedCount >= infoList.count()*3 );

    // Delete all the uploaded files
    Q_FOREACH(GCF::IGDriveLiteFileUploader *ul, ulList)
    {
        QString itemId = ul->uploadedItem().id();
        if(itemId.isEmpty())
            continue;

        m_gDriveLite->deletePermanently(itemId);
    }

    GCF::SignalSpy rowsRemovedSpy(model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    qDeleteAll(ulList);
    QTest::qWait(100);
    QVERIFY( rowsRemovedSpy.count() == infoList.count() );
    QVERIFY(model->rowCount() == 0);
}

void GDriveLiteTest::testSharing()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    // We will pick some file. Share it, unshare it and see if the permissions
    // for that are getting updated or not.
    GCF::GDriveContent::Item shareFileItem = m_fileItemList.at( m_fileItemList.count()/2 );
    QString shareFileId = shareFileItem.id();

    GCF::IGDriveLiteContentSharingModel *sharingModel = m_gDriveLite->createSharingModel(shareFileId);
    QVERIFY(sharingModel);
    GCF::SignalSpy spy(sharingModel, SIGNAL(modified()));
    QVERIFY(spy.wait());
    QVERIFY(sharingModel->rowCount() >= 1);
    spy.clear();

    int rowCount = sharingModel->rowCount();

    // Share the file
    GCF::GDriveLiteShareRequest request;
    request.EMailAddress = "prashanth.udupa@gmail.com";
    request.EMailMessage = "Sharing this file from tst_GDriveLiteTest.cpp";
    request.Role = GCF::IGDriveLiteContentSharingModel::Writer;
    request.SendNotificationEMails = true;
    GCF::Result result = sharingModel->share(request);
    if(!result.isSuccess())
        qDebug() << "Error while sharing: " << result.message();
    QVERIFY(result.isSuccess() == true);
    QVERIFY(spy.count() >= 1);
    QVERIFY(sharingModel->rowCount() == rowCount+1);
    spy.clear();

    // Test data from sharing information
    QModelIndex index = sharingModel->index( sharingModel->rowCount()-1 );
    QVERIFY(index.data(GCF::IGDriveLiteContentSharingModel::Name).toString() == "Prashanth Udupa");
    QVERIFY(index.data(GCF::IGDriveLiteContentSharingModel::Domain).toString() == "gmail.com");
    QVERIFY(index.data(GCF::IGDriveLiteContentSharingModel::RoleType).toInt() == GCF::IGDriveLiteContentSharingModel::Writer);

    // Unshare the file
    result = sharingModel->unshare(index);
    if(!result.isSuccess())
        qDebug() << "Error while sharing: " << result.message();
    QVERIFY(result.isSuccess() == true);
    QVERIFY(spy.count() >= 1);
    QVERIFY(sharingModel->rowCount() == rowCount);
    spy.clear();

    delete sharingModel;
}

void GDriveLiteTest::testTrashAndRestore()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();

    // Pick a file and ensure that it has not been trashed
    GCF::GDriveContent::Item fileItem = m_fileItemList.at( m_fileItemList.count()/2 );
    QString fileId = fileItem.id();
    QVERIFY(m_gDriveLite->contentModel()->item(fileId).isTrashed() == false);

    // Find out the folder in which the file exists
    QModelIndex fileIndex = contentModel->indexForId(fileId);
    QModelIndex fileFolderIndex = fileIndex.parent();

    // Trash the file
    GCF::Result result = m_gDriveLite->trash(fileId);
    QVERIFY(result.isSuccess());
    QVERIFY(contentModel->item(fileId).isTrashed() == true);

    // Ensure that the file doesnt exist in the folder
    int folderCount = contentModel->rowCount(fileFolderIndex);
    for(int i=0; i<folderCount; i++)
    {
        QModelIndex index = contentModel->index(i, 0, fileFolderIndex);
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Id).toString() != fileId);
    }

    // Ensure that the file shows up in the trash.
    QModelIndex trashModelIndex = contentModel->specialItemIndex(GCF::IGDriveLiteContentModel::Trash);
    int trashCount = contentModel->rowCount(trashModelIndex);
    bool fileFound = false;
    for(int i=0; i<trashCount; i++)
    {
        QModelIndex index = contentModel->index(i, 0, trashModelIndex);
        if(index.data(GCF::IGDriveLiteContentModel::Id).toString() == fileId)
        {
            fileFound = true;
            break;
        }
    }
    QVERIFY(fileFound);

    // Restore the file
    result = m_gDriveLite->restore(fileId);
    QVERIFY(result.isSuccess() == true);

    // Ensure that the file isnt in trash anymore
    trashCount = contentModel->rowCount(trashModelIndex);
    for(int i=0; i<trashCount; i++)
    {
        QModelIndex index = contentModel->index(i, 0, trashModelIndex);
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Id).toString() != fileId);
    }

    // Ensure that the file is back in the folder, from where we had originally deleted it
    folderCount = contentModel->rowCount(fileFolderIndex);
    fileFound = false;
    for(int i=folderCount-1; i>=0; i--)
    {
        QModelIndex index = contentModel->index(i, 0, fileFolderIndex);
        if(index.data(GCF::IGDriveLiteContentModel::Id).toString() == fileId)
        {
            fileFound = true;
            break;
        }
    }
    QVERIFY(fileFound);
}

void GDriveLiteTest::testRename()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    // Pick a file and rename it
    GCF::GDriveContent::Item fileItem = m_fileItemList.at( m_fileItemList.count()/2 );
    const QString originalName = fileItem.title();
    const QString newName = QUuid::createUuid().toString();
    GCF::Result result = m_gDriveLite->rename(fileItem.id(), newName);
    QVERIFY(result.isSuccess() == true);

    // Check if the new name is reflected in the content model also
    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    QModelIndex index = contentModel->indexForId(fileItem.id());
    if(index.isValid())
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Title).toString() == newName);

    // Verify if the new name has reached the server
    GDriveRequestAPI api(m_gDriveLite);
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.title() == newName);
    }

    // Rename it back to the original name name
    result = m_gDriveLite->rename(fileItem.id(), originalName);
    QVERIFY(result.isSuccess());

    // Check if the changed name is reflected in the content model also
    index = contentModel->indexForId(fileItem.id());
    if(index.isValid())
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Title).toString() == originalName);

    // Verify if the orginal name has reached the server
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.title() == originalName);
    }
}

void GDriveLiteTest::testDescribe()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    // Pick a file and rename it
    GCF::GDriveContent::Item fileItem = m_fileItemList.at( m_fileItemList.count()/2 );
    const QString originalDesc = fileItem.title();
    const QString newDesc = QUuid::createUuid().toString();
    GCF::Result result = m_gDriveLite->describe(fileItem.id(), newDesc);
    QVERIFY(result.isSuccess() == true);

    // Check if the new name is reflected in the content model also
    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    QModelIndex index = contentModel->indexForId(fileItem.id());
    if(index.isValid())
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Description).toString() == newDesc);

    // Verify if the new name has reached the server
    GDriveRequestAPI api(m_gDriveLite);
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.description() == newDesc);
    }

    // Rename it back to the original name name
    result = m_gDriveLite->describe(fileItem.id(), originalDesc);
    QVERIFY(result.isSuccess());

    // Check if the changed name is reflected in the content model also
    index = contentModel->indexForId(fileItem.id());
    if(index.isValid())
        QVERIFY(index.data(GCF::IGDriveLiteContentModel::Description).toString() == originalDesc);

    // Verify if the orginal name has reached the server
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.description() == originalDesc);
    }
}

void GDriveLiteTest::testMove()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_folderItemList.count() >= 2);
    QVERIFY(m_fileItemList.count() >= 2);

    // Pick a file for moving
    GCF::GDriveContent::Item fileItem = m_fileItemList.at( m_fileItemList.count()/2 );

    // Pick a folder to move it into
    GCF::GDriveContent::Item originalFolderItem;
    GCF::GDriveContent::Item moveFolderItem;
    Q_FOREACH(GCF::GDriveContent::Item folderItem, m_folderItemList)
    {
        Q_FOREACH(GCF::GDriveContent::ItemParent parent, fileItem.parents())
        {
            if(parent.id() == folderItem.id())
                originalFolderItem = folderItem;
            else if(!moveFolderItem.isValid())
                moveFolderItem = folderItem;

            if(originalFolderItem.isValid() && moveFolderItem.isValid())
                break;
        }
    }

    // Get hold of the content model
    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    QModelIndex originalFolderIndex = contentModel->indexForId(originalFolderItem.id());
    QModelIndex moveFolderIndex = contentModel->indexForId(moveFolderItem.id());

    // Move the file to the new folder
    GCF::Result result = m_gDriveLite->move(fileItem.id(), moveFolderItem.id());
    QVERIFY(result.isSuccess());

    // Check if the file was actually moved in the content model also!
    if(originalFolderIndex.isValid() && moveFolderIndex.isValid())
    {
        QModelIndex index = contentModel->indexForId(fileItem.id());
        QVERIFY(index.parent() != originalFolderIndex);
        QVERIFY(index.parent() == moveFolderIndex);
    }

    // Now move back
    result = m_gDriveLite->move(fileItem.id(), originalFolderItem.id());
    QVERIFY(result.isSuccess());

    // Check if the file was actually moved in the content model also!
    if(originalFolderIndex.isValid() && moveFolderIndex.isValid())
    {
        QModelIndex index = contentModel->indexForId(fileItem.id());
        QVERIFY(index.parent() == originalFolderIndex);
        QVERIFY(index.parent() != moveFolderIndex);
    }
}

void GDriveLiteTest::testStarUnstar()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_fileItemList.count() >= 2);

    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    GCF::GDriveContent::Item fileItem = m_fileItemList.at( m_fileItemList.count()/2 );

    // Star the file
    GCF::Result result = m_gDriveLite->star(fileItem.id());
    QVERIFY(result.isSuccess());

    // Fetch if the file is actually starred on the server
    GDriveRequestAPI api(m_gDriveLite);
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.isStarred() == true);
    }

    // Ensure that the item in content model is updated
    QModelIndex index = contentModel->indexForId(fileItem.id());
    QVERIFY(index.data(GCF::IGDriveLiteContentModel::IsStarred).toBool());

    // Unstar the file
    result = m_gDriveLite->unstar(fileItem.id());
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(fileItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.isStarred() == false);
    }

    // Ensure that the item in content model is updated
    index = contentModel->indexForId(fileItem.id());
    QVERIFY(index.data(GCF::IGDriveLiteContentModel::IsStarred).toBool() == false);
}

void GDriveLiteTest::testMkdir()
{
    QVERIFY(m_settings.count());
    QVERIFY(m_gDriveLite->isAuthenticated() && m_gDriveLite->authenticatedUser().isValid());
    QVERIFY(m_folderItemList.count() >= 1);

    const GCF::GDriveContent::Item folderItem = m_folderItemList.first();
    const QString folderId = folderItem.id();
    const QString newFolderName = QUuid::createUuid().toString();
    GCF::IGDriveLiteContentModel* contentModel = m_gDriveLite->contentModel();

    // Create the folder
    GCF::Result result = m_gDriveLite->mkdir(newFolderName, folderId);

    // Verify the newly created folder
    GCF::GDriveContent::Item newFolderItem( result.data().toMap() );
    QVERIFY(newFolderItem.title() == newFolderName);

    // Fetch if the new folder is actually created on the server
    GDriveRequestAPI api(m_gDriveLite);
    result = api.get(QString("https://www.googleapis.com/drive/v2/files/%1").arg(newFolderItem.id()));
    if(result.isSuccess())
    {
        GCF::GDriveContent::Item item(result.data().toMap());
        QVERIFY(item.id() == newFolderItem.id());
        QVERIFY(item.title() == newFolderItem.title());
        QVERIFY(item.isFolder());
    }

    // Check if the content model has gotten updated
    QModelIndex folderIndex = contentModel->indexForId(folderId);
    if(folderIndex.isValid())
    {
        bool found = false;
        for(int i=contentModel->rowCount(folderIndex)-1; i>=0; i--)
        {
            QModelIndex index = contentModel->index(i,0,folderIndex);
            if(index.data(GCF::IGDriveLiteContentModel::Id).toString() == newFolderItem.id())
            {
                found = true;
                break;
            }
        }

        QVERIFY(found);
    }

    // Delete the newly created directory
    result = m_gDriveLite->deletePermanently(newFolderItem.id());
    QVERIFY(result.isSuccess());
}

void GDriveLiteTest::verifyContentModel()
{
    // Fully load the content model
    QElapsedTimer timer;
    timer.start();
    QVERIFY(this->ensureContentModelIsFullyLoaded());
    qDebug() << "Content model was loaded in " << timer.elapsed() << " milliseconds";

    // Now perform the comparison
    QStringList expected = this->expectedContents();
    QStringList given = this->dumpModel();
    if(given.count() != expected.count())
    {
        qDebug() << "Expected" << expected.count() << ", but got" << given.count() << "items";
        if(given.count() > expected.count())
        {
            for(int i=0; i<expected.count(); i++)
                given.removeAll( expected.at(i) );
            qDebug() << " Extra given: " << given;
        }
        else
        {
            for(int i=0; i<given.count(); i++)
                expected.removeAll( given.at(i) );
            qDebug() << " Extra expected: " << expected;
        }

        QFAIL("Expected and given counts dont match");
    }

    for(int i=0; i<expected.count(); i++)
    {
        QString exp = expected.at(i);
        QString expId = exp.contains(':') ? exp.section(':', 0, 0) : "null";
        QString gvn = given.at(i);
        QString gvnId = gvn.contains(':') ? gvn.section(':', 0, 0) : "null";
        if( expId != gvnId )
            qDebug() << i << "Expected" << exp << ", but got" << gvn;
        QVERIFY(expId == gvnId);
    }
}

bool GDriveLiteTest::ensureContentModelIsFullyLoaded()
{
    QElapsedTimer timer;

    GCF::IGDriveLiteContentModel *contentModel = m_gDriveLite->contentModel();
    timer.start();
    while(contentModel->rowCount() == 0)
    {
        if(timer.elapsed() > 20000)
            return false;

        GCF::SignalSpy spy(contentModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
        spy.wait();
    }

    for(int i=0; i<contentModel->rowCount(); i++)
    {
        QModelIndex index = contentModel->index(i, 0);
        if( !this->ensureFolderIsFullyLoaded(index) )
            return false;
    }

    return true;
}

bool GDriveLiteTest::ensureFolderIsFullyLoaded(const QModelIndex &folderIndex)
{
    if(!folderIndex.isValid())
        return false;

    if(!this->verifyModelIndexFlags(folderIndex))
    {
        qDebug() << folderIndex.data(GCF::IGDriveLiteContentModel::Title).toString()
                 << " doesnt have appropriate flags set";
        return false;
    }

    // First ensure that the folder content (as in list of its children) is fully loaded
    qDebug() << "Waiting for folder "
             << folderIndex.data(GCF::IGDriveLiteContentModel::Title).toString()
             << " to get loaded";
    while(!folderIndex.data(GCF::IGDriveLiteContentModel::IsFolderContentLoaded).toBool())
    {
        // Ask for information of the first child of this folder. Thats when the folder
        // information loading is triggered
        QModelIndex firstChild = folderIndex.model()->index(0, 0, folderIndex);
        firstChild.data(GCF::IGDriveLiteContentModel::Title);

        // Wait for the folder-item to be loaded
        GCF::SignalSpy folderSpy(m_gDriveLite->contentModel(), SIGNAL(folderItemLoaded(QModelIndex)));
        folderSpy.wait();
    }

    // Now ensure that each and every child index is fully loaded.
    int childrenCount = folderIndex.model()->rowCount(folderIndex);
    for(int i=0; i<childrenCount; i++)
    {
        QModelIndex childIndex = folderIndex.model()->index(i, 0, folderIndex);

        // Ensure that the child is loaded
        while(!childIndex.data(GCF::IGDriveLiteContentModel::IsFileInfoLoaded).toBool())
        {
            // Wait for 20 seconds for the folder to load
            GCF::SignalSpy fileSpy(m_gDriveLite->contentModel(), SIGNAL(fileItemLoaded(QModelIndex)));
            fileSpy.wait();
        }
        qDebug() << "  " << (i+1) << "/" << childrenCount << " - "
                 << childIndex.data(GCF::IGDriveLiteContentModel::Title).toString();

        if(!this->verifyModelIndexFlags(folderIndex))
        {
            qDebug() << childIndex.data(GCF::IGDriveLiteContentModel::Title).toString()
                     << " doesnt have appropriate flags set";
            return false;
        }
    }

    // Now recursively dig into the folders within folderIndex
    for(int i=0; i<childrenCount; i++)
    {
        QModelIndex childIndex = folderIndex.model()->index(i, 0, folderIndex);
        if(childIndex.data(GCF::IGDriveLiteContentModel::IsFolder).toBool())
        {
            if( !this->ensureFolderIsFullyLoaded(childIndex) )
                return false;
        }
    }

    return true;
}

bool GDriveLiteTest::verifyModelIndexFlags(const QModelIndex &index)
{
    if(!index.isValid())
        return true;

    GCF::GDriveContent::Item item = m_gDriveLite->contentModel()->item(index);
    if(item.isValid())
        return (index.flags()&Qt::ItemIsEnabled) && (index.flags()&Qt::ItemIsSelectable);

    if(index.parent() == QModelIndex())
        return (index.flags()&Qt::ItemIsEnabled) && (index.flags()&Qt::ItemIsSelectable);

    return 0;
}

QStringList GDriveLiteTest::dumpModel()
{
    QStringList retList;

    for(int i=0; i<m_gDriveLite->contentModel()->rowCount(); i++)
    {
        QModelIndex index = m_gDriveLite->contentModel()->index(i, 0);
        retList += this->dumpModelIndex(QString(), index);
    }

    qSort(retList);

    return retList;
}

QStringList GDriveLiteTest::dumpModelIndex(const QString &path, const QModelIndex &index)
{
    QStringList retList;

    QString id = index.data(GCF::IGDriveLiteContentModel::Id).toString();
    QString path2 = QString("%1/%2").arg(path).arg(index.data(GCF::IGDriveLiteContentModel::Title).toString());
    retList << QString("%1: %2").arg(id).arg(path2);

    int count = index.model()->rowCount(index);
    for(int i=0; i<count; i++)
    {
        QModelIndex childIndex = index.model()->index(i, 0, index);
        retList += this->dumpModelIndex(path2, childIndex);
    }

    return retList;
}

QStringList GDriveLiteTest::expectedContents()
{
    return m_gDriveContentTitleIdMap.keys();
}

QString GDriveLiteTest::logFileContents(bool deleteFile) const
{
    QString retString;
    {
        QFile file( GCF::Log::instance()->logFileName() );
        file.open( QFile::ReadOnly );
        retString = file.readAll();
    }

    if(deleteFile)
        QFile::remove( GCF::Log::instance()->logFileName() );
    return retString;
}

void GDriveLiteTest::printDownloadersStatus()
{
    for(int i=0; i<m_gDriveLite->downloaders()->downloaderCount(); i++)
    {
        GCF::IGDriveLiteFileDownloader *dl = m_gDriveLite->downloaders()->downloaderAt(i);

        QString name, status;
        name = dl->item().title();
        if(name.isEmpty())
            name = dl->id();

        if(dl->isDownloadComplete())
            if(dl->downloadCompleteResult().isSuccess())
                status = "Complete";
            else
                status = QString("Complete: %1").arg(dl->downloadCompleteResult().message());
        else
            status = QString("Downloading %1 percent").arg(dl->downloadProgress());

        qDebug("%d: %s %s", i, qPrintable(name), qPrintable(status));
    }
    qDebug() << "----";
}

void GDriveLiteTest::printUploadersStatus()
{
    for(int i=0; i<m_gDriveLite->uploaders()->uploaderCount(); i++)
    {
        GCF::IGDriveLiteFileUploader *ul = m_gDriveLite->uploaders()->uploaderAt(i);

        QString uploadFilename = QFileInfo(ul->uploadFileName()).baseName();
        QString status;

        if(ul->isUploadComplete())
            if(ul->uploadCompleteResult().isSuccess())
                status = "Complete";
            else
                status = QString("Complete: %1").arg(ul->uploadCompleteResult().message());
        else
            status = QString("Uploading %1 percent").arg(ul->uploadProgress());

        qDebug("%d: %s %s", i, qPrintable(uploadFilename), qPrintable(status));
    }
    qDebug() << "----";
}

struct FunctionTimer {
    FunctionTimer() { m_timer.start(); }
    ~FunctionTimer() {
        qDebug() << "Finished in " << m_timer.elapsed();
    }

    qint64 elapsedTime() const { return m_timer.elapsed(); }

private:
    QElapsedTimer m_timer;
};

void GDriveLiteTest::fetchDriveContents()
{
    // This function will fetch contents of the entire google-drive
    // And constructs a simple content map. The function is long
    // and will take a long long time to finish. Really depends
    // on the size of the google-drive itself.

    qDebug() << "Started fetching drive contents";
    FunctionTimer funcTimer;

    // Information of all items in the drive will land in
    // this structure.
    QVariantList items;

    GDriveRequestAPI api(m_gDriveLite);
    api.addQuery("maxResults", 1000);
    while(1)
    {
        GCF::Result result = api.get("https://www.googleapis.com/drive/v2/files");
        if(!result.isSuccess())
        {
            qDebug() << "Failed to GET from https://www.googleapis.com/drive/v2/files";
            qDebug() << "    " << result.message();
            return;
        }

        QVariantMap response = result.data().toMap();
        // We assume that the response has no error!

        items.append( response.value("items").toList() );

        QVariant nextPageToken = response.value("nextPageToken");
        if(nextPageToken.isNull() || !nextPageToken.isValid())
            break;
        if(nextPageToken == api.query().value("pageToken"))
            break;

        api.addQuery("pageToken", nextPageToken);
    }

    qDebug() << "Fetched drive contents in " << funcTimer.elapsedTime() << " ms";

    GCF::ObjectTree driveContentsTree;

    QVariantMap info;
    info["title"] = "My Drive";
    info["id"] = m_gDriveLite->authenticatedUser().rootFolderId();
    GCF::ObjectTreeNode *myDriveNode = new GCF::ObjectTreeNode(driveContentsTree.rootNode(),
                                                               m_gDriveLite->authenticatedUser().rootFolderId(),
                                                               this, info);

    info["title"] = "Trash";
    info["id"] = "trash";
    GCF::ObjectTreeNode *trashNode = new GCF::ObjectTreeNode(driveContentsTree.rootNode(),
                                                             "trash", this, info);

    info["title"] = "Shared with me";
    info["id"] = "sharedWithMe";
    GCF::ObjectTreeNode *sharedWithMeNode = new GCF::ObjectTreeNode(driveContentsTree.rootNode(),
                                                                    "sharedWithMe", this, info);

    info["title"] = "Starred";
    info["id"] = "starred";
    GCF::ObjectTreeNode *starredNode = new GCF::ObjectTreeNode(driveContentsTree.rootNode(),
                                                               "starred", this, info);

    while(items.count())
    {
        QList<int> toRemoveList;

        for(int i=0; i<items.count(); i++)
        {
            QVariantMap itemData = items.at(i).toMap();
            GCF::GDriveContent::Item item(itemData);

            // Include in trashed if not already done so
            if(item.isTrashed())
            {
                if(!trashNode->node(item.id()))
                    new GCF::ObjectTreeNode(trashNode, item.id(), this, itemData);
                toRemoveList.append(i);
                continue;
            }

            // Include in shared if not already done so
            if(item.isShared() && item.sharedWithMeDate().isValid())
            {
                if(!sharedWithMeNode->node(item.id()))
                    new GCF::ObjectTreeNode(sharedWithMeNode, item.id(), this, itemData);
            }

            // Include in starred if not already done so
            if(item.isStarred())
            {
                if(!starredNode->node(item.id()))
                    new GCF::ObjectTreeNode(starredNode, item.id(), this, itemData);
            }

            // Include it in all the found parents
            QList<GCF::GDriveContent::ItemParent> parents = item.parents();
            bool allParentsFound = true;
            Q_FOREACH(GCF::GDriveContent::ItemParent parent, parents)
            {
                QList<GCF::ObjectTreeNode*> parentNodes = this->contentNodes(parent.id(), &driveContentsTree);
                if(parentNodes.count())
                {
                    Q_FOREACH(GCF::ObjectTreeNode *parentNode, parentNodes)
                    {
                        if(!parentNode->node(item.id()))
                            new GCF::ObjectTreeNode(parentNode, item.id(), this, itemData);
                    }
                }
                else
                    allParentsFound = false;
            }

            if(allParentsFound)
                toRemoveList.append(i);
        }

        if(toRemoveList.count() == 0)
        {
            qDebug() << "Could not find a way to include the following items into the content tree";
            Q_FOREACH(QVariant item, items)
            {
                GCF::GDriveContent::Item citem(item.toMap());
                qDebug() << citem.id() << citem.title();
            }

            break;
        }

        // Remove all items that have been completely processed
        for(int i=toRemoveList.count()-1; i>=0; i--)
            items.removeAt(toRemoveList.at(i));
    }

    // Populate m_

    // Now construct a map of gdrive contents
    m_gDriveContentTitleIdMap.clear();
    m_fileItemList.clear();
    m_folderItemList.clear();
    this->populateGDriveContentInformation(myDriveNode);
    this->populateGDriveContentInformation(sharedWithMeNode);
    this->populateGDriveContentInformation(trashNode);
    this->populateGDriveContentInformation(starredNode);

    // Sort the lists
    this->sortFileItemList();
    this->sortFolderItemList();
}

void GDriveLiteTest::populateGDriveContentInformation(GCF::ObjectTreeNode *node, const QString &path)
{
    GCF::GDriveContent::Item item(node->info());
    QString newPath = QString("%1/%2").arg(path).arg(item.title());
    QString key = QString("%1: %2").arg(item.id()).arg(newPath);
    m_gDriveContentTitleIdMap[key] = item.data();

    if(item.isFolder() && node->children().count() == 0)
    {
        key = "null: " + newPath + "/Folder empty";
        m_gDriveContentTitleIdMap[key] = "null";
    }
    else
    {
        for(int i=0; i<node->children().count(); i++)
            this->populateGDriveContentInformation(node->children().at(i), newPath);
    }

    if(item.isTrashed() || item.isShared() || node->parent() == node->owningTree()->rootNode())
        return;

    if(item.isFolder())
    {
        if(node->children().count())
        {
            QVariantMap data = item.data();
            data["_childCount"] = node->children().count();
            m_folderItemList.append(GCF::GDriveContent::Item(data));
        }
    }
    else
        m_fileItemList.append(item);
}

bool fileItemLessThanFunction(const GCF::GDriveContent::Item &i1, const GCF::GDriveContent::Item &i2)
{
    return i1.fileSize() < i2.fileSize();
}

void GDriveLiteTest::sortFileItemList()
{
    qSort(m_fileItemList.begin(), m_fileItemList.end(), fileItemLessThanFunction);
}

bool folderItemLessThanFunction(const GCF::GDriveContent::Item &i1, const GCF::GDriveContent::Item &i2)
{
    return i1.data().value("_childCount").toInt() < i2.data().value("_childCount").toInt();
}

void GDriveLiteTest::sortFolderItemList()
{
    qSort(m_folderItemList.begin(), m_folderItemList.end(), folderItemLessThanFunction);
}

int main(int argc, char *argv[])
{
    GCF::GuiApplication app(argc, argv);
    GDriveLiteTest tc;
    if(tc.isValid())
        return QTest::qExec(&tc, argc, argv);

    return 0;
}

#include "tst_GDriveLiteTest.moc"
