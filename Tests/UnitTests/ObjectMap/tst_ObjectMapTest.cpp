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

#include <QString>
#include <QtTest>

#include "Object.h"
#include "ObjectMapListeners.h"

#include <GCF3/ObjectMap>
#include <GCF3/Version>

class ObjectMapTest : public QObject
{
    Q_OBJECT
    
public:
    ObjectMapTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testDefaultConstructor();
    void testInsert();
    void testRemove();
    void testDelete();
    void testRemoveAll();
    void testDeleteAll();
    void testCopyConstructor();
    void testConstructionWithQMap();

    void testSimpleListener();
    void testBroadcastListener();
    void testWatcher();

private:
    template <class T>
    void verify(const GCF::ObjectMap<T>& gMap, const QMap<QObject*,T>& qMap) {
        QVERIFY(gMap.count() == qMap.count());
        QObjectList objects = qMap.keys();
        Q_FOREACH(QObject *obj, objects) {
            QVERIFY( gMap.contains(obj) );
            QVERIFY( gMap.value(obj) == qMap.value(obj) );
        }
    }
};

ObjectMapTest::ObjectMapTest()
{
}

void ObjectMapTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectMapTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectMapTest::cleanup()
{
    QObjectList objects = Object::List();
    qDeleteAll(objects);
    QVERIFY(Object::Count() == 0);
}

void ObjectMapTest::testDefaultConstructor()
{
    GCF::ObjectMap<QString> objectMap;
    QVERIFY(objectMap.count() == 0);
    QVERIFY(objectMap.isEmpty() == true);
}

void ObjectMapTest::testInsert()
{
    QMap<QObject*,QString> qMap;
    GCF::ObjectMap<QString> gMap;

    for(int i=0; i<10; i++)
    {
        Object *object = new Object;
        QString name = QString("Object%1").arg(i+1);
        qMap.insert(object, name);
        gMap.insert(object, name);

        QVERIFY(gMap.value(object) == qMap.value(object));
    }

    this->verify<QString>(gMap, qMap);
}

void ObjectMapTest::testRemove()
{
    GCF::ObjectMap<QString> gMap;

    for(int i=0; i<10; i++)
    {
        Object *object = new Object;
        QString name = QString("Object%1").arg(i+1);
        gMap.insert(object, name);
    }

    // Remove some objects from the map now.
    for(int i=0; i<10; i+=2)
        gMap.remove( Object::List().at(i) );

    QVERIFY(gMap.count() == Object::Count()/2);
    for(int i=0; i<10; i+=2)
        QVERIFY(gMap.contains( Object::List().at(i) ) == false);
    for(int i=1; i<10; i+=2)
        QVERIFY(gMap.contains( Object::List().at(i) ) == true);
}

void ObjectMapTest::testDelete()
{
    GCF::ObjectMap<int> gMap;

    // Insert some objects into the map.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i+1);
    }

    // Delete some objects from the map at random.
    for(int i=9; i>0; i-=2)
    {
        QObject *object = Object::List().at(i);
        QVERIFY( gMap.contains(object) );

        delete object;
        QVERIFY( gMap.contains(object) == false );
    }

    // Ensure that the map-count is updated
    QVERIFY(gMap.count() == Object::Count());

    // Ensure that the remaining objects have their values as an odd-number only.
    Q_FOREACH(QObject *obj, Object::List())
    {
        int val = gMap.value(obj);
        QVERIFY( (val%2) == 1 );
    }
}

void ObjectMapTest::testRemoveAll()
{
    GCF::ObjectMap<int> gMap;

    // Insert some objects into the map.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i+1);
    }

    // Remove all objects from the map
    gMap.removeAll();

    // Ensure that the map is empty, but the objects are intact
    QVERIFY(Object::Count() == 10);
    QVERIFY(gMap.count() == 0);
}

void ObjectMapTest::testDeleteAll()
{
    GCF::ObjectMap<int> gMap;

    // Insert some objects into the map.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i+1);
    }

    // Remove all objects from the map
    gMap.deleteAll();

    // Ensure that both map and the object-list is empty
    QVERIFY(Object::Count() == 0);
    QVERIFY(gMap.count() == 0);
}

