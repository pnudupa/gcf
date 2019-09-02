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

/**
\class GCF::ObjectList ObjectList.h <GCF3/ObjectList>
\brief This class manages a list of live \c QObject pointers.
\ingroup gcf_core


This class can be used as a replacement for \c QList<QObject*>, when you want
only valid pointers to \c QObject to be stored in the list. This class automatically
notifies and removes invalid \c QObject pointers, as and when the objects that the
pointers point to are deleted elsewhere in the application.

Three types of events can happen on \ref GCF::ObjectList
\li A \c QObject pointer got added to the list
\li A \c QObject pointer got removed from the list
\li A \c QObject, whose pointer is in the list, got deleted

You can listen to these events by implementing \ref GCF::ObjectListEventListener
and setting pointer to an instance of that using the \ref setEventListener() function.

If you want to have several listeners receive event notifications on an object list,
then you can install a \ref GCF::ObjectListEventBroadcaster on the object list and
register all event listeners with the broadcaster.

Alternatively, you can install a \ref GCF::ObjectListWatcher as event listener on the
object list and connect to event-signals in the watcher.

Instances of this class are shallow-copyable. This means a reference counter is
used to manage copies of this class. While modifications trigger a deep-copy+modify,
access functions work on a shared data.
*/

#include "ObjectList.h"
#include "ObjectList_p.h"
#include <QAtomicInt>

namespace GCF
{
    struct ObjectListData
    {
        ObjectListData() : refCount(1) { }

        void ref() { refCount.ref(); }
        void deref() { if(!refCount.deref()) delete this; }
        bool needsDetach() const {
#if QT_VERSION >= 0x050000
            return refCount.load() > 1;
#else
            return ((int)refCount) > 1;
#endif
        }

        GCF::QObjectListWatcher watcher;

    private:
        QAtomicInt refCount;
#ifdef Q_OS_MAC
        char unused[4] __attribute__((unused));
#endif
    };
}

/**
Default constructor.
*/
GCF::ObjectList::ObjectList()
{
    d = new ObjectListData;
}

/**
Constructs an object list with \c obj and adds the \c obj to list.

\param obj \c QObject pointer that needs to be added to this list
*/
GCF::ObjectList::ObjectList(QObject *obj)
{
    d = new ObjectListData;
    this->add(obj);
}

/**
Constructs an object list and adds \c QObject pointers in \c objects to it.

\param objects \c QObjectList that will be added to this list
*/
GCF::ObjectList::ObjectList(const QObjectList &objects)
{
    d = new ObjectListData;
    Q_FOREACH(QObject *obj, objects)
        this->add(obj);
}

/**
Copy constructor.

\param other the other \ref GCF::ObjectList object from which information is copied.
*/
GCF::ObjectList::ObjectList(const ObjectList &other)
{
    d = other.d;
    d->ref();
    if(d->watcher.eventListener)
        this->detach();
}

/**
Destructor.

\note Does not delete objects stored inside the list.
*/
GCF::ObjectList::~ObjectList()
{
    d->deref();
}

/**
Adds  \c QObject pointer to this list.

\param obj  \c QObject pointer that needs to be added to this list

\note the \ref GCF::ObjectListEventListener::objectAdded() method will be
called to notify the addition.
*/
void GCF::ObjectList::add(QObject *obj)
{
    this->detach();
    d->watcher.add(obj);
}

/**
Inserts a pointer to \c QObject at a specific index.

\param obj pointer to \c QObject that will be inserted into the list
\param index index at which the pointer will be inserted.
*/
void GCF::ObjectList::insert(int index, QObject *obj)
{
    this->detach();
    d->watcher.add(obj, index);
}

/**
Removes the \c QObject pointer obj from the list.

\param obj pointer to \c QObject that will be removed from the list.
*/
void GCF::ObjectList::remove(QObject *obj)
{
    this->detach();
    d->watcher.remove(obj);
}

/**
Removes the \c QObject pointer at the specified index from the list.

\param index index from which the \c QObject pointer will be removed.
*/
void GCF::ObjectList::removeAt(int index)
{
    QObject *obj = this->at(index);
    return this->remove(obj);
}

