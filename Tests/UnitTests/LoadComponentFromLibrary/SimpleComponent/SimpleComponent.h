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

#ifndef SIMPLECOMPONENT_H
#define SIMPLECOMPONENT_H

#include <GCF3/Component>

struct SimpleComponentData;
class SimpleComponent : public GCF::Component
{
    Q_OBJECT

public:
    SimpleComponent(QObject *parent=0);
    ~SimpleComponent();

    Q_PROPERTY(QString name READ name WRITE setName)
    void setName(const QString &name);
    QString name() const;

    Q_INVOKABLE QVariantMap propertiesBeforeInitialize() const;

protected:
    void initializeEvent(GCF::InitializeEvent *e);

private:
    SimpleComponentData *d;
};

#endif // SIMPLECOMPONENT_H
