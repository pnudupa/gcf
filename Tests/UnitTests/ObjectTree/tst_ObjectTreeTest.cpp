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
#include <QtXml>

#include "Object.h"
#include <GCF3/Version>
#include <GCF3/ObjectTree>

static int ObjectTreeNodeCount = 0;
class ObjectTreeNode2 : public GCF::ObjectTreeNode
{
public:
    ObjectTreeNode2(ObjectTreeNode *parent,
                   const QString &name,
                   QObject *object,
                   const QVariantMap &info=QVariantMap())
        : GCF::ObjectTreeNode(parent, name, object, info) {
        ++::ObjectTreeNodeCount;
    }
    ~ObjectTreeNode2() {
        --::ObjectTreeNodeCount;
    }
};

class ObjectTreeTest : public QObject
{
    Q_OBJECT
    
public:
    ObjectTreeTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testConstructor();
    void testTreeBuild();
    void testPathResolution();
    void testInfo();
    void testSubTreeDeletion();
    void testObjectTreeSignals();
    void testFind();
    void testSimilarPathNames();
    void testNode();
    void testUniqueNames();
    void testSetParent1();
    void testSetParent2();
    void testSetParent3();

private:
    void loadTree(GCF::ObjectTree *tree, const QString &fName=QString(":/ObjectTrees/Tree1.xml"));
    void dumpTreeNode(GCF::ObjectTreeNode *node, int indent=0);
    void loadElement(GCF::ObjectTreeNode *parent, const QDomElement &nodeE, const QString &path);
};

ObjectTreeTest::ObjectTreeTest()
{
}

void ObjectTreeTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectTreeTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void ObjectTreeTest::cleanup()
{
    QObjectList objects = Object::List();
    qDeleteAll(objects);
    QVERIFY(Object::Count() == 0);
}

void ObjectTreeTest::testConstructor()
{
    GCF::ObjectTree tree;

    QVERIFY(tree.rootNode() != 0);
    QVERIFY(tree.rootNode()->name() == "Application");
    QVERIFY(tree.rootNode()->path() == "Application");
    QVERIFY(tree.rootNode()->object() != 0);
    QVERIFY(tree.rootNode()->children().count() == 0);
}

void ObjectTreeTest::testTreeBuild()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    QVERIFY(tree.rootNode()->children().count() == 1);

    GCF::ObjectTreeNode *eatablesNode = tree.rootNode()->children().at(0);
    QVERIFY(eatablesNode->children().count() == 4);

    const int counts[4] = {5, 5, 3, 2};
    for(int i=0; i<4; i++)
    {
        GCF::ObjectTreeNode *node = eatablesNode->children().at(i);
        QVERIFY(node->children().count() == counts[i]);
    }

    const int dishesCount[2] = {3, 5};
    GCF::ObjectTreeNode *dishesNode = eatablesNode->children().last();
    for(int i=0; i<2; i++)
    {
        GCF::ObjectTreeNode *node = dishesNode->children().at(i);
        QVERIFY(node->children().count() == dishesCount[i]);
    }
}

void ObjectTreeTest::testPathResolution()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    Q_FOREACH(QObject *obj, Object::List())
    {
        QString path = obj->objectName();

        GCF::ObjectTreeNode *node = tree.node(path);
        QVERIFY(node != 0);
        QVERIFY(node->path() == path);

        GCF::ObjectTreeNode *node2 = tree.node(obj);
        QVERIFY(node == node2);

        QVERIFY(tree.object(path) == obj);
    }
}

void ObjectTreeTest::testInfo()
{
    GCF::ObjectTree tree;

    QVERIFY(tree.info().count() == 0);
    tree.writableInfo()["Key"] = QString("Value");
    QVERIFY(tree.info().count() == 1);
    QVERIFY(tree.info().constBegin().key() == "Key");
    QVERIFY(tree.info().constBegin().value() == QVariant(QString("Value")));

    this->loadTree(&tree);
    Q_FOREACH(QObject *obj, Object::List())
    {
        QString path = obj->objectName();

        GCF::ObjectTreeNode *node = tree.node(path);
        node->writableInfo()["Path"] = path;
    }

    Q_FOREACH(QObject *obj, Object::List())
    {
        QString path = obj->objectName();

        GCF::ObjectTreeNode *node = tree.node(path);
        QVERIFY(node->info().value("Path") == QVariant(path));
    }
}

