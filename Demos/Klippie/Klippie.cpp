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

#include "Klippie.h"

#include <GCF3/QmlApplication>
#include <GCF3/ObjectTree>

Klippie::Klippie(QObject *parent)
    : QObject(parent)
{
    m_model = new KlippieModel(this);

    QVariantMap info;
    info["allowmetaaccess"] = true;
    new GCF::ObjectTreeNode(gAppService->objectTree()->rootNode(), "KlippieModel", m_model, info);

    m_server.listen();

    connect(&m_discovery, SIGNAL(foundServer(GCF::IpcServerInfo)),
            this, SLOT(onFoundServer(GCF::IpcServerInfo)));
    m_discovery.start(50006);
}

Klippie::~Klippie()
{

}

void Klippie::onFoundServer(const GCF::IpcServerInfo &info)
{
    GCF::IpcRemoteObject *remoteKlippyModel
            = new GCF::IpcRemoteObject(info.Address, info.Port, "Application.KlippieModel", this);
    remoteKlippyModel->setObjectName( QString("%1 (%2:%3)").arg(info.User).arg(info.Address.toString()).arg(info.Port) );

    connect(remoteKlippyModel, SIGNAL(activated()),
            this, SLOT(onRemoteKlippieModelActivated()));
    connect(remoteKlippyModel, SIGNAL(deactivated()),
            this, SLOT(onRemoteKlippieModelDeactivated()));
    connect(remoteKlippyModel, SIGNAL(couldNotActivate(QString)),
            this, SLOT(onRemoteKlippieModelCouldNotActivate(QString)));

    emit notice( QString("Found %1").arg(remoteKlippyModel->objectName()) );
}

void Klippie::onRemoteKlippieModelActivated()
{
    GCF::IpcRemoteObject *remoteKlippyModel = qobject_cast<GCF::IpcRemoteObject*>(this->sender());

    GCF::Result result = GCF::ipcConnect(remoteKlippyModel, SIGNAL(newItem(QVariant)), m_model, SLOT(prependItem(QVariant)));
    if(result.isSuccess())
        emit notice( QString("Syncing with %1").arg(remoteKlippyModel->objectName()) );
    else
    {
        emit notice( QString("Cannot sync with %1 because %2").arg(remoteKlippyModel->objectName()).arg(result.message()) );
        remoteKlippyModel->deleteLater();
    }
}

void Klippie::onRemoteKlippieModelDeactivated()
{
    GCF::IpcRemoteObject *remoteKlippyModel = qobject_cast<GCF::IpcRemoteObject*>(this->sender());
    remoteKlippyModel->deleteLater();

    emit notice( QString("Dectivated %1").arg(remoteKlippyModel->objectName()) );
}

void Klippie::onRemoteKlippieModelCouldNotActivate(const QString &message)
{
    GCF::IpcRemoteObject *remoteKlippyModel = qobject_cast<GCF::IpcRemoteObject*>(this->sender());
    remoteKlippyModel->deleteLater();

    emit notice( QString("Could not activate %1. Reason: %2").arg(remoteKlippyModel->objectName()).arg(message) );
}