/**
Appends \c QObject pointers inside \ref GCF::ObjectList \c list to the current list.

\param list \ref GCF::ObjectList from which the objects will be added to this.
*/
void GCF::ObjectList::add(const ObjectList &list)
{
    this->detach();
    const QObjectList &olist = list.toList();
    Q_FOREACH(QObject *obj, olist)
        this->add(obj);
}

/**
Inserts \c QObject pointers from other \ref GCF::ObjectList 'list' to current list,
starting from the specified index.

\param list \ref ObjectList from which the objects will be inserted.
*/
void GCF::ObjectList::insert(int index, const QObjectList &list)
{
    this->detach();
    for(int i=0; i<list.count(); i++)
        this->insert(index+i, list.at(i));
}

/**
Removes the \c QObject pointers from the current list which are present in other
\c ObjectList \c list .

\param list list of \c QObject pointers that needs to be removed.
*/
void GCF::ObjectList::remove(const ObjectList &list)
{
    this->detach();
    const QObjectList &olist = list.toList();
    Q_FOREACH(QObject *obj, olist)
        this->remove(obj);
}

/**
Returns the object count of this.

\return object count of this.
*/
int GCF::ObjectList::count() const
{
    return d->watcher.qObjectList.count();
}

/**
Returns the \c QObject pointer at the index.

\param index index of \c QObject pointer which needs to be returned.
\return the \c QObject pointer at index.
*/
QObject *GCF::ObjectList::at(int index) const
{
    return d->watcher.qObjectList.at(index);
}

/**
Returns the \c QObject pointer at the first index of the list.

\return the QObject pointer at first index.
*/
QObject *GCF::ObjectList::first() const
{
    return d->watcher.qObjectList.first();
}

/**
Returns the last \c QObject pointer in this list.

\return the \c QObject pointer which exists at last index.
*/
QObject *GCF::ObjectList::last() const
{
    return d->watcher.qObjectList.last();
}

/**
Returns the index of \c QObject pointer in the list if it exists, otherwise
returns -1.

\param obj \c QObject pointer of which index needs to be returned.
\return index of the \c QObject in the list.
*/
int GCF::ObjectList::indexOf(QObject *obj) const
{
    return d->watcher.qObjectList.indexOf(obj);
}

/**
Returns the existing \c QObject pointers as a \c QObjectList.

\return list of QObjects.
*/
const QObjectList &GCF::ObjectList::toList() const
{
    return d->watcher.qObjectList;
}

/**
Removes all the \c QObject pointers from the current list.
\note Does not delete the \c QObject pointers while removing.
*/
void GCF::ObjectList::removeAll()
{
    this->detach();
    d->watcher.removeAll();
}

/**
Deletes all the \c QObject in the current list.
*/
void GCF::ObjectList::deleteAll()
{
    this->detach();
    d->watcher.deleteAll();
}

/**
Sets an event listener on this. If there was already an event listener set, it
will be replaced.
\param listener event listener that needs to be set on this.
*/
void GCF::ObjectList::setEventListener(GCF::ObjectListEventListener *listener)
{
    this->detach();
    d->watcher.eventListener = listener;
}

/**
Returns the event listener which is currently set.
\return event listener which is currently set.
*/
GCF::ObjectListEventListener *GCF::ObjectList::eventListener() const
{
    return d->watcher.eventListener;
}

/**
Overloaded equal to operator. Shallow copies data from other \ref ObjectList
to this.
\return the reference to the this.
*/
GCF::ObjectList &GCF::ObjectList::operator = (const GCF::ObjectList &other)
{
    if(d == other.d)
        return *this;

    d->deref();
    d = other.d;
    d->ref();

    if(d->watcher.eventListener)
        this->detach();

    return *this;
}

void GCF::ObjectList::detach()
{
    if(d->needsDetach())
    {
        ObjectListData *newd = new ObjectListData;
        newd->watcher.deepCopyFrom( &d->watcher );
        d->deref();
        d = newd;

        // Event listener is not copied (on purpose).
        // We dont want a single event-listener to be listening
        // to events from two separate object-lists, objects
        // especially when they are detached.
    }
}

/**
\fn GCF::ObjectList::operator [] (int index) const

Returns the object pointer at the specified index.
\param index index of \c QObject pointer that needs to be returned.
\return \c QObject pointer at specified index.
*/

/**
\fn GCF::ObjectList::operator += (QObject *obj)

Adds the \c QObject pointer obj to this and returns the reference of this list.
\param obj \c QObject pointer which needs to be added.
\return reference to this.
*/

