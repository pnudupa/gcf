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
#include <QVector>

#include <GCF3/Version>
#include <GCF3/ObjectList>
#include <GCF3/SignalSpy>

#include "Object.h"
#include "ObjectListListeners.h"

class ObjectListTest : public QObject
{
    Q_OBJECT
    
public:
    ObjectListTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testDefaultConstructor();
    void testAdd();
    void testRemove();
    void testDelete();
    void testDeleteAll();
    void testConstructionWithQObjectList();
    void testConstructionWithQObjectStar();
    void testAddList();
    void testRemoveList();
    void testRemoveAll();
    void testContainsAndIndexOf();
    void testInsert();
    void testRemoveAt();

    void testSimpleListener();
    void testBroadcastListener();
    void testWatcher();

private:
    void verifyObjectList(const GCF::ObjectList &list);
};

ObjectListTest::ObjectListTest()
{
}

void ObjectListTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectListTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectListTest::cleanup()
{
    QObjectList objects = Object::List();
    qDeleteAll(objects);
    QVERIFY(Object::Count() == 0);
}

void ObjectListTest::testDefaultConstructor()
{
    GCF::ObjectList objectList;
    QVERIFY(objectList.count() == 0);
    QVERIFY(objectList.isEmpty() == true);
}

void ObjectListTest::testAdd()
{
    GCF::ObjectList objectList;

    // Test GCF::ObjectList::add() function
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
    }

    // Verify sanity of the list first
    this->verifyObjectList(objectList);

    // Adding the same set of objects again shouldnt alter the list
    QObjectList objects = objectList.toList();
    Q_FOREACH(QObject *obj, objects)
        objectList.add(obj);
    QVERIFY(objectList.count() == objects.count());
}

void ObjectListTest::testRemove()
{
    GCF::ObjectList objectList;

    // Add some objects
    QObjectList objects;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
        objects.append(obj);
    }

    // Remove objects at even-index
    for(int i=0; i<10; i+=2)
        objectList.remove( objects.at(i) );

    // Verify that object-list has half of what was added
    QVERIFY(objectList.count() == objects.count()/2);

    // Deleting any of the removed objects must not alter the size of objectList
    int objectListCount = objectList.count();
    for(int i=0; i<10; i+=2)
    {
        delete objects.at(i);
        QVERIFY(objectList.count() == objectListCount);
    }
}

void ObjectListTest::testDelete()
{
    GCF::ObjectList objectList;

    // Test GCF::ObjectList::add() function
    QObjectList objects;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
        objects.append(obj);
    }

    // Delete the 5th object
    delete objects.at(4);
    this->verifyObjectList(objectList);

    // Delete 2 more objects
    delete objects.at(3);
    delete objects.at(7);
    this->verifyObjectList(objectList);
}

void ObjectListTest::testDeleteAll()
{
    GCF::ObjectList objectList;

    // Add some objects
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
    }

    // Verify the given list (duplicate test)
    this->verifyObjectList(objectList);

    // Delete all objects in the list
    objectList.deleteAll();

    // Verify again
    this->verifyObjectList(objectList);
}

void ObjectListTest::testConstructionWithQObjectList()
{
    QObjectList objects;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objects.append(obj);
    }

    GCF::ObjectList objectList(objects);
    this->verifyObjectList(objectList);

    // Test if objects in QObjectList and GCF::ObjectList are in sync
    // with each other.
    QVERIFY(objectList.count() == objects.count());
    for(int i=0; i<objects.count(); i++)
        QVERIFY(objectList.at(i) == objects.at(i));

    // Delete all objects from the QObjectList
    qDeleteAll(objects);

    // Our GCF::ObjectList must now be empty
    QVERIFY(objectList.isEmpty() == true);
}

void ObjectListTest::testConstructionWithQObjectStar()
{
    Object *object = new Object(qApp);

    GCF::ObjectList objectList(object);
    this->verifyObjectList(objectList);

    QVERIFY(objectList.count() == 1);
    QVERIFY(objectList.at(0) == object);

    delete object;
    QVERIFY(objectList.isEmpty() == true);
}

void ObjectListTest::testAddList()
{
    // Create a list of objects in a QObjectList
    QObjectList objects;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objects.append(obj);
    }

    // Construct a list that will be added into another object-list.
    GCF::ObjectList toAddList(objects);

    // Construct an list with just one object and add the toAddList to it
    GCF::ObjectList objectList(new Object(qApp));
    objectList.add(toAddList);
    this->verifyObjectList(objectList);

    QVERIFY(objectList.count() == 1+toAddList.count());
    for(int i=1; i<objectList.count(); i++)
        QVERIFY(objectList.at(i) == toAddList.at(i-1));
}

void ObjectListTest::testRemoveList()
{
    GCF::ObjectList objectList;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
    }

    QObjectList toRemoveObjects = objectList.toList();
    QObject *unremovedObject = toRemoveObjects.takeAt( toRemoveObjects.count()/2 );

    objectList.remove( GCF::ObjectList(toRemoveObjects) );
    QVERIFY(objectList.count() == 1);
    QVERIFY(objectList.at(0) == unremovedObject);
}