void ObjectMapTest::testCopyConstructor()
{
    GCF::ObjectMap<int> gMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i);
    }

    GCF::ObjectMap<int> gMapCopy = gMap;

    // Check whether the copy has been accurate or not
    QVERIFY(gMap.count() == gMapCopy.count());
    QVERIFY(gMap.keys() == gMapCopy.keys());
    QVERIFY(gMap.values() == gMapCopy.values());
    QVERIFY(gMap.toMap() == gMapCopy.toMap());

    // Now insert independently into the maps and ensure that
    // inserting into one doesnt adversely affect the other.
    gMap.insert(new Object, 10);
    gMapCopy.insert(new Object, 11);

    QVERIFY( gMap.contains(Object::List().at(10)) == true );
    QVERIFY( gMapCopy.contains(Object::List().at(11)) == true );
    QVERIFY( gMapCopy.contains(Object::List().at(10)) == false );
    QVERIFY( gMap.contains(Object::List().at(11)) == false );
}

void ObjectMapTest::testConstructionWithQMap()
{
    QMap<QObject*,int> qMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        qMap[object] = i;
    }

    GCF::ObjectMap<int> gMap(qMap);

    // Check whether the copy has been accurate or not
    QVERIFY(gMap.count() == qMap.count());
    QVERIFY(gMap.keys() == qMap.keys());
    QVERIFY(gMap.values() == qMap.values());
    QVERIFY(gMap.toMap() == qMap);

    // Just "QVERIFY(gMap.toMap() == qMap);" would suffice.
    // The other QVERIFY statements are inserted - anyway!
}

void ObjectMapTest::testSimpleListener()
{
    SimpleListener listener;
    GCF::ObjectMap<int> gMap;

    QVERIFY(gMap.eventListener() == 0);

    gMap.setEventListener(&listener);
    listener.setMap(gMap);
    QVERIFY(gMap.eventListener() == &listener);

    // Test if GCF::ObjectMapEventListener::objectInserted() is invoked or not
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i);

        QVERIFY(listener.lastObjectPointer() == object);
        QVERIFY(listener.lastEvent() == SimpleListener::InsertObjectEvent);
        QVERIFY(listener.lastEventValue() == i);
        listener.reset();
    }

    // Test if GCF::ObjectMapEventListener::objectRemoved() is invoked or not
    for(int i=9; i>=0; i-=2)
    {
        QObject *object = Object::List().at(i);
        gMap.remove(object);

        QVERIFY(listener.lastObjectPointer() == object);
        QVERIFY(listener.lastEvent() == SimpleListener::RemoveObjectEvent);
        QVERIFY(listener.lastEventValue() == i);
        listener.reset();
    }

    // Test if GCF::ObjectMapEventListener::objectDeleted() is invoked or not
    QObjectList objects = Object::List();
    for(int i=0; i<objects.count(); i+=2)
    {
        QObject *object = objects.at(i);
        delete object;

        QVERIFY(listener.events().count() == 2);
        QVERIFY(listener.events().at(0).Object == object);
        QVERIFY(listener.events().at(0).Event == SimpleListener::RemoveObjectEvent);
        QVERIFY(listener.events().at(0).Value == i);
        QVERIFY(listener.events().at(1).Object == object);
        QVERIFY(listener.events().at(1).Event == SimpleListener::DeleteObjectEvent);
        QVERIFY(listener.events().at(1).Value == -2);
        listener.reset();
    }

    // Ensure that upon copy - the listener is not copied
    {
        GCF::ObjectMap<int> copy = gMap;
        QVERIFY(copy.eventListener() != gMap.eventListener());

        copy.insert(new Object, 20);
        QVERIFY(copy.eventListener() != gMap.eventListener());

        delete copy.keys().at(0);
        QVERIFY(listener.events().count() == 0);
    }

    // Ensure that upon copy - the listener is not copied
    {
        GCF::ObjectMap<int> copy;
        copy = gMap;
        QVERIFY(copy.eventListener() != gMap.eventListener());

        copy.insert(new Object, 20);
        QVERIFY(copy.eventListener() != gMap.eventListener());

        delete copy.keys().at(0);
        QVERIFY(listener.events().count() == 0);
    }
}

