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
#include <QFile>

#include "Object.h"

#include <GCF3/Component>
#include <GCF3/Application>

class FindObjectTest : public QObject
{
    Q_OBJECT
    
public:
    FindObjectTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testFindObject();
    void testFindObjects();
};

FindObjectTest::FindObjectTest()
{
}

void FindObjectTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));

    GCF::Component *component = new GCF::Component;
    component->load();

    component->addContentObject("ObjectType1", new ObjectType1(qApp));
    component->addContentObject("ObjectType2", new ObjectType2(qApp));
    component->addContentObject("ObjectType3", new ObjectType3(qApp));
    component->addContentObject("ObjectType4", new ObjectType4(qApp));
    component->addContentObject("ObjectType5", new ObjectType5(qApp));
    component->addContentObject("ObjectType1Again", new ObjectType1(qApp));

    Q_FOREACH(QObject *obj, Object::List())
    {
        GCF::ObjectTreeNode *node = gApp->objectTree()->node(obj);
        obj->setObjectName( node->path() );
    }
}

void FindObjectTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void FindObjectTest::testFindObject()
{
    QObject *obj = 0;
    GCF::ObjectTreeNode *node = 0;

    obj = gFindObject<ObjectType3>();
    QVERIFY(obj->objectName() == "Application.GCF_Component.ObjectType3");

    obj = gFindObject<ObjectType2>(&node);
    QVERIFY(obj->objectName() == "Application.GCF_Component.ObjectType2");
    QVERIFY(node->path() == "Application.GCF_Component.ObjectType2");

    obj = gFindObject<GCF::Application>(&node);
    QVERIFY(obj == gApp);
    QVERIFY(node == gApp->objectTree()->rootNode());

    obj = gFindObject<QFile>(&node);
    QVERIFY(obj == 0);
    QVERIFY(node == 0);
}

void FindObjectTest::testFindObjects()
{
    QList<ObjectType1*> objects;
    QList<QFile*> files;
    QList<GCF::Application*> apps;
    QList<GCF::ObjectTreeNode*> nodes;

    objects = gFindObjects<ObjectType1>(&nodes);

    QVERIFY(objects.count() == 2);
    QVERIFY(nodes.count() == 2);

    QObject *obj1 = gFindObject("Application.GCF_Component.ObjectType1");
    QObject *obj2 = gFindObject("Application.GCF_Component.ObjectType1Again");
    QVERIFY(obj1 == objects.at(1) || obj1 == objects.at(0));
    QVERIFY(obj2 == objects.at(1) || obj2 == objects.at(0));
    QVERIFY(nodes.at(0)->path() == objects.at(0)->objectName());
    QVERIFY(nodes.at(0)->path() == objects.at(0)->objectName());

    files = gFindObjects<QFile>(&nodes);
    QVERIFY(files.count() == 0);
    QVERIFY(nodes.count() == 0);

    apps = gFindObjects<GCF::Application>();
    QVERIFY(apps.count() == 1);
    QVERIFY(apps.at(0) == gApp);
}

int main(int argc, char *argv[])
{
    GCF::Application app(argc, argv);
    FindObjectTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_FindObjectTest.moc"