void ObjectListTest::testRemoveAll()
{
    GCF::ObjectList objectList;
    for(int i=0; i<10; i++)
    {
        QObject *obj = new Object(qApp);
        obj->setObjectName( QString("QObject%1").arg(i) );
        objectList.add(obj);
    }

    objectList.removeAll();

    QVERIFY(objectList.count() == 0);
    QVERIFY(Object::Count() == 10);
}

void ObjectListTest::testContainsAndIndexOf()
{
    QObjectList qList;
    GCF::ObjectList gList;
    for(int i=0; i<20; i++)
    {
        qList.append( new Object(qApp) );
        gList.add( qList.last() );
    }

    for(int i=0; i<qList.count(); i++)
    {
        QObject *obj = qList.at(i);
        QVERIFY(gList.contains(obj));
        QVERIFY(gList.indexOf(obj) == i);
    }

    qDeleteAll(qList);
    QVERIFY(gList.count() == 0);
}

void ObjectListTest::testInsert()
{
    // Add some items to a list
    QObjectList qList;
    GCF::ObjectList gList;
    for(int i=0; i<20; i++)
    {
        qList.append( new Object(qApp) );
        gList.add( qList.last() );
    }

    // Now insert an object in the middle and check for signals
    GCF::ObjectListWatcher watcher;
    watcher.watch(gList);

    GCF::SignalSpy spy(&watcher, SIGNAL(objectAdded(int,QObject*)));
    qList.insert(5, new Object(qApp));
    gList.insert(5, qList.at(5));
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().first().toInt() == 5);
    QVERIFY(spy.first().last().value<QObject*>() == qList.at(5));

    // Insert an object list and check
    QObjectList qList2;
    for(int i=0; i<6; i++)
    {
        qList2.append(new Object(qApp));
        qList.insert(8+i, qList2.last());
    }

    spy.clear();
    gList.insert(8, qList2);
    QVERIFY(spy.count() == qList2.count());
    for(int i=0; i<spy.count(); i++)
    {
        QVERIFY(spy.at(i).first().toInt() == 8+i);
        QVERIFY(spy.at(i).last().value<QObject*>() == qList2.at(i));
    }

    // Check the complete list
    QVERIFY(qList == gList.toList());
}

void ObjectListTest::testRemoveAt()
{
    // Add some items to a list
    QObjectList qList;
    GCF::ObjectList gList;
    for(int i=0; i<20; i++)
    {
        qList.append( new Object(qApp) );
        gList.add( qList.last() );
    }

    // Now remove an object in the middle and check for signals
    GCF::ObjectListWatcher watcher;
    watcher.watch(gList);
    GCF::SignalSpy spy(&watcher, SIGNAL(objectRemoved(int,QObject*)));

    gList.removeAt(6);
    QVERIFY(spy.count() == 1);
    QVERIFY(spy.first().first().toInt() == 6);
    QVERIFY(spy.first().last().value<QObject*>() == qList.at(6));
}

void ObjectListTest::testSimpleListener()
{
    SimpleListener listener;
    GCF::ObjectList objectList;

    QVERIFY(objectList.eventListener() == 0);

    objectList.setEventListener(&listener);

    // Check if GCF::ObjectListEventListener::objectAdded() is invoked
    // properly or not.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        objectList.add(object);

        QVERIFY(listener.lastObjectIndex() == i);
        QVERIFY(listener.lastObjectPointer() == object);
        listener.reset();
    }

    // Check if GCF::ObjectListEventListener::objectRemoved() is invoked
    // properly or not.
    QObjectList objects = objectList.toList();
    for(int i=9; i>=0; i-=2)
    {
        QObject *obj = objects.at(i);
        objectList.remove( obj );

        QVERIFY(listener.lastObjectIndex() == i);
        QVERIFY(listener.lastObjectPointer() == obj);
        listener.reset();
    }

    // Check whether GCF::ObjectListEventListener::objectRemoved() and
    // GCF::ObjectListEventListener::objectDeleted() methods are
    // invoked in proper order upon object deletion.
    objects = objectList.toList();
    for(int i=objects.count()-1; i>=0; i--)
    {
        QObject *obj = objects.at(i);
        delete obj;

        const QList<SimpleListenerEvent>& events = listener.events();
        QVERIFY(events.count() == 2);
        QVERIFY(events.first().Object == obj);
        QVERIFY(events.first().Index == i);
        QVERIFY(events.first().Event == SimpleListener::RemoveObjectEvent);
        QVERIFY(events.last().Object == obj);
        QVERIFY(events.last().Index == i);
        QVERIFY(events.last().Event == SimpleListener::DeleteObjectEvent);
        listener.reset();
    }

    // Ensure that upon copy - the listener is not copied
    {
        GCF::ObjectList copy = objectList;
        QVERIFY(copy.eventListener() != objectList.eventListener());

        copy += new Object(qApp);
        QVERIFY(copy.eventListener() != objectList.eventListener());

        delete copy.at(0);
        QVERIFY(listener.events().count() == 0);
    }

    // Ensure that upon copy - the listener is not copied
    {
        GCF::ObjectList copy;
        copy = objectList;
        QVERIFY(copy.eventListener() != objectList.eventListener());

        copy += new Object(qApp);
        QVERIFY(copy.eventListener() != objectList.eventListener());

        delete copy.at(0);
        QVERIFY(listener.events().count() == 0);
    }
}

