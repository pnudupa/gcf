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

#ifndef MAPTOOBJECT_H
#define MAPTOOBJECT_H

#include <QMap>
#include "ObjectList.h"
#include "Log.h"

namespace GCF
{

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"

class MapToObjectEventListener
{
public:
    MapToObjectEventListener() { }
    virtual ~MapToObjectEventListener() { }

    virtual QString listenerType() const {
        return "GCF::MapToObjectEventListener";
    }

    virtual void objectInserted(QObject *object) { Q_UNUSED(object); }
    virtual void objectRemoved(QObject *object) { Q_UNUSED(object); }
    virtual void objectDeleted(QObject *object) { Q_UNUSED(object); }
};

#pragma clang diagnostic pop

template <class T>
class MapToObject : public GCF::ObjectListEventListener
{
public:
    MapToObject() : m_listener(nullptr) {
        m_objectList.setEventListener(this);
    }
    MapToObject(const MapToObject &other)
        : GCF::ObjectListEventListener(), m_listener(nullptr) {
        m_map = other.m_map;
        m_objectList = other.m_objectList;
        m_objectList.setEventListener(this);
    }
    MapToObject(const QMap<T,QObject*>& map) : m_listener(nullptr) {
        QList<T> keys = map.keys();
        Q_FOREACH(T key, keys)
            this->insert(key, map.value(key));
        m_objectList.setEventListener(this);
    }
    ~MapToObject() { }

    void setEventListener(MapToObjectEventListener *listener) {
        m_listener = listener;
    }
    MapToObjectEventListener *eventListener() const {
        return m_listener;
    }

    MapToObject &operator = (const MapToObject &other) {
        if( this == &other )
            return *this;

        m_listener = nullptr;
        m_map = other.m_map;
        m_objectList = other.m_objectList;
        m_objectList.setEventListener(this);
        return *this;
    }

    void insert(const T &value, QObject *object) {
        m_map.insert(value, object);
        m_objectList.add(object);
    }

    void remove(const T &value) {
        QObject *object = m_map.value(value);
        if( m_map.keys(object).count() <= 1 )
            m_objectList.remove(object);
        else
            m_map.remove(value);
    }

    // We should provide the non-const variant of the [] operator.
    //     QObject*& operator[] (const T &value)

    QObject *operator [] (const T &value) const {
        Q_ASSERT(m_map.contains(value));
        return m_map[value];
    }

    int count() const { return m_map.count(); }
    bool isEmpty() const { return m_map.isEmpty(); }
    bool contains(const T &value) const { return m_map.contains(value); }
    QObject *value(const T &key) const { return m_map.value(key, 0); }
    QList<T> keys() const { return m_map.keys(); }
    QObjectList values() const { return m_map.values(); }
    QMap<T,QObject*> toMap() const { return m_map; }
    const QMap<T,QObject*>& map() const { return m_map; }

    void removeAll() {
        m_objectList.removeAll();
        Q_ASSERT(m_map.isEmpty());
    }

    void deleteAll() {
        m_objectList.deleteAll();
        Q_ASSERT(m_map.isEmpty());
    }

private:
    virtual QString listenerType() const {
        return QString("GCF::MapToObject");
    }
    virtual void objectAdded(int, QObject *obj) {
        if(m_listener)
            m_listener->objectInserted(obj);
    }
    virtual void objectRemoved(int, QObject *obj) {
        if(m_listener)
            m_listener->objectRemoved(obj);
        QList<T> keys = m_map.keys(obj);
        Q_FOREACH(T key, keys)
            m_map.remove(key);
    }
    virtual void objectDeleted(int, QObject *obj) {
        if(m_listener)
            m_listener->objectDeleted(obj);
    }

private:
    QMap<T,QObject*> m_map;
    GCF::ObjectList m_objectList;
    MapToObjectEventListener *m_listener;
};

#if QT_VERSION >= 0x050000
class MapToObjectEventBroadcaster Q_DECL_FINAL : public MapToObjectEventListener
#else
class MapToObjectEventBroadcaster : public MapToObjectEventListener
#endif
{
public:
    void addListener(MapToObjectEventListener *listener) {
        if(listener && !m_listenerList.contains(listener))
            m_listenerList.append(listener);
    }
    void removeListener(MapToObjectEventListener *listener) {
        if(listener && m_listenerList.contains(listener))
            m_listenerList.removeAll(listener);
    }
    void removeAllListeners() {
        m_listenerList.clear();
    }
    MapToObjectEventListener *listenerAt(int index) const {
        if(index < 0 || index >= m_listenerList.count())
            return nullptr;
        return m_listenerList.at(index);
    }

    QString listenerType() const {
        return QString("GCF::MapToObjectEventBroadcaster");
    }
    void objectInserted(QObject *obj) {
        Q_FOREACH(MapToObjectEventListener *l, m_listenerList)
            l->objectInserted(obj);
    }
    void objectRemoved(QObject *obj) {
        Q_FOREACH(MapToObjectEventListener *l, m_listenerList)
            l->objectRemoved(obj);
    }
    void objectDeleted(QObject *obj) {
        Q_FOREACH(MapToObjectEventListener *l, m_listenerList)
            l->objectDeleted(obj);
    }

private:
    QList<MapToObjectEventListener*> m_listenerList;
};

#if QT_VERSION >= 0x050000
class GCF_EXPORT MapToObjectWatcher Q_DECL_FINAL : public QObject, public MapToObjectEventListener
#else
class GCF_EXPORT MapToObjectWatcher : public QObject, public MapToObjectEventListener
#endif
{
    Q_OBJECT

public:
    MapToObjectWatcher(QObject *parent=nullptr) : QObject(parent) { }
    ~MapToObjectWatcher() { }

    template <class T>
    void watch(MapToObject<T>& map) {
        if(map.eventListener())
            GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                "Installing a GCF::MapToObjectWatcher on a GCF::MapToObject that is already being watched "
                "by a listener of type " + map.eventListener()->listenerType());
        map.setEventListener(this);
    }

    QString listenerType() const {
        return QString("GCF::ObjectListWatcher");
    }

signals:
    void objectInserted(QObject *obj);
    void objectRemoved(QObject *obj);
    void objectDeleted(QObject *obj);
};

}

#endif // MAPTOOBJECT_H
