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

#include "Authenticator.h"
#include "GDriveLiteContentListModel.h"
#include "ThumbnailImageProvider.h"
#include "GDriveLiteHelper.h"
#include "QmlDialog.h"

#include <GCF3/Component>
#include <GCF3/IGDriveLite>
#include <GCF3/QmlApplication>

#include <QtQml>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>

class NamFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QNetworkAccessManager *create(QObject *parent) {
        QNetworkAccessManager *nam = new QNetworkAccessManager(parent);
        nam->setCookieJar(new QNetworkCookieJar(nam));
        return nam;
    }
};

bool error(const QString &msg)
{
    QmlMessageBox messageBox(msg, false);
    messageBox.setTitle("Error");
    return messageBox.exec() == 1;
}

int main(int argc, char **argv)
{
    GCF::QmlApplication a(argc, argv);
    a.processArguments(); // For test hooks

    QString clientID = a.argumentsMap().value("--clientID").toString();
    if(clientID.isEmpty())
        clientID = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_CLIENT_ID");
    QString clientSecret = a.argumentsMap().value("--clientSecret").toString();
    if(clientSecret.isEmpty())
        clientSecret = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_CLIENT_SECRET");
    if(clientID.isEmpty() || clientSecret.isEmpty())
        return ::error("Please provide --clientID:xxx and --clientSecret:xxx command-line arguments");

    QList< QPair<QByteArray,QVariant> > props;
    props << qMakePair<QByteArray,QVariant>("clientID", clientID);
    props << qMakePair<QByteArray,QVariant>("clientSecret", clientSecret);

    GCF::Component *gDriveLite = a.loadComponent("GDrive/GDriveLite", props);
    if(!gDriveLite)
        return ::error("Could not load GDrive/GDriveLite component");

    GCF::IGDriveLite *gDriveIface = qobject_cast<GCF::IGDriveLite*>(gDriveLite);
    if(!gDriveLite)
        return ::error("GCF::IGDriveLite interface was not found");

    GCF::IGDriveLiteAuthenticator *auth = gDriveIface->authenticator();
    if(!auth)
        return ::error("GCF::IGDriveLiteAuthenticator interface was not found");

    Authenticator authenticator(gDriveIface);
    GDriveLiteContentListModel contentListModel(gDriveIface->contentModel());
    GDriveLiteHelper gDriveHelper(gDriveIface);

    NamFactory namFactory;

    QQuickView qmlView;
    qmlView.setTitle("GDriveLite Qml Example");
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView.engine()->setNetworkAccessManagerFactory(&namFactory);
    qmlView.engine()->rootContext()->setContextProperty("gDriveLite", gDriveIface);
    qmlView.engine()->rootContext()->setContextProperty("gDriveAuthenticator", &authenticator);
    qmlView.engine()->rootContext()->setContextProperty("gDriveContentModel", gDriveIface->contentModel());
    qmlView.engine()->rootContext()->setContextProperty("gDriveContentListModel", &contentListModel);
    qmlView.engine()->rootContext()->setContextProperty("gDriveHelper", &gDriveHelper);
    qmlView.engine()->addImageProvider("thumbnail", new ThumbnailImageProvider(gDriveIface));
    qmlView.setSource(QUrl("qrc:///Qml/MainWindow.qml"));
    qmlView.showMaximized();

    return a.exec();
}
