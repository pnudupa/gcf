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

#ifndef KLIPPIE_H
#define KLIPPIE_H

#include <QObject>
#include <GCF3/IpcServerDiscovery>
#include <GCF3/IpcRemoteObject>
#include <GCF3/IpcServer>

#include "KlippieModel.h"

class Klippie : public QObject
{
    Q_OBJECT

public:
    Klippie(QObject *parent=nullptr);
    ~Klippie();

    KlippieModel *model() const { return m_model; }

signals:
    void notice(const QString &message);

private slots:
    void onFoundServer(const GCF::IpcServerInfo &info);
    void onRemoteKlippieModelActivated();
    void onRemoteKlippieModelDeactivated();
    void onRemoteKlippieModelCouldNotActivate(const QString &message);

private:
    KlippieModel *m_model;
    GCF::IpcServer m_server;
    GCF::IpcServerDiscovery m_discovery;
};

#endif // KLIPPIE_H
