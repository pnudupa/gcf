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

#ifndef OBJECTLIST_P_H
#define OBJECTLIST_P_H

#include <QObject>
#include "ObjectList.h"

namespace GCF
{

class QObjectListWatcher : public QObject
{
    Q_OBJECT

public:
    QObjectListWatcher() : eventListener(0) { }

    QObjectList qObjectList; // the list that is being watched.
    GCF::ObjectListEventListener *eventListener;

    void add(QObject *obj, int index=-1) {
        if( !obj || qObjectList.contains(obj) )
            return;
        if(index < 0 || index > qObjectList.count()) {
            qObjectList.append(obj);
            index = qObjectList.count()-1;
        } else
            qObjectList.insert(index, obj);
        connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onObjectDestroyed(QObject*)));
        if(this->eventListener)
            this->eventListener->objectAdded(index, obj);
    }

    void remove(QObject *obj) {
        int index = obj ? qObjectList.indexOf(obj) : -1;
        if( !obj || index < 0 )
            return;
        if(this->eventListener)
            this->eventListener->objectRemoved(index, obj);
        qObjectList.removeAt(index);
        disconnect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onObjectDestroyed(QObject*)));
    }

    void removeAll() {
        QObjectList list = qObjectList;
        while(list.count()) {
            QObject *obj = list.takeFirst();
            this->remove(obj);
        }
    }

    void deleteAll() {
        QObjectList list = qObjectList;
        while(list.count()) {
            QObject *obj = list.takeFirst();
            delete obj;
        }
    }

    void deepCopyFrom(QObjectListWatcher *other) {
        while(qObjectList.count())
            this->remove(qObjectList.first());
        Q_FOREACH(QObject *obj, other->qObjectList)
            this->add(obj);

        // Event listener is not copied (on purpose).
        // We dont want a single event-listener to be listening
        // to events from two separate object-lists, objects
        // especially when they are detached.
    }

private slots:
    void onObjectDestroyed(QObject *obj) {
        if(!obj) return;
        int index = qObjectList.indexOf(obj);
        if(this->eventListener) {
            this->eventListener->objectRemoved(index, obj);
            this->eventListener->objectDeleted(index, obj);
        }
        qObjectList.removeAt(index);
    }
};

}

#endif // OBJECTLIST_P_H