void ObjectListTest::testBroadcastListener()
{
    QVector<SimpleListener> listeners(5);
    GCF::ObjectListEventBroadcaster broadcaster;
    for(int i=0; i<listeners.count(); i++)
        broadcaster.addListener( &listeners[i] );

    GCF::ObjectList objectList;
    objectList.setEventListener(&broadcaster);

    // Check if GCF::ObjectListEventListener::objectAdded() is invoked
    // properly or not.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        objectList.add(object);

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];
            QVERIFY(listener.lastObjectIndex() == i);
            QVERIFY(listener.lastObjectPointer() == object);
            listener.reset();
        }
    }

    // Check if GCF::ObjectListEventListener::objectRemoved() is invoked
    // properly or not.
    QObjectList objects = objectList.toList();
    for(int i=9; i>=0; i-=2)
    {
        QObject *obj = objects.at(i);
        objectList.remove( obj );

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];
            QVERIFY(listener.lastObjectIndex() == i);
            QVERIFY(listener.lastObjectPointer() == obj);
            listener.reset();
        }
    }

    // Check whether GCF::ObjectListEventListener::objectRemoved() and
    // GCF::ObjectListEventListener::objectDeleted() methods are
    // invoked in proper order upon object deletion.
    objects = objectList.toList();
    for(int i=objects.count()-1; i>=0; i--)
    {
        QObject *obj = objects.at(i);
        delete obj;

        for(int l=0; l<listeners.count(); l++)
        {
            SimpleListener &listener = listeners[l];

            const QList<SimpleListenerEvent>& events = listener.events();
            QVERIFY(events.count() == 2);
            QVERIFY(events.first().Object == obj);
            QVERIFY(events.first().Index == i);
            QVERIFY(events.first().Event == SimpleListener::RemoveObjectEvent);
            QVERIFY(events.last().Object == obj);
            QVERIFY(events.last().Index == i);
            QVERIFY(events.last().Event == SimpleListener::DeleteObjectEvent);
            listener.reset();
        }
    }
}

void ObjectListTest::testWatcher()
{
    GCF::ObjectListWatcher objectListWatcher;
    GCF::ObjectList objectList;
    objectListWatcher.watch(objectList);

    // Initialize signal-spies
    QSignalSpy addSpy(&objectListWatcher, SIGNAL(objectAdded(int,QObject*)));
    QSignalSpy removeSpy(&objectListWatcher, SIGNAL(objectRemoved(int,QObject*)));
    QSignalSpy deleteSpy(&objectListWatcher, SIGNAL(objectDeleted(int,QObject*)));

    // Check if GCF::ObjectListEventListener::objectAdded() is invoked
    // properly or not.
    for(int i=0; i<10; i++)
    {
        Object *object = new Object(qApp);
        objectList.add(object);

        QVERIFY(addSpy.count() == 1);
        QVERIFY(removeSpy.count() == 0);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(addSpy.last().at(0).toInt() == i);
        QVERIFY(addSpy.last().at(1).value<QObject*>() == object);

        addSpy.clear();
    }

    // Check if GCF::ObjectListEventListener::objectRemoved() is invoked
    // properly or not.
    QObjectList objects = objectList.toList();
    for(int i=9; i>=0; i-=2)
    {
        QObject *obj = objects.at(i);
        objectList.remove( obj );

        QVERIFY(addSpy.count() == 0);
        QVERIFY(removeSpy.count() == 1);
        QVERIFY(deleteSpy.count() == 0);
        QVERIFY(removeSpy.last().at(0).toInt() == i);
        QVERIFY(removeSpy.last().at(1).value<QObject*>() == obj);

        removeSpy.clear();
    }

    // Check whether GCF::ObjectListEventListener::objectRemoved() and
    // GCF::ObjectListEventListener::objectDeleted() methods are
    // invoked in proper order upon object deletion.
    objects = objectList.toList();
    for(int i=objects.count()-1; i>=0; i--)
    {
        QObject *obj = objects.at(i);
        delete obj;

        QVERIFY(addSpy.count() == 0);
        QVERIFY(removeSpy.count() == 1);
        QVERIFY(deleteSpy.count() == 1);

        QVERIFY(removeSpy.last().at(0).toInt() == i);
        // QVERIFY(removeSpy.last().at(1).value<QObject*>() == obj);
        QVERIFY(deleteSpy.last().at(0).toInt() == i);
        // QVERIFY(deleteSpy.last().at(1).value<QObject*>() == obj);

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

void ObjectListTest::verifyObjectList(const GCF::ObjectList &list)
{
    QVERIFY(list.count() == Object::Count());

    if(list.count() == Object::Count())
        for(int i=0; i<list.count(); i++)
            QVERIFY(Object::List().contains(list.at(i)));
}

QTEST_APPLESS_MAIN(ObjectListTest)

#include "tst_ObjectListTest.moc"