void ObjectTreeTest::testSubTreeDeletion()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    QVERIFY(::ObjectTreeNodeCount == 28);

    GCF::ObjectTreeNode *node = tree.node("Application.Eatables.Dishes");
    delete node;
    QVERIFY(::ObjectTreeNodeCount == 17);
}

void ObjectTreeTest::testObjectTreeSignals()
{
    GCF::ObjectTree tree;

    QSignalSpy addSpy(&tree, SIGNAL(nodeAdded(GCF::ObjectTreeNode*,GCF::ObjectTreeNode*)));
    QSignalSpy removeSpy(&tree, SIGNAL(nodeRemoved(GCF::ObjectTreeNode*,GCF::ObjectTreeNode*)));
    QSignalSpy deleteSpy(&tree, SIGNAL(nodeObjectDestroyed(GCF::ObjectTreeNode*)));

    this->loadTree(&tree);

    QVERIFY(addSpy.count() == Object::Count());
    QVERIFY(removeSpy.count() == 0);
    QVERIFY(deleteSpy.count() == 0);
    for(int i=0; i<addSpy.count(); i++)
    {
        QList<QVariant> signalArgs = addSpy.at(i);
        QVERIFY(signalArgs.count() == 2);

        GCF::ObjectTreeNode *parent = signalArgs.first().value<GCF::ObjectTreeNode*>();
        GCF::ObjectTreeNode *child = signalArgs.last().value<GCF::ObjectTreeNode*>();

        QVERIFY(parent != child);
        QVERIFY(child->parent() == parent);
        QVERIFY(parent->children().contains(child));
        QVERIFY(child->object());
        QVERIFY(parent->object());
        QVERIFY(!child->name().isEmpty());
        QVERIFY(!parent->name().isEmpty());
        QVERIFY(child->path().startsWith(parent->path()));
    }
    addSpy.clear();

    GCF::ObjectTreeNode *foodGrainsNode = tree.node("Application.Eatables.FoodGrains");
    int objectCount = Object::Count();
    int foodGrainsCount = foodGrainsNode->children().count();

    QList<GCF::ObjectTreeNode*> foodGrainTypeNodes;
    for(int i=foodGrainsNode->children().count()-1; i>=0; i--)
    {
        GCF::ObjectTreeNode *child = foodGrainsNode->children().at(i);
        foodGrainTypeNodes.append(child);
        delete child;
    }

    QVERIFY(addSpy.count() == 0);
    QVERIFY(removeSpy.count() == foodGrainsCount);
    QVERIFY(deleteSpy.count() == 0);
    QVERIFY(Object::Count() == objectCount);
    for(int i=0; i<removeSpy.count(); i++)
    {
        QList<QVariant> signalArgs = removeSpy.at(i);
        QVERIFY(signalArgs.count() == 2);

        GCF::ObjectTreeNode *parent = signalArgs.first().value<GCF::ObjectTreeNode*>();
        GCF::ObjectTreeNode *child = signalArgs.last().value<GCF::ObjectTreeNode*>();

        QVERIFY(parent == foodGrainsNode);
        QVERIFY(child == foodGrainTypeNodes.at(i));
    }
    removeSpy.clear();

    QStringList removedObjectNames;
    for(int i=10; i>=4; i--)
    {
        QObject *object = Object::List().at(i);
        removedObjectNames.append( object->objectName() );
        delete object;
    }

    QVERIFY(deleteSpy.count() == removedObjectNames.count());
    QVERIFY(Object::Count() == objectCount-removedObjectNames.count());
    for(int i=0; i<deleteSpy.count(); i++)
    {
        QList<QVariant> signalArgs = deleteSpy.at(i);
        QVERIFY(signalArgs.count() == 1);

        GCF::ObjectTreeNode *node = signalArgs.first().value<GCF::ObjectTreeNode*>();
        QVERIFY(node->path() == removedObjectNames.at(i));
    }
    deleteSpy.clear();
}

