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

#include "SimpleComponent.h"
#include <QMetaObject>
#include <QMetaProperty>

struct SimpleComponentData
{
    QString name;
    QVariantMap propertiesBeforeInitialize;
};

SimpleComponent::SimpleComponent(QObject *parent)
    : GCF::Component(parent)
{
    d = new SimpleComponentData;
    d->name = "SimpleComponent";
}

SimpleComponent::~SimpleComponent()
{
    delete d;
}

void SimpleComponent::setName(const QString &name)
{
    if(this->isLoaded())
        return;

    d->name = name;
}

QString SimpleComponent::name() const
{
    return d->name;
}

QVariantMap SimpleComponent::propertiesBeforeInitialize() const
{
    return d->propertiesBeforeInitialize;
}

void SimpleComponent::initializeEvent(GCF::InitializeEvent *e)
{
    if(e->isPreInitialize())
    {
        QVariantMap map;

        for(int i=0; i<this->metaObject()->propertyCount(); i++)
        {
            QMetaProperty prop = this->metaObject()->property(i);
            map[ QString::fromLatin1(prop.name()) ] = prop.read(this);
        }

        Q_FOREACH(QByteArray prop, this->dynamicPropertyNames())
            map[ QString::fromLatin1(prop) ] = this->property(prop);

        d->propertiesBeforeInitialize = map;
    }

    GCF::Component::initializeEvent(e);
}

GCF_EXPORT_COMPONENT(SimpleComponent)

