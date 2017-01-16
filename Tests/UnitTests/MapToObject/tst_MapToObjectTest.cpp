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

#include <GCF3/Version>
#include <GCF3/MapToObject>

#include "Object.h"
#include "MapToObjectListeners.h"

class MapToObjectTest : public QObject
{
    Q_OBJECT

public:
    MapToObjectTest();

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
    void verify(const GCF::MapToObject<T>& gMap, const QMap<T,QObject*>& qMap) {
        QVERIFY(gMap.count() == qMap.count());
        QList<T> keys = qMap.keys();
        Q_FOREACH(T key, keys) {
            QVERIFY( gMap.contains(key) );
            QVERIFY( gMap.value(key) == qMap.value(key) );
        }
    }
};

MapToObjectTest::MapToObjectTest()
{
}

void MapToObjectTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void MapToObjectTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void MapToObjectTest::cleanup()
{
    QObjectList objects = Object::List();
    qDeleteAll(objects);
    QVERIFY(Object::Count() == 0);
}

void MapToObjectTest::testDefaultConstructor()
{
    GCF::MapToObject<QString> objectMap;
    QVERIFY(objectMap.count() == 0);
    QVERIFY(objectMap.isEmpty() == true);
}

void MapToObjectTest::testInsert()
{
    QMap<QString,QObject*> qMap;
    GCF::MapToObject<QString> gMap;

    for(int i=0; i<10; i++)
    {
        Object *object = new Object;
        QString name = QString("Object%1").arg(i+1);
        qMap.insert(name, object);
        gMap.insert(name, object);

        QVERIFY(gMap.value(name) == qMap.value(name));
    }

    this->verify<QString>(gMap, qMap);
}

void MapToObjectTest::testRemove()
{
    GCF::MapToObject<QString> gMap;

    for(int i=0; i<10; i++)
    {
        Object *object = new Object;
        QString name = QString("Object%1").arg(i+1);
        gMap.insert(name, object);
    }

    // Remove some objects from the map now.
    for(int i=0; i<10; i+=2)
        gMap.remove( QString("Object%1").arg(i+1) );

    QVERIFY(gMap.count() == Object::Count()/2);
    for(int i=0; i<10; i+=2)
        QVERIFY(gMap.contains( QString("Object%1").arg(i+1) ) == false);
    for(int i=1; i<10; i+=2)
        QVERIFY(gMap.contains( QString("Object%1").arg(i+1) ) == true);
}

void MapToObjectTest::testDelete()
{
    GCF::MapToObject<int> gMap;

    // Insert some objects into the map.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i+1, object);
    }

    // Delete some objects from the map at random.
    for(int i=9; i>0; i-=2)
    {
        QObject *object = Object::List().at(i);
        QVERIFY( gMap.contains(i+1) );

        delete object;
        QVERIFY( gMap.contains(i+1) == false );
    }

    // Ensure that the map-count is updated
    QVERIFY(gMap.count() == Object::Count());

    // Ensure that the remaining objects have their values as an odd-number only.
    QList<int> keys = gMap.keys();
    Q_FOREACH(int key, keys)
        QVERIFY( (key%2) == 1 );
}

void MapToObjectTest::testRemoveAll()
{
    GCF::MapToObject<int> gMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i+1, object);
    }

    // Remove all objects from the map
    gMap.removeAll();

    // Ensure that the map is empty, but the objects are intact
    QVERIFY(Object::Count() == 10);
    QVERIFY(gMap.count() == 0);
}

void MapToObjectTest::testDeleteAll()
{
    GCF::MapToObject<int> gMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i+1, object);
    }

    // Remove all objects from the map
    gMap.deleteAll();

    // Ensure that both map and the object-list is empty
    QVERIFY(Object::Count() == 0);
    QVERIFY(gMap.count() == 0);
}

void MapToObjectTest::testCopyConstructor()
{
    GCF::MapToObject<int> gMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i, object);
    }

    GCF::MapToObject<int> gMapCopy = gMap;

    // Check whether the copy has been accurate or not
    QVERIFY(gMap.count() == gMapCopy.count());
    QVERIFY(gMap.keys() == gMapCopy.keys());
    QVERIFY(gMap.values() == gMapCopy.values());
    QVERIFY(gMap.toMap() == gMapCopy.toMap());

    // Now insert independently into the maps and ensure that
    // inserting into one doesnt adversely affect the other.
    gMap.insert(10, new Object);
    gMapCopy.insert(11, new Object);

    QVERIFY( gMap.value(10) == Object::List().at(10) );
    QVERIFY( gMapCopy.value(11) == Object::List().at(11) );
    QVERIFY( gMapCopy.value(10) == 0 );
    QVERIFY( gMap.value(11) == 0 );
}

