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

#ifndef SIMPLECONTENTCOMPONENT_H
#define SIMPLECONTENTCOMPONENT_H

#include "SimpleComponent.h"
#include "Object.h"
#include <QMap>

class SimpleContentComponent : public SimpleComponent
{
public:
    SimpleContentComponent(QObject *parent=0) : SimpleComponent(parent) { }
    ~SimpleContentComponent() { }

    QString name() const { return "SimpleContentComponent"; }
    const QMap<QString,QObject*>& objectMap() const {
        return m_objectMap;
    }

protected:
    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/SimpleContentComponent/ComponentContent.xml");
        SimpleComponent::contentLoadEvent(e);
    }

    QObject *loadObject(const QString &name, const QVariantMap &) {
        if(m_objectMap.contains(name))
            return m_objectMap.value(name);
        m_objectMap[name] = new Object(this);
        return m_objectMap.value(name);
    }

    bool unloadObject(const QString &name, QObject *object, const QVariantMap &) {
        if(m_objectMap.contains(name)) {
            Q_ASSERT(object == m_objectMap.value(name));
            m_objectMap.remove(name);
            delete object;
            return true;
        }
        return false;
    }

private:
    QMap<QString,QObject*> m_objectMap;
};

#endif // SIMPLECONTENTCOMPONENT_H
