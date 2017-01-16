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

#ifndef CONNECTIONTESTCOMPONENT_H
#define CONNECTIONTESTCOMPONENT_H

#include <GCF3/Component>

#include "Sender.h"
#include "Receiver.h"

class ConnectionTestComponent : public GCF::Component
{
public:
    ConnectionTestComponent(QObject *parent=0) : GCF::Component(parent) { }

    QString name() const { return "Connections"; }

protected:
    ~ConnectionTestComponent() { }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/Connections.xml");
    }

    QObject *loadObject(const QString &name, const QVariantMap &info) {
        QString type = info.value("type").toString();
        if(type == "sender")
            return new Sender(this);
        else if(type == "receiver")
            return new Receiver(this);
        return GCF::Component::loadObject(name, info);
    }
};


#endif // CONNECTIONTESTCOMPONENT_H