void ObjectTreeTest::testFind()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    QList<GCF::ObjectTreeNode*> nodes;

    // Test template find
    nodes = tree.findObjectNodes<ObjectType1>();
    QVERIFY(nodes.count() == 1);
    QVERIFY(nodes.at(0)->path() == "Application.Eatables");

    nodes = tree.findObjectNodes<ObjectType2>();
    QVERIFY(nodes.count() == 6);

    nodes = tree.findObjectNodes<ObjectType3>();
    QVERIFY(nodes.count() == 19);

    nodes = tree.findObjectNodes<ObjectType4>();
    QVERIFY(nodes.count() == 1);

    nodes = tree.findObjectNodes<ObjectType5>();
    QVERIFY(nodes.count() == 1);

    QVERIFY(tree.findObjectNode<ObjectType4>()->path() == "Application.Eatables.FoodGrains.Dhal");
    QVERIFY(tree.findObjectNode<ObjectType5>()->path() == "Application.Eatables.Dishes.BisiBeleBhath.Tomato");

    // Test non-template find
    nodes = tree.findObjectNodes("ObjectType1");
    QVERIFY(nodes.count() == 1);
    QVERIFY(nodes.at(0)->path() == "Application.Eatables");

    nodes = tree.findObjectNodes("ObjectType2");
    QVERIFY(nodes.count() == 6);

    nodes = tree.findObjectNodes("ObjectType3");
    QVERIFY(nodes.count() == 19);

    nodes = tree.findObjectNodes("ObjectType4");
    QVERIFY(nodes.count() == 1);

    nodes = tree.findObjectNodes("ObjectType5");
    QVERIFY(nodes.count() == 1);

    QVERIFY(tree.findObjectNode("ObjectType4")->path() == "Application.Eatables.FoodGrains.Dhal");
    QVERIFY(tree.findObjectNode("ObjectType5")->path() == "Application.Eatables.Dishes.BisiBeleBhath.Tomato");
}

void ObjectTreeTest::testSimilarPathNames()
{
    /*
     * While writing tst_LoadComponentTest.cpp, I encountered a bug in
     * ObjectTree. This test case is to catch regressions; should it ever
     * surface again!
     *
     * Suppose that there is a path called Application.SimpleComponent1
     * and Application.SimpleComponent10. Searching for SimpleComponent10
     * returned object corresponding to SimpleComponent1.
     *
     * The bug is fixed. And this testcase demonstrates that.
     */

    GCF::ObjectTree tree;

    for(int i=0; i<100; i++)
    {
        Object *obj = new Object(qApp);
        QString name = QString("Object%1").arg(i+1);
        QString path = QString("Application.%1").arg(name);
        new GCF::ObjectTreeNode(tree.rootNode(), name, obj);
        obj->setObjectName(path);
    }

    for(int i=0; i<100; i++)
    {
        QString path = QString("Application.Object%1").arg(i+1);
        QVERIFY(tree.object(path)->objectName() == path);
    }
}

void ObjectTreeTest::testNode()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    GCF::ObjectTreeNode *eatables = tree.node("Application.Eatables");
    QVERIFY(eatables != 0);

    QVERIFY(eatables->node("Fruits") != 0);
    QVERIFY(eatables->node("Fruits")->name() == "Fruits");
    QVERIFY(eatables->node("Fruits")->path() == "Application.Eatables.Fruits");

    QVERIFY(eatables->node("Vegetables") != 0);
    QVERIFY(eatables->node("Vegetables")->name() == "Vegetables");
    QVERIFY(eatables->node("Vegetables")->path() == "Application.Eatables.Vegetables");

    QVERIFY(eatables->node("FoodGrains") != 0);
    QVERIFY(eatables->node("FoodGrains")->name() == "FoodGrains");
    QVERIFY(eatables->node("FoodGrains")->path() == "Application.Eatables.FoodGrains");

    QVERIFY(eatables->node("Dishes") != 0);
    QVERIFY(eatables->node("Dishes")->name() == "Dishes");
    QVERIFY(eatables->node("Dishes")->path() == "Application.Eatables.Dishes");
}

void ObjectTreeTest::testUniqueNames()
{
    GCF::ObjectTree tree;

    for(int i=0; i<10; i++)
    {
        GCF::ObjectTreeNode *child = new GCF::ObjectTreeNode(tree.rootNode(), "Child", this);
        if(i == 0)
            QVERIFY(child->name() == "Child");
        else
            QVERIFY(child->name() == QString("Child%1").arg(i));
    }
}

void ObjectTreeTest::testSetParent1()
{
    GCF::ObjectTree tree;
    this->loadTree(&tree);

    GCF::ObjectTreeNode *bhath = tree.node("Application.Eatables.Dishes.BisiBeleBhath");
    QVERIFY(bhath != 0);

    GCF::ObjectTreeNode *foodGrains = tree.node("Application.Eatables.FoodGrains");
    QVERIFY(foodGrains != 0);

    QVERIFY(bhath->setParent(foodGrains) == false);
}

