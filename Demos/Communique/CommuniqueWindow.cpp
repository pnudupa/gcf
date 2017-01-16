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

#include "CommuniqueWindow.h"
#include "ui_CommuniqueWindow.h"

#include <GCF3/Log>
#include <GCF3/IpcServer>
#include <GCF3/IpcRemoteObject>
#include <GCF3/MapToObject>

struct CommuniqueWindowData
{
    Ui::CommuniqueWindow ui;

    GCF::MapToObject<QString> remoteObjectMap;
    GCF::IpcServer server;
    GCF::IpcServerDiscovery discovery;
};

CommuniqueWindow::CommuniqueWindow(QWidget *parent) :
    QWidget(parent)
{
    d = new CommuniqueWindowData;

    d->ui.setupUi(this);

    d->server.listen();
    if(d->server.isListening())
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString(),
                                   QString("IpcServer listening on %1").arg(d->server.serverPort()));
    else
        GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT,
                                    QString(),
                                    QString("IpcServer not listening on any port: %1").arg(d->server.errorString()));

    connect(&d->discovery, SIGNAL(foundServer(GCF::IpcServerInfo)),
            this, SLOT(onFoundServer(GCF::IpcServerInfo)));
    d->discovery.start(55002);

    QString title = QString("%1 - %2").arg(GCF::IpcServerDiscovery::user()).arg(this->windowTitle());
    this->setWindowTitle(title);
}

CommuniqueWindow::~CommuniqueWindow()
{
    delete d;
}

void CommuniqueWindow::incomingMessage(const QString &from, const QString &msg)
{
    QString line = QString("<b>%1</b>: %2<br/>").arg(from).arg(msg);
    d->ui.txtChatHistory->append(line);
}

void CommuniqueWindow::onFoundServer(const GCF::IpcServerInfo &info)
{
    GCF::IpcRemoteObject *remoteObject
            = new GCF::IpcRemoteObject(info.Address, info.Port,
                                       "Application.CommuniqueWindow");
    remoteObject->setProperty("IpcServerInfo", QVariant::fromValue(info));
    connect(remoteObject, SIGNAL(activated()), this, SLOT(onServerActivated()));
    connect(remoteObject, SIGNAL(deactivated()), this, SLOT(onServerDeactivated()));
    connect(remoteObject, SIGNAL(couldNotActivate(QString)),
            this, SLOT(onServerCouldNotActivate(QString)));
}

void CommuniqueWindow::onServerActivated()
{
    GCF::IpcRemoteObject *remoteObject = qobject_cast<GCF::IpcRemoteObject*>(this->sender());
    if(!remoteObject)
        return;

    GCF::IpcServerInfo info = remoteObject->property("IpcServerInfo").value<GCF::IpcServerInfo>();
    if(d->remoteObjectMap.contains(info.User))
    {
        remoteObject->deleteLater();
        return;
    }

    QString line = QString("<i>User '%1' joined</i><br/>").arg(info.User);
    d->ui.txtChatHistory->append(line);

    GCF::ipcConnect(this, SIGNAL(outgoingMessage(QString,QString)),
                    remoteObject, SLOT(incomingMessage(QString,QString)));

    d->remoteObjectMap.insert(info.User, remoteObject);
}

void CommuniqueWindow::onServerDeactivated()
{
    GCF::IpcRemoteObject *remoteObject = qobject_cast<GCF::IpcRemoteObject*>(this->sender());
    if(!remoteObject)
        return;

    GCF::IpcServerInfo info = remoteObject->property("IpcServerInfo").value<GCF::IpcServerInfo>();
    QString line = QString("<i>User '%1' left</i><br/>").arg(info.User);
    d->ui.txtChatHistory->append(line);

    QList<GCF::IpcServerInfo> foundServers = d->discovery.foundServers();
    Q_FOREACH(GCF::IpcServerInfo foundServer, foundServers)
    {
        if(info.User == foundServer.User)
            d->discovery.releaseFoundServer(foundServer);
    }

    d->remoteObjectMap.remove(info.User);

    remoteObject->deleteLater();
}

void CommuniqueWindow::onServerCouldNotActivate(const QString &msg)
{
    GCF::IpcRemoteObject *remoteObject = qobject_cast<GCF::IpcRemoteObject*>(this->sender());
    if(!remoteObject)
        return;

    GCF::IpcServerInfo info = remoteObject->property("IpcServerInfo").value<GCF::IpcServerInfo>();
    QString line = QString("<i>'%1' could not join: %2</i><br/>").arg(info.User).arg(msg);
    d->ui.txtChatHistory->append(line);

    remoteObject->deleteLater();
}

void CommuniqueWindow::on_cmdSend_clicked()
{
    QString user = GCF::IpcServerDiscovery::user();
    QString msg = d->ui.txtChatText->text();
    if(msg.isEmpty())
        return;

    d->ui.txtChatText->clear();
    QString line = QString("<b>Me</b>: %1<br/>").arg(msg);
    d->ui.txtChatHistory->append(line);

    emit outgoingMessage(user, msg);
}