/**
\fn GCF::ObjectList::operator -= (QObject *obj)

Removes the \c QObject pointer obj from this and returns the reference of this list.
\param obj \c QObject pointer which needs be removed.
\return reference to this.
*/

/**
\fn GCF::ObjectList::operator + (QObject *obj)

Adds the \c QObject pointer and \c QObject pointers in current list to a new \c
ObjectList and returns the new list.
\param obj \c QObject pointer which will be added.
\return new \ref ObjectList which contains \c QObject pointer obj and
existing objects in this.
*/

/**
\fn GCF::ObjectList::operator - (QObject *obj)

Creates and returns a new \ref ObjectList which contains subtracted list of
\c QObject pointers of current list from \c obj.
\param obj \c QObject pointer that will be subtracted from this.
\return new \ref ObjectList which does not contain \c QObject pointer obj and
 contains the QObjects of this.
*/

/**
\fn GCF::ObjectList::operator += (const ObjectList &list)

Adds the \c QObject pointers in the other \ref ObjectList 'list' to this and
returns reference to this list.
\param list \ref ObjectList containing \c QObject pointers which needs be added.
\return reference to current list.
*/

/**
\fn GCF::ObjectList::operator -= (const ObjectList &list)

Removes the \c QObject pointers from current list which are present in \ref ObjectList
\c list and returns reference to this.
\param list \ref ObjectList containing \c QObject pointers which needs be removed.
\return reference to this.
*/

/**
\fn GCF::ObjectList::operator + (const ObjectList &list)

Creates and returns a new \ref ObjectList which will be sum of \c QObject
pointers in \ref ObjectList 'list' and this.
\param list \ref ObjectList containing \c QObject pointers which needs be added.
\return new \ref ObjectList which contains QObjects of both this and \ref
ObjectList 'list'.
*/

/**
\fn GCF::ObjectList::operator - (const ObjectList &list)

Creates and returns a new \ref ObjectList which contains subtracted list of
QObjects of current list from \c list.
\param list \ref ObjectList containing \c QObject pointers which needs to be
subtracted.
\return new \ref ObjectList which contains subtracted list of \c QObject
pointers of current list from \c list.
*/

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::ObjectListEventListener ObjectList.h <GCF3/ObjectList>
\brief ObjectListEventListener listens to GCF::ObjectList's events.
\ingroup gcf_core

When set as event listener of \c GCF::ObjectList, the corresponding virtual
functions of this class will be called when a \c QObject pointer is added,
removed or deleted from the \ref GCF::ObjectList.

To set this as event listener of \ref GCF::ObjectList you need to use the method
\ref GCF::ObjectList::setEventListener(GCF::ObjectListEventListener* listener).
Only one event listener can listen to \ref GCF::ObjectList at a time.

If you want to have several listeners receive event notifications on a
\ref GCF::ObjectList list, then you can install a \ref
GCF::ObjectListEventBroadcaster on the \ref GCF::ObjectList and register all
event listeners with the broadcaster.
*/

/**
\fn GCF::ObjectListEventListener::ObjectListEventListener()

Default constructor.
*/

/**
\fn GCF::ObjectListEventListener::~ObjectListEventListener()

Destructor.
*/

/**
\fn GCF::ObjectListEventListener::listenerType() const

\return the listener type. By default listener type is "GCF::ObjectListEventListener".
*/

/**
\fn GCF::ObjectListEventListener::objectAdded(int index, QObject *obj)

By default there is no implementation. This function will be called
when an object is added to the \ref GCF::ObjectList.
\param index index at which the object had been added to the \ref GCF::ObjectList.
\param obj \c QObject pointer which was added to the \ref GCF::ObjectList.
*/

/**
\fn GCF::ObjectListEventListener::objectRemoved(int index, QObject *obj)

By default there is no implementation. This function will be called
when an object is removed from the \ref GCF::ObjectList.
\param index index from which the object had been removed from \ref GCF::ObjectList.
\param obj QObject pointer which was removed from the \ref GCF::ObjectList.
*/

