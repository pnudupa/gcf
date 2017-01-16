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

#ifndef OBJECTLISTLISTENERS_H
#define OBJECTLISTLISTENERS_H

#include <GCF3/ObjectList>

struct SimpleListenerEvent
{
    SimpleListenerEvent() : Object(0), Index(-1), Event(-1) { }
    SimpleListenerEvent(QObject *o, int i, int e)
        : Object(o), Index(i), Event(e) { }

    QObject *Object;
    int Index;
    int Event;
};

class SimpleListener : public GCF::ObjectListEventListener
{
    QList<SimpleListenerEvent> m_events;

public:
    enum Event
    {
        UnknownEvent = -1,
        AddObjectEvent,
        RemoveObjectEvent,
        DeleteObjectEvent
    };

    const QList<SimpleListenerEvent>& events() { return m_events; }

    QObject *lastObjectPointer() const { return m_events.last().Object; }
    int lastObjectIndex() const { return m_events.last().Index; }
    Event lastEvent() const { return Event( m_events.last().Event ); }
    void reset() { m_events.clear(); }

    virtual QString listenerType() const {
        return QString("SimpleListener");
    }
    virtual void objectAdded(int index, QObject *obj) {
        m_events.append( SimpleListenerEvent(obj,index,AddObjectEvent) );
    }
    virtual void objectRemoved(int index, QObject *obj) {
        m_events.append( SimpleListenerEvent(obj,index,RemoveObjectEvent) );
    }
    virtual void objectDeleted(int index, QObject *obj) {
        m_events.append( SimpleListenerEvent(obj,index,DeleteObjectEvent) );
    }
};


#endif // OBJECTLISTLISTENERS_H