void MapToObjectTest::testConstructionWithQMap()
{
    QMap<int,QObject*> qMap;
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        qMap.insert(i, object);
    }

    GCF::MapToObject<int> gMap(qMap);

    // Check whether the copy has been accurate or not
    QVERIFY(gMap.count() == qMap.count());
    QVERIFY(gMap.keys() == qMap.keys());
    QVERIFY(gMap.values() == qMap.values());
    QVERIFY(gMap.toMap() == qMap);

    // Just "QVERIFY(gMap.toMap() == qMap);" would suffice.
    // The other QVERIFY statements are inserted - anyway!
}

void MapToObjectTest::testSimpleListener()
{
    SimpleListener listener;
    GCF::MapToObject<int> gMap;

    QVERIFY(gMap.eventListener() == 0);

    gMap.setEventListener(&listener);
    listener.setMap(gMap);
    QVERIFY(gMap.eventListener() == &listener);

    // Test if GCF::MapToObjectEventListener::objectInserted() is invoked or not
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i, object);

        QVERIFY(listener.lastObjectPointer() == object);
        QVERIFY(listener.lastEvent() == SimpleListener::InsertObjectEvent);
        QVERIFY(listener.lastEventValue() == i);
        listener.reset();
    }

    // Test if GCF::MapToObjectEventListener::objectRemoved() is invoked or not
    for(int i=9; i>=0; i-=2)
    {
        QObject *object = Object::List().at(i);
        gMap.remove(i);

        QVERIFY(listener.lastObjectPointer() == object);
        QVERIFY(listener.lastEvent() == SimpleListener::RemoveObjectEvent);
        QVERIFY(listener.lastEventValue() == i);
        listener.reset();
    }

    // Test if GCF::MapToObjectEventListener::objectDeleted() is invoked or not
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
        GCF::MapToObject<int> copy = gMap;
        QVERIFY(copy.eventListener() != gMap.eventListener());

        copy.insert(20, new Object);
        QVERIFY(copy.eventListener() != gMap.eventListener());

        delete copy.value(20);
        QVERIFY(listener.events().count() == 0);
    }

    // Ensure that upon copy - the listener is not copied
    {
        GCF::MapToObject<int> copy;
        copy = gMap;
        QVERIFY(copy.eventListener() != gMap.eventListener());

        copy.insert(20, new Object);
        QVERIFY(copy.eventListener() != gMap.eventListener());

        delete copy.value(20);
        QVERIFY(listener.events().count() == 0);
    }
}

void MapToObjectTest::testBroadcastListener()
{
    GCF::MapToObjectEventBroadcaster broadcaster;

    GCF::MapToObject<int> gMap;
    gMap.setEventListener(&broadcaster);
    QVERIFY(gMap.eventListener() == &broadcaster);

    QVector<SimpleListener> listeners(5);
    for(int i=0; i<listeners.count(); i++)
    {
        listeners[i].setMap(gMap);
        broadcaster.addListener( &listeners[i] );
    }

    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        gMap.insert(i, object);

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
        gMap.remove(i);

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

void MapToObjectTest::testWatcher()
{
    GCF::MapToObjectWatcher objectMapWatcher;
    GCF::MapToObject<int> objectMap;
    objectMapWatcher.watch<int>(objectMap);

    // Initialize signal-spies
    QSignalSpy insertSpy(&objectMapWatcher, SIGNAL(objectInserted(QObject*)));
    QSignalSpy removeSpy(&objectMapWatcher, SIGNAL(objectRemoved(QObject*)));
    QSignalSpy deleteSpy(&objectMapWatcher, SIGNAL(objectDeleted(QObject*)));

    // Check if GCF::MapToObjectEventMapener::objectAdded() is invoked
    // properly or not.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        objectMap.insert(i, object);

        QVERIFY(insertSpy.count() == 1);
        QVERIFY(removeSpy.count() == 0);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(insertSpy.last().at(0).value<QObject*>() == object);

        insertSpy.clear();
    }

    // Check if GCF::MapToObjectEventMapener::objectRemoved() is invoked
    // properly or not.
    QObjectList objects = Object::List();
    for(int i=9; i>=0; i-=2)
    {
        QObject *obj = objects.at(i);
        objectMap.remove(i);

        QVERIFY(insertSpy.count() == 0);
        QVERIFY(removeSpy.count() == 1);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(removeSpy.last().at(0).value<QObject*>() == obj);

        removeSpy.clear();
    }

    // Check whether GCF::MapToObjectEventMapener::objectRemoved() and
    // GCF::MapToObjectEventMapener::objectDeleted() methods are
    // invoked in proper order upon object deletion.
    objects = objectMap.values();
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

QTEST_APPLESS_MAIN(MapToObjectTest)

#include "tst_MapToObjectTest.moc"