void ObjectTreeTest::testSetParent2()
{
    GCF::ObjectTree tree;

    GCF::ObjectTreeNode *animals = new GCF::ObjectTreeNode("Animals", new Object);
    GCF::ObjectTreeNode *tiger = new GCF::ObjectTreeNode(animals, "Tiger", new Object);
    GCF::ObjectTreeNode *elephant = new GCF::ObjectTreeNode(animals, "Elephant", new Object);
    GCF::ObjectTreeNode *dog = new GCF::ObjectTreeNode(animals, "Dog", new Object);

    GCF::ObjectTreeNode *flowers = new GCF::ObjectTreeNode("Flowers", new Object);
    GCF::ObjectTreeNode *rose = new GCF::ObjectTreeNode(flowers, "Rose", new Object);
    GCF::ObjectTreeNode *tulip = new GCF::ObjectTreeNode(flowers, "Tulip", new Object);
    GCF::ObjectTreeNode *lotus = new GCF::ObjectTreeNode(flowers, "Lotus", new Object);

    QVERIFY(tree.node("Application.Animals.Tiger") == 0);
    QVERIFY(tree.node("Application.Animals.Elephant") == 0);
    QVERIFY(tree.node("Application.Animals.Dog") == 0);
    QVERIFY(tree.node("Application.Animals") == 0);

    QVERIFY(tree.node("Application.Flowers.Rose") == 0);
    QVERIFY(tree.node("Application.Flowers.Tulip") == 0);
    QVERIFY(tree.node("Application.Flowers.Lotus") == 0);
    QVERIFY(tree.node("Application.Flowers") == 0);

    QVERIFY(animals->node("Tiger") == tiger);
    QVERIFY(animals->node("Elephant") == elephant);
    QVERIFY(animals->node("Dog") == dog);

    QVERIFY(flowers->node("Rose") == rose);
    QVERIFY(flowers->node("Tulip") == tulip);
    QVERIFY(flowers->node("Lotus") == lotus);

    QVERIFY(animals->owningTree() == 0);
    QVERIFY(flowers->owningTree() == 0);
    QVERIFY(tiger->owningTree() == 0);
    QVERIFY(elephant->owningTree() == 0);
    QVERIFY(dog->owningTree() == 0);
    QVERIFY(flowers->owningTree() == 0);
    QVERIFY(rose->owningTree() == 0);
    QVERIFY(tulip->owningTree() == 0);
    QVERIFY(lotus->owningTree() == 0);

    QVERIFY(animals->setParent(tree.rootNode()) == true);
    QVERIFY(flowers->setParent(tree.rootNode()) == true);

    QVERIFY(tree.node("Application.Animals") == animals);
    QVERIFY(tree.node("Application.Animals.Tiger") == tiger);
    QVERIFY(tree.node("Application.Animals.Elephant") == elephant);
    QVERIFY(tree.node("Application.Animals.Dog") == dog);

    QVERIFY(tree.node("Application.Flowers") == flowers);
    QVERIFY(tree.node("Application.Flowers.Rose") == rose);
    QVERIFY(tree.node("Application.Flowers.Tulip") == tulip);
    QVERIFY(tree.node("Application.Flowers.Lotus") == lotus);

    QVERIFY(animals->node("Tiger") == tiger);
    QVERIFY(animals->node("Elephant") == elephant);
    QVERIFY(animals->node("Dog") == dog);

    QVERIFY(flowers->node("Rose") == rose);
    QVERIFY(flowers->node("Tulip") == tulip);
    QVERIFY(flowers->node("Lotus") == lotus);

    QVERIFY(animals->owningTree() == &tree);
    QVERIFY(flowers->owningTree() == &tree);
    QVERIFY(tiger->owningTree() == &tree);
    QVERIFY(elephant->owningTree() == &tree);
    QVERIFY(dog->owningTree() == &tree);
    QVERIFY(flowers->owningTree() == &tree);
    QVERIFY(rose->owningTree() == &tree);
    QVERIFY(tulip->owningTree() == &tree);
    QVERIFY(lotus->owningTree() == &tree);

    QVERIFY(animals->setParent(tree.rootNode()) == false);
    QVERIFY(flowers->setParent(tree.rootNode()) == false);

    delete Object::List().at(6); // the object correspinding to tulip node
    delete Object::List().at(3); // the object correspinding to dog node
    QVERIFY(tulip->object() == 0);
    QVERIFY(dog->object() == 0);
    QVERIFY(tree.object("Application.Animals.Dog") == 0);
    QVERIFY(tree.object("Application.Flowers.Tulip") == 0);
}

