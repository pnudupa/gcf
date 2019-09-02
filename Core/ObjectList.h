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

#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "GCFGlobal.h"
#include <QObject>

namespace GCF
{

class ObjectListEventListener;

struct ObjectListData;
class GCF_EXPORT ObjectList
{
public:
    ObjectList();
    explicit ObjectList(QObject *obj);
    explicit ObjectList(const QObjectList &objects);
    ObjectList(const ObjectList &other);
    ~ObjectList();

    void add(QObject *obj);
    void insert(int index, QObject *obj);
    void remove(QObject *obj);
    void removeAt(int index);
    void add(const ObjectList &list);
    void insert(int index, const QObjectList &list);
    void remove(const ObjectList &list);
    int count() const;
    QObject *at(int index) const;
    QObject *first() const;
    QObject *last() const;
    int indexOf(QObject *obj) const;
    const QObjectList &toList() const;
    bool isEmpty() const { return this->count() == 0; }
    bool contains(QObject *obj) const { return this->indexOf(obj) >= 0; }

    void removeAll(); // Simply removes all pointers
    void deleteAll(); // Clears pointers and deletes objects also

    void setEventListener(GCF::ObjectListEventListener *listener);
    GCF::ObjectListEventListener *eventListener() const;

    ObjectList &operator = (const ObjectList &other);
    QObject *operator [] (int index) const { return this->at(index); }

    ObjectList &operator += (QObject *obj) {
        this->add(obj);
        return *this;
    }
    ObjectList &operator -= (QObject *obj) {
        this->remove(obj);
        return *this;
    }
    ObjectList operator + (QObject *obj) {
        ObjectList ret = *this;
        ret += obj;
        return ret;
    }
    ObjectList operator - (QObject *obj) {
        ObjectList ret = *this;
        ret -= obj;
        return ret;
    }

    ObjectList &operator += (const ObjectList &list) {
        this->add(list);
        return *this;
    }
    ObjectList &operator -= (const ObjectList &list) {
        this->remove(list);
        return *this;
    }
    ObjectList operator + (const ObjectList &list) {
        ObjectList ret = *this;
        ret += list;
        return ret;
    }
    ObjectList operator - (const ObjectList &list) {
        ObjectList ret = *this;
        ret -= list;
        return ret;
    }

private:
    void detach();

private:
    ObjectListData *d;
};

GCF_INTERFACE_BEGIN

class ObjectListEventListener
{
public:
    ObjectListEventListener() { }
    virtual ~ObjectListEventListener() { }

    virtual QString listenerType() const {
        return QString("GCF::ObjectListEventListener");
    }
    virtual void objectAdded(int index, QObject *obj) {
        Q_UNUSED(index); Q_UNUSED(obj);
    }
    virtual void objectRemoved(int index, QObject *obj) {
        Q_UNUSED(index); Q_UNUSED(obj);
    }
    virtual void objectDeleted(int index, QObject *obj) {
        Q_UNUSED(index); Q_UNUSED(obj);
    }
};

GCF_INTERFACE_END

#if QT_VERSION >= 0x050000
class GCF_EXPORT ObjectListEventBroadcaster Q_DECL_FINAL : public ObjectListEventListener
#else
class GCF_EXPORT ObjectListEventBroadcaster : public ObjectListEventListener
#endif
{
public:
    void addListener(ObjectListEventListener *listener) {
        if(listener && !m_listenerList.contains(listener))
            m_listenerList.append(listener);
    }
    void removeListener(ObjectListEventListener *listener) {
        if(listener && m_listenerList.contains(listener))
            m_listenerList.removeAll(listener);
    }
    void removeAllListeners() {
        m_listenerList.clear();
    }
    ObjectListEventListener *listenerAt(int index) const {
        if(index < 0 || index >= m_listenerList.count())
            return nullptr;
        return m_listenerList.at(index);
    }

    QString listenerType() const {
        return QString("GCF::ObjectListEventBroadcaster");
    }
    void objectAdded(int index, QObject *obj) {
        Q_FOREACH(ObjectListEventListener *l, m_listenerList)
            l->objectAdded(index, obj);
    }
    void objectRemoved(int index, QObject *obj) {
        Q_FOREACH(ObjectListEventListener *l, m_listenerList)
            l->objectRemoved(index, obj);
    }
    void objectDeleted(int index, QObject *obj) {
        Q_FOREACH(ObjectListEventListener *l, m_listenerList)
            l->objectDeleted(index, obj);
    }

private:
    QList<ObjectListEventListener*> m_listenerList;
};

#if QT_VERSION >= 0x050000
class GCF_EXPORT ObjectListWatcher Q_DECL_FINAL : public QObject, public ObjectListEventListener
#else
class GCF_EXPORT ObjectListWatcher : public QObject, public ObjectListEventListener
#endif
{
    Q_OBJECT

public:
    ObjectListWatcher(QObject *parent=nullptr) : QObject(parent) { }
    ~ObjectListWatcher() { }

    void watch(ObjectList &list);

    QString listenerType() const {
        return QString("GCF::ObjectListWatcher");
    }

signals:
    void objectAdded(int index, QObject *obj);
    void objectRemoved(int index, QObject *obj);
    void objectDeleted(int index, QObject *obj);
};

}

#endif // OBJECTLIST_H