/**
\fn GCF::ObjectListEventListener::objectDeleted(int index, QObject *obj)

By default there is no implementation. This function will be called
when an object is deleted, which was there in the GCF::ObjectList.
\param index index at which the object pointer the existed in GCF::ObjectList.
\param obj \c QObject pointer which was deleted.
*/

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::ObjectListEventBroadcaster ObjectList.h <GCF3/ObjectList>
\brief ObjectListEventBroadcaster broadcasts added, removed and deleted events
of \ref ObjectList.
\ingroup gcf_core


ObjectListEventBroadcaster broadcasts added, removed and deleted events of
\ref GCF::ObjectList to all the \ref GCF::ObjectListEventListener's added to it.
You can add or remove a \ref GCF::ObjectListEventListener
to this using \ref addListener(ObjectListEventListener *listener),
\ref removeListener(ObjectListEventListener *listener) functions.

This class inherits from GCF::ObjectListEventListener. To sends notifications to
all \ref CF::ObjectListEventListener added to this; you first need to set
this event broadcaster as \ref GCF::ObjectListEventListener of the \ref GCF::ObjectList.
*/

/**
\fn GCF::ObjectListEventBroadcaster::addListener(ObjectListEventListener
*listener)

Adds the \ref GCF::ObjectListEventListener to the broadcast list. From this
onwards the added event listener will also get notifications for events on
\ref GCF::ObjectList.
\param listener \ref GCF::ObjectListEventListener that needs to be added
to the broadcast list.
*/

/**
\fn GCF::ObjectListEventBroadcaster::removeListener(ObjectListEventListener
*listener)

Removes the \ref GCF::ObjectListEventListener from the broadcast list.
\param listener \ref GCF::ObjectListEventListener that needs to be removed
 to the broadcast list.
*/

/**
\fn GCF::ObjectListEventBroadcaster::removeAllListeners()

Removes all \ref GCF::ObjectListEventListener from the broadcast list.
*/

/**
\fn GCF::ObjectListEventBroadcaster::listenerAt(int index) const

Returns the \ref GCF::ObjectListEventListener at the specified index.
\param index index for  which the \ref GCF::ObjectListEventListener
needs to be returned.
\return \ref ObjectListEventListener at the specified index.
*/

/**
\fn GCF::ObjectListEventBroadcaster::listenerType() const

Returns the listener type of this class. Default listener type is
"GCF::ObjectListEventBroadcaster".
\return listener type string.
*/

///////////////////////////////////////////////////////////////////////////////
/**
\class GCF::ObjectListWatcher ObjectList.h <GCF3/ObjectList>
\brief This watches over ObjectList to emit signals when the
objects are added, removed or deleted from GCF::ObjectList.
\ingroup gcf_core


You can set this to watch over an \ref GCF::ObjectList by using
\ref GCF::ObjectListWatcher::watch(ObjectList &list) method.

Only one watcher can watch over \ref GCF::ObjectList at a time.If there is
already a watcher watching that ObjectList when a new one is set then
it will be replaced by the new one.
*/

#include "Log.h"

/**
\fn GCF::ObjectListWatcher::ObjectListWatcher()

Default constructor.
*/

/**
\fn GCF::ObjectListWatcher::~ObjectListWatcher()

Destructor.
*/

/**
Sets the \ref GCF::ObjectList to watch. If there is already a watcher watching
that \ref GCF::ObjectList then it will be replaced with this.
*/
void GCF::ObjectListWatcher::watch(ObjectList &list)
{
    if(list.eventListener())
        GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
            "Installing a GCF::ObjectListWatcher on a GCF::ObjectList that is already being watched "
            "by a listener of type " + list.eventListener()->listenerType());
    list.setEventListener(this);
}

/**
\fn GCF::ObjectListWatcher::objectAdded(int index, QObject *obj);

This signal will be  emitted when a \c QObject is added to the
\ref GCF::ObjectList.
\param index index at which the object was inserted.
\param obj \c QObject added.
*/

/**
\fn GCF::ObjectListWatcher::objectRemoved(int index, QObject *obj);
This signal will be emitted when a \c QObject is removed from the
\ref GCF::ObjectList.
\param index index from which the \c QObject was removed.
\param obj \c QObject removed.
*/

/**
\fn GCF::ObjectListWatcher::objectDeleted(int index, QObject *obj);
This signal will be emitted when a QObject is deleted from the
\ref GCF::ObjectList.
\param index index of \c QObject in \ref GCF::ObjectList which got deleted.
\param obj \c QObject deleted.
*/