void ObjectTreeTest::testSetParent3()
{
    GCF::ObjectTree tree;

    GCF::ObjectTreeNode *life = new GCF::ObjectTreeNode("Life", new Object);
    GCF::ObjectTreeNode *animals = new GCF::ObjectTreeNode("Animals", new Object);
    GCF::ObjectTreeNode *flowers = new GCF::ObjectTreeNode("Flowers", new Object);

    QVERIFY(tree.node("Application.Life.Animals") == 0);
    QVERIFY(tree.node("Application.Life.Flowers") == 0);
    QVERIFY(tree.node("Application.Life") == 0);

    QVERIFY(life->node("Animals") == 0);
    QVERIFY(life->node("Flowers") == 0);

    QVERIFY(animals->setParent(life) == true);
    QVERIFY(flowers->setParent(life) == true);

    QVERIFY(tree.node("Application.Life.Animals") == 0);
    QVERIFY(tree.node("Application.Life.Flowers") == 0);
    QVERIFY(tree.node("Application.Life") == 0);

    QVERIFY(life->node("Animals") == animals);
    QVERIFY(life->node("Flowers") == flowers);

    QVERIFY(life->setParent(tree.rootNode()) == true);

    QVERIFY(tree.node("Application.Life.Animals") == animals);
    QVERIFY(tree.node("Application.Life.Flowers") == flowers);
    QVERIFY(tree.node("Application.Life") == life);

    QVERIFY(life->node("Animals") == animals);
    QVERIFY(life->node("Flowers") == flowers);

    delete Object::List().last();
    QVERIFY(flowers->object() == 0);
    QVERIFY(tree.object("Application.Life.Flowers") == 0);

    delete Object::List().last();
    QVERIFY(animals->object() == 0);
    QVERIFY(tree.object("Application.Life.Animals") == 0);

    delete Object::List().last();
    QVERIFY(life->object() == 0);
    QVERIFY(tree.object("Application.Life") == 0);

    QVERIFY(life->setParent(flowers) == false);
    QVERIFY(flowers->setParent(animals) == false);
    QVERIFY(animals->setParent(flowers) == false);
}

void ObjectTreeTest::loadTree(GCF::ObjectTree *tree, const QString &fName)
{
    QFile file(fName);
    if( !file.open(QFile::ReadOnly) )
        return;

    QDomDocument doc;
    if( !doc.setContent(&file) )
    {
        qDebug() << "Cannot parse XML file " << fName;
        return;
    }

    QDomElement rootE = doc.documentElement();
    this->loadElement(tree->rootNode(), rootE, QString("Application"));
}

void ObjectTreeTest::dumpTreeNode(GCF::ObjectTreeNode *node, int indent)
{
    QString tab = QString(indent, ' ');
    qDebug() << tab << node->name() << node->object();
    for(int i=0; i<node->children().count(); i++)
        this->dumpTreeNode(node->children().at(i), indent+1);
}

void ObjectTreeTest::loadElement(GCF::ObjectTreeNode *parent, const QDomElement &nodeE, const QString &path)
{
    int type = nodeE.attribute("Type", "0").toInt();
    Object *object = 0;
    switch(type)
    {
    case 1: object = new ObjectType1(qApp); break;
    case 2: object = new ObjectType2(qApp); break;
    case 3: object = new ObjectType3(qApp); break;
    case 4: object = new ObjectType4(qApp); break;
    case 5: object = new ObjectType5(qApp); break;
    default: object = new Object(qApp); break;
    }

    QString path2 = path.isEmpty() ? nodeE.tagName() : (path + "." + nodeE.tagName());
    object->setObjectName(path2);

    GCF::ObjectTreeNode *elementNode
            = new ObjectTreeNode2(parent, nodeE.tagName(), object);

    QDomElement childE = nodeE.firstChildElement();
    while(!childE.isNull())
    {
        this->loadElement(elementNode, childE, path2);
        childE = childE.nextSiblingElement();
    }
}

QTEST_APPLESS_MAIN(ObjectTreeTest)

#include "tst_ObjectTreeTest.moc"

