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

#ifndef APPEVENTLISTENER_H
#define APPEVENTLISTENER_H

#include <QtDebug>
#include <QObject>
#include <QMetaType>

#include <GCF3/Component>

struct EventInfo
{
    EventInfo() : Component(0) { }
    EventInfo(GCF::Component *comp, const QString &name)
        : Component(comp), EventName(name) { }

    GCF::Component *Component;
    QString EventName;
};

#include <QMetaType>
Q_DECLARE_METATYPE(EventInfo)

QDebug operator << (QDebug dbg, const EventInfo &info)
{
    if(info.Component)
        dbg.nospace() << "EventInfo(" << info.Component->name() << ", " << info.EventName << ")";
    else
        dbg.nospace() << "EventInfo(" << "NULL" << ", " << info.EventName << ")";

    return dbg.space();
}

class AppEventListener : public QObject
{
public:
    AppEventListener(QObject *parent=0) : QObject(parent) { }
    ~AppEventListener() { }

    QList<EventInfo> events() const { return m_events; }
    void clear() { m_events.clear(); }

    bool eventFilter(QObject *obj, QEvent *e) {
        if(e->type() < QEvent::User)
            return false;

        GCF::Component *comp = qobject_cast<GCF::Component*>(obj);
        if(!comp)
            return false;

        EventInfo info;
        info.Component = comp;
        info.EventName = GCF::ComponentEvent::eventName(e->type());
        m_events.append(info);

        return false;
    }

private:
    QList<EventInfo> m_events;
};

#endif // APPEVENTLISTENER_H