void ObjectMapTest::testBroadcastListener()
{
    GCF::ObjectMapEventBroadcaster broadcaster;

    GCF::ObjectMap<int> gMap;
    gMap.setEventListener(&broadcaster);

    QVector<SimpleListener> listeners(5);
    for(int i=0; i<listeners.count(); i++)
    {
        listeners[i].setMap(gMap);
        broadcaster.addListener( &listeners[i] );
    }

    QVERIFY(gMap.eventListener() == &broadcaster);

    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(object, i);

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];
            QVERIFY(listener.lastObjectPointer() == object);
            QVERIFY(listener.lastEvent() == SimpleListener::InsertObjectEvent);
            QVERIFY(listener.lastEventValue() == i);
            listener.reset();
        }
    }

    for(int i=9; i>=0; i-=2)
    {
        QObject *object = Object::List().at(i);
        gMap.remove(object);

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];
            QVERIFY(listener.lastObjectPointer() == object);
            QVERIFY(listener.lastEvent() == SimpleListener::RemoveObjectEvent);
            QVERIFY(listener.lastEventValue() == i);
            listener.reset();
        }
    }

    QObjectList objects = Object::List();
    for(int i=0; i<objects.count(); i+=2)
    {
        QObject *object = objects.at(i);
        delete object;

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];
            QVERIFY(listener.events().count() == 2);
            QVERIFY(listener.events().at(0).Object == object);
            QVERIFY(listener.events().at(0).Event == SimpleListener::RemoveObjectEvent);
            QVERIFY(listener.events().at(0).Value == i);
            QVERIFY(listener.events().at(1).Object == object);
            QVERIFY(listener.events().at(1).Event == SimpleListener::DeleteObjectEvent);
            QVERIFY(listener.events().at(1).Value == -2);
            listener.reset();
        }
    }
}

void ObjectMapTest::testWatcher()
{
    GCF::ObjectMapWatcher objectMapWatcher;
    GCF::ObjectMap<int> objectMap;
    objectMapWatcher.watch<int>(objectMap);

    // Initialize signal-spies
    QSignalSpy insertSpy(&objectMapWatcher, SIGNAL(objectInserted(QObject*)));
    QSignalSpy removeSpy(&objectMapWatcher, SIGNAL(objectRemoved(QObject*)));
    QSignalSpy deleteSpy(&objectMapWatcher, SIGNAL(objectDeleted(QObject*)));

    // Check if GCF::ObjectMapEventMapener::objectAdded() is invoked
    // properly or not.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        objectMap.insert(object, i);

        QVERIFY(insertSpy.count() == 1);
        QVERIFY(removeSpy.count() == 0);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(insertSpy.last().at(0).value<QObject*>() == object);

        insertSpy.clear();
    }

    // Check if GCF::ObjectMapEventMapener::objectRemoved() is invoked
    // properly or not.
    QObjectList objects = Object::List();
    for(int i=9; i>=0; i-=2)
    {
        QObject *obj = objects.at(i);
        objectMap.remove( obj );

        QVERIFY(insertSpy.count() == 0);
        QVERIFY(removeSpy.count() == 1);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(removeSpy.last().at(0).value<QObject*>() == obj);

        removeSpy.clear();
    }

    // Check whether GCF::ObjectMapEventMapener::objectRemoved() and
    // GCF::ObjectMapEventMapener::objectDeleted() methods are
    // invoked in proper order upon object deletion.
    objects = objectMap.keys();
    Q_FOREACH(QObject *obj, objects)
    {
        delete obj;

        QVERIFY(insertSpy.count() == 0);
        QVERIFY(removeSpy.count() == 1);
        QVERIFY(deleteSpy.count() == 1);

        // QVERIFY(removeSpy.last().at(0).value<QObject*>() == obj);
        // QVERIFY(deleteSpy.last().at(0).value<QObject*>() == obj);

        /**
          Calling removeSpy.last().at(1).value<QObject*>() on Mac
          crashes the test. This is because .value<QObject*>() works
          only if the qobject-contained in it is a live object.

          In this case - the QObject *is not live. Infact it is dead!

          We have to make peace with the test-case that and trust that
          it works.
          */

        removeSpy.clear();
        deleteSpy.clear();
    }
}

QTEST_APPLESS_MAIN(ObjectMapTest)

#include "tst_ObjectMapTest.moc"
