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

#ifndef DIALCOMPONENT_H
#define DIALCOMPONENT_H

#include <GCF3/GuiComponent>

#include <QDial>

class DialComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    DialComponent(QObject *parent=0) : GCF::GuiComponent(parent) { }

    QString name() const { return "DialComponent"; }

signals:
    void dialValueChanged(int value);

protected slots:
    void onDialChanged(int value) {
        QMetaObject::invokeMethod(this, "dialValueChanged", Qt::QueuedConnection, Q_ARG(int,value));
    }

protected:
    ~DialComponent() { }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/SimpleGUI/DialComponent.xml");
        GCF::Component::contentLoadEvent(e);
    }

    QWidget *loadWidget(const QString &name, const QVariantMap &info) {
        if(name == "Dial") {
            QDial *dial = new QDial;
            connect(dial, SIGNAL(valueChanged(int)), this, SLOT(onDialChanged(int)));
            return dial;
        }
        return GCF::GuiComponent::loadWidget(name, info);
    }
};

#endif // DIALCOMPONENT_H
