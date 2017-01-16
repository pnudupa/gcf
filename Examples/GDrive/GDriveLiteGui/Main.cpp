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

#include <GCF3/GuiApplication>
#include <GCF3/Component>
#include <GCF3/IGDriveLite>
#include <GCF3/IGDriveLiteAuthenticator>

#include <QMessageBox>
#include <QWebView>
#include <QHBoxLayout>
#include <QTimer>
#include <QTreeView>
#include <QProcessEnvironment>
#include <QWebFrame>
#include <QWebElement>
#include <QMouseEvent>
#include <QKeyEvent>

#include "DriveExplorerWindow.h"

int error(const QString &msg)
{
    qDebug("%s", qPrintable(msg));
    QMessageBox::information(0, "Error", msg);
    return -1;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    AuthDialog(const QUrl &url, QWidget *parent=0)
        : QDialog(parent) {

        this->setWindowTitle("Authentication Dialog");
        this->setLayout(new QHBoxLayout);

        m_webView = new QWebView(this);
        m_webView->setUrl(url);
        this->layout()->addWidget(m_webView);

        connect(m_webView, SIGNAL(loadFinished(bool)),
                this, SLOT(onLoadFinished(bool)));
        connect(m_webView, SIGNAL(titleChanged(QString)),
                this, SLOT(onTitleChanged(QString)));
    }

    QString title() const { return m_title; }

private slots:
    void onLoadFinished(bool val) {
        QString login = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_LOGIN");
        QString password = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_PASSWORD");
        if(val && !login.isEmpty() && !password.isEmpty()) {
            QWebFrame *frame = m_webView->page()->mainFrame();
            QWebElement email = frame->findFirstElement("#Email");
            email.setAttribute("value", login);
            QWebElement passwd = frame->findFirstElement("#Passwd");
            passwd.setAttribute("value", password);
        }
        disconnect(m_webView, SIGNAL(loadFinished(bool)),
                this, SLOT(onLoadFinished(bool)));
    }

    void onTitleChanged(const QString &title) {
        this->setWindowTitle(title);
        if(title.startsWith("Success code=")) {
            m_title = title;
            this->accept();
        }
    }

private:
    QString m_title;
    QWebView *m_webView;
};

int main(int argc, char **argv)
{
    GCF::GuiApplication a(argc, argv);
    a.processArguments(); // For test hooks

    QString clientID = a.argumentsMap().value("--clientID").toString();
    if(clientID.isEmpty())
        clientID = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_CLIENT_ID");
    QString clientSecret = a.argumentsMap().value("--clientSecret").toString();
    if(clientSecret.isEmpty())
        clientSecret = QProcessEnvironment::systemEnvironment().value("GDRIVE_TEST_CLIENT_SECRET");
    if(clientID.isEmpty() || clientSecret.isEmpty())
        return ::error("Please provide --clientID:xxx and --clientSecret:xxx commandline arguments");

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

    DriveExplorerWindow window(gDriveIface);
    new GCF::ObjectTreeNode(a.objectTree()->rootNode(), "DriveExplorerWindow", &window);
    window.showMaximized();

    AuthDialog authDialog(auth->authenticationPageUrl());
    new GCF::ObjectTreeNode(a.objectTree()->rootNode(), "AuthDialog", &authDialog);
    authDialog.exec();

    QString title = authDialog.title();
    if(title.isEmpty())
        return ::error("User did not authenticate properly");

    GCF::Result result = auth->authenticateFromPageTitle(title);
    if(!result.isSuccess())
        return ::error(result.message());

    return a.exec();
}

#include "Main.moc"
