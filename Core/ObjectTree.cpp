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

#include "ObjectTree.h"

#include <QtDebug>
#include <QPointer>
#include <QMetaType>
#include <QCoreApplication>

/**
\class GCF::ObjectTree ObjectTree.h <GCF3/ObjectTree>
\brief A class for representing a logical object-tree structure.
\ingroup gcf_core


This class was primarily designed for use with \ref GCF::ApplicationServices to
capture a tree structure of components and their objects
(\sa GCF::ApplicationServies::objectTree())
But it can also be used to represent any tree of nodes (\ref GCF::ObjectTreeNode
instances) where each node represents one single \c QObject.

\image html object-tree.png

\li \ref GCF::ObjectTreeNode represents a single node in the tree. Each node can have
several children, which in turn can have several children each and so on. A
\li \ref GCF::ObjectTree has a single root-node; which can be accessed via the \ref rootNode()
method. The name of root-node is always "Application". The object managed by the root node is
always the \c QCoreApplication instance in the application (or the object tree itself for
non-event-loop applications).

A node is characterized by the following fields
\li \c name - name of the node. [No two nodes can have the same name within the same hierarchy,
although this rule is not enforced]
\li \c path - a dot (.) separated list of names that uniquely identiies a node in the tree
\li \c object - pointer to a \c QObject that the node is managing. The pointer will be NULL
if the object got deleted after the node was constructed, or if no object was specified during
the construction of the node. [No two nodes in the entire tree must point to the same object,
although this rule is not enforced.]
\li \c info - a \c QVariantMap of key=value pairs.

Whenever a node is added to any sub-tree within the object tree the \ref nodeAdded() signal
is emitted. When an object managed by the node is destroyed the \ref nodeObjectDestroyed()
signal is emitted. The \ref nodeRemoved() signal is emitted whenever a node is deleted from
the object tree.

Nodes can be searched in the object-tree by using any of the following methods
\li \ref node(QString) - search for node by path
\li \ref node(QObject*) - search for node by object that it is managing

\sa GCF::ObjectTreeNode
*/

namespace GCF
{

struct ObjectTreeData
{
    ObjectTreeData() : rootNode(nullptr) { }

    ObjectTreeNode *rootNode;
    GCF::ObjectMap<ObjectTreeNode*> nodeMap;
    QVariantMap info;
};

struct ObjectTreeNodeData
{
    ObjectTreeNodeData() : parent(nullptr),
        object(nullptr), tree(nullptr) { }

    ObjectTreeNode *parent;
    QString name;
    QPointer<QObject> object;
    QVariantMap info;
    ObjectTree *tree;
    QList<ObjectTreeNode*> children;
};

void registerNodeType()
{
    // register the meta-type for GCF::ObjectTreeNode*
    static bool nodeMetaTypeRegistered = false;
    if(!nodeMetaTypeRegistered)
    {
        qRegisterMetaType<GCF::ObjectTreeNode*>("GCF::ObjectTreeNode*");
        nodeMetaTypeRegistered = true;
    }
}

}

/**
 * Constructs a object tree
 *
 * \param parent pointer to the parent object of this object-tree instance.
 */
GCF::ObjectTree::ObjectTree(QObject *parent)
    : QObject(parent)
{
    GCF::registerNodeType();

    d = new ObjectTreeData;

    QObject *rootObject = qApp ? qobject_cast<QObject*>(qApp) : qobject_cast<QObject*>(this);
    d->rootNode = new ObjectTreeNode(nullptr, "Application", rootObject);
    d->rootNode->d->tree = this;
    d->nodeMap.insert(d->rootNode->object(), d->rootNode);
    d->nodeMap.setEventListener(this);
}

/**
 * Destructor. All nodes managed by this object tree are automatically
 * deleted. The objects that the nodes refer-to are not deleted.
 */
GCF::ObjectTree::~ObjectTree()
{
    delete d->rootNode;
    delete d;
}

/**
 * \return a const reference to the information table (\c QVariantMap)
 * maintained against this node.
 */
const QVariantMap &GCF::ObjectTree::info() const
{
    return d->info;
}

/**
 * \return a reference to the information table (\c QVariantMap)
 * maintained against this node.
 */
QVariantMap &GCF::ObjectTree::writableInfo()
{
    return d->info;
}

/**
 * \return a pointer the root node of this object tree
 */
GCF::ObjectTreeNode *GCF::ObjectTree::rootNode() const
{
    return d->rootNode;
}

/**
 * \param path path of the node that is being searched
 * \return a pointer to the node at \c path OR null if no such node exists
 */
GCF::ObjectTreeNode *GCF::ObjectTree::node(const QString &path) const
{
    return d->rootNode->node(path);
}

/**
 * \param object pointer to an object that is referenced by any of the nodes in this tree
 * \return a pointer to the node at that references object OR null if no such node exists
 */
GCF::ObjectTreeNode *GCF::ObjectTree::node(QObject *object) const
{
    if(object)
        return d->nodeMap.value(object);

    return nullptr;
}

/**
 * \param path path of the node that is being searched
 * \return pointer to the \c QObject referenced by the node that was searched OR
 * null if no such node exists.
 *
 * Same as \c node(path)->object()
 */
QObject *GCF::ObjectTree::object(const QString &path) const
{
    return d->rootNode->object(path);
}

/**
 * This function offers the capacity to search the object tree for an object
 * of type \c className
 *
 * \param className name of the class or interface whose object we are searching
 * in this object-tree
 *
 * \return pointer to a node that reference an object of type \c className OR null
 * if no such node was found.
 *
 * \note if the object tree has several nodes referencing objects of type \c className,
 * then a pointer to any one of them is returned.
 *
 * \sa findObjectNodes()
 */
GCF::ObjectTreeNode *GCF::ObjectTree::findObjectNode(const QString &className) const
{
    QByteArray className1 = className.toLatin1();
    QMap<QObject*,ObjectTreeNode*>::const_iterator it = this->begin();
    QMap<QObject*,ObjectTreeNode*>::const_iterator end = this->end();
    while( it != end )
    {
        if( it.key()->inherits(className1) )
            return it.value();
        ++it;
    }

    return nullptr;
}

/**
 * \fn GCF::ObjectTreeNode *GCF::ObjectTree<T>::findObjectNode() const
 *
 * This function offers the capacity to search the object tree for an object
 * of type \c T
 *
 * \return pointer to a node that reference an object of type \c T OR null
 * if no such node was found.
 *
 * \note if the object tree has several nodes referencing objects of type \c T,
 * then a pointer to any one of them is returned.
 *
 * \sa findObjectNodes<T>()
 */

/**
 * This function offers the capacity to search the object tree for an objects
 * of type \c className
 *
 * \param className name of the class or interface whose object we are searching
 * in this object-tree
 *
 * \return list of pointers to a nodes that reference objects of type \c className. The
 * returned list will be empty if no such nodes were found.
 *
 * \note no guarantee is made about the order in which the nodes are returned.
 *
 * \sa findObjectNode()
 */
QList<GCF::ObjectTreeNode*> GCF::ObjectTree::findObjectNodes(const QString &className) const
{
    QByteArray className1 = className.toLatin1();
    QList<GCF::ObjectTreeNode*> nodes;

    QMap<QObject*,ObjectTreeNode*>::const_iterator it = this->begin();
    QMap<QObject*,ObjectTreeNode*>::const_iterator end = this->end();
    while( it != end )
    {
        if( it.key()->inherits(className1) )
            nodes.append( it.value() );
        ++it;
    }

    return nodes;
}

/**
 * \fn QList<GCF::ObjectTreeNode*> GCF::ObjectTree<T>::findObjectNodes() const
 *
 * This function offers the capacity to search the object tree for an objects
 * of type \c T
 *
 * \param className name of the class or interface whose object we are searching
 * in this object-tree
 *
 * \return list of pointers to a nodes that reference objects of type \c T. The
 * returned list will be empty if no such nodes were found.
 *
 * \note no guarantee is made about the order in which the nodes are returned.
 *
 * \sa findObjectNode<T>()
 */

/**
 * \internal
 */
void GCF::ObjectTree::objectRemoved(QObject *object)
{
    ObjectTreeNode *node = d->nodeMap.value(object);
    emit nodeObjectDestroyed(node);
    node->resetObjectPointer();
}

/**
 * \internal
 */
void GCF::ObjectTree::mapNode(GCF::ObjectTreeNode *node, QObject *object)
{
    if(object == nullptr)
        qDebug() << "Object is NULL!!!";
    d->nodeMap.setEventListener(nullptr);
    d->nodeMap.insert(object, node);
    emit nodeAdded(node->parent(), node);
    d->nodeMap.setEventListener(this);
}

/**
 * \internal
 */
void GCF::ObjectTree::unmapNode(GCF::ObjectTreeNode *node)
{
    d->nodeMap.setEventListener(nullptr);
    d->nodeMap.remove(node->object());
    emit nodeRemoved(node->parent(), node);
    d->nodeMap.setEventListener(this);
}

/**
 * \internal
 */
QMap<QObject*,GCF::ObjectTreeNode*>::const_iterator GCF::ObjectTree::begin() const
{
    return d->nodeMap.map().begin();
}

/**
 * \internal
 */
QMap<QObject*,GCF::ObjectTreeNode*>::const_iterator GCF::ObjectTree::end() const
{
    return d->nodeMap.map().end();
}

/**
 * \fn void GCF::ObjectTree::nodeAdded(GCF::ObjectTreeNode *parent, GCF::ObjectTreeNode *child)
 *
 * This signal is emitted whenever a node is added in the object-tree.
 *
 * \param parent pointer to the parent node underwhich a new node was added
 * \param child pointer to the child node, that was actually added
 */

/**
 * \fn void GCF::ObjectTree::nodeRemoved(GCF::ObjectTreeNode *parent, GCF::ObjectTreeNode *child)
 *
 * This signal is emitted whenever a node is removed in the object tree. A node is
 * removed only when it is destroyed or deleted.
 *
 * \param parent pointer to the parent node whose child was removed
 * \param child pointer to the child-node that got deleted
 *
 * \note When this signal is emitted, you can call the \ref GCF::ObjectTreeNode::info()
 * and \ref GCF::ObjectTreeNode::object() and \ref GCF::ObjectTreeNode::name() methods
 * on the child. After the signal delivery is complete, the \c child pointer will not be
 * valid.
 */

/**
 * \fn void GCF::ObjectTree::nodeObjectDestroyed(GCF::ObjectTreeNode *node)
 *
 * This signal is emitted whenever an object referenced by the node gets deleted.
 *
 * \param node pointer to the node whose object got deleted.
 */

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::ObjectTreeNode ObjectTree.h <GCF3/ObjectTree>
\brief Represents a node in \ref GCF::ObjectTree
\ingroup gcf_core


This class represents a single node in \ref GCF::ObjectTree.

A node is characterized by the following fields
\li \c name - name of the node. [No two nodes can have the same name within the same hierarchy,
although this rule is not enforced]
\li \c path - a dot (.) separated list of names that uniquely identiies a node in the tree
\li \c object - pointer to a \c QObject that the node is managing. The pointer will be NULL
if the object got deleted after the node was constructed, or if no object was specified during
the construction of the node. [No two nodes in the entire tree must point to the same object,
although this rule is not enforced.]
\li \c info - a \c QVariantMap of key=value pairs.

\sa \ref GCF::ObjectTree
*/

/**
 * Constructor
 * \param parent pointer to the parent node underwhich this new node should be created.
 * \param name name of the node [the name must be unique within a parent]
 * \param object pointer to a \c QObject that this node references [a valid pointer must
 * always be specified]
 * \param info a \c QVariantMap of key=value pairs
 */
GCF::ObjectTreeNode::ObjectTreeNode(GCF::ObjectTreeNode *parent,
                                    const QString &name,
                                    QObject *object,
                                    const QVariantMap &info)
{
    d = new ObjectTreeNodeData;
    d->parent = parent;
    d->name = this->uniqueName(name);
    d->object = object;
    d->info = info;
    if(d->parent)
    {
        d->parent->d->children.append(this);
        d->tree = d->parent->owningTree();
        if(d->tree)
            d->tree->mapNode(this, d->object);
    }
}

/**
 * \internal
 */
GCF::ObjectTreeNode::ObjectTreeNode(const QString &name,
                                    QObject *object,
                                    const QVariantMap &info)
{
    d = new ObjectTreeNodeData;
    d->parent = nullptr;
    d->name = this->uniqueName(name);
    d->object = object;
    d->info = info;
}

/**
 * Destructor deletes all children nodes under this node.
 */
GCF::ObjectTreeNode::~ObjectTreeNode()
{
    d->tree->unmapNode(this);
    if(d->parent)
        d->parent->d->children.removeAll(this);

    QList<ObjectTreeNode*> children = d->children;
    d->children.clear();
    qDeleteAll(children);

    delete d;
}

/**
 * \internal
 */
bool GCF::ObjectTreeNode::setParent(ObjectTreeNode *parentNode)
{
    if(d->parent)
        return false;

    d->parent = parentNode;

    if(d->parent)
    {
        d->parent->d->children.append(this);

        d->tree = d->parent->owningTree();
        this->registerWithTree();
    }

    return true;
}

/**
 * \return name of this node
 */
QString GCF::ObjectTreeNode::name() const
{
    return d->name;
}

/**
 * \return pointer to the \c QObject referenced by this node
 */
QObject *GCF::ObjectTreeNode::object() const
{
    return d->object;
}

/**
 * \return const reference to the \c QVariantMap information table associated
 * with this node
 */
const QVariantMap &GCF::ObjectTreeNode::info() const
{
    return d->info;
}

/**
 * \return reference to the \c QVariantMap information table associated
 * with this node
 */
QVariantMap &GCF::ObjectTreeNode::writableInfo()
{
    return d->info;
}

/**
 * \return pointer to the parent of this node
 */
GCF::ObjectTreeNode *GCF::ObjectTreeNode::parent() const
{
    return d->parent;
}

/**
 * \return list of children nodes
 */
QList<GCF::ObjectTreeNode*> GCF::ObjectTreeNode::children() const
{
    return d->children;
}

/**
 * \return pointer to the object tree that owns this node
 */
GCF::ObjectTree *GCF::ObjectTreeNode::owningTree() const
{
    return d->tree;
}

/**
 * \param path path of the node that is being searched
 * \return a pointer to the node at \c path OR null if no such node exists
 *
 * \note This function is similar to \ref GCF::ObjectTree::node(QString), but it
 * only searches the sub-tree under this node.
 */
GCF::ObjectTreeNode *GCF::ObjectTreeNode::node(const QString &path) const
{
    if( !path.isEmpty() )
    {
        if(path == d->name)
            return const_cast<GCF::ObjectTreeNode*>(this);

        QString path2 = path;
        if(path.section('.', 0, 0) == d->name)
        {
            path2.remove(0, d->name.length()+1);
            if(path2.isEmpty())
                return const_cast<GCF::ObjectTreeNode*>(this);
        }

        Q_FOREACH(ObjectTreeNode *child, d->children)
        {
            ObjectTreeNode *n = child->node(path2);
            if(n)
                return n;
        }

        return nullptr;
    }

    return nullptr;
}

/**
 * \param object pointer to an object that is referenced by any of the nodes in this tree
 * \return a pointer to the node at that references object OR null if no such node exists
 *
 * \note This function is similar to \ref GCF::ObjectTree::node(QObject*), but it
 * only searches the sub-tree under this node.
 */
GCF::ObjectTreeNode *GCF::ObjectTreeNode::node(QObject *object) const
{
    if(!object)
        return nullptr;

    if(d->object == object)
        return const_cast<ObjectTreeNode*>(this);

    Q_FOREACH(ObjectTreeNode *child, d->children)
    {
        ObjectTreeNode *n = child->node(object);
        if(n)
            return n;
    }

    return nullptr;
}

/**
 * \param path path of the node that is being searched
 * \return pointer to the \c QObject referenced by the node that was searched OR
 * null if no such node exists.
 *
 * Same as \c node(path)->object()
 */
QObject *GCF::ObjectTreeNode::object(const QString &path) const
{
    ObjectTreeNode *n = this->node(path);
    if(n)
        return n->object();

    return nullptr;
}

/**
 * \return path of this node
 */
QString GCF::ObjectTreeNode::path() const
{
    if(d->parent)
    {
        QString ret = QString("%1.%2").arg( d->parent->path() ).arg(d->name);
        return ret;
    }

    return d->name;
}

/**
 * \internal
 */
void GCF::ObjectTreeNode::resetObjectPointer()
{
    d->object = nullptr;
}

/**
 * \internal
 */
QString GCF::ObjectTreeNode::uniqueName(const QString &name) const
{
    if(!d->parent) // Root node will have no parent
        return name;

    QString uname = name;
    int counter = 0;

    while(1)
    {
        // Check to see if the name has any duplicates
        bool unique = (d->parent->node(uname) == nullptr);

        // If the name is unique, then we can use this name
        if(unique)
            break;

        // Otherwise append the name with a counter and check
        // if that name is unique.
        uname = QString("%1%2").arg(name).arg(++counter);
    }

    return uname;
}

/**
 * \internal
 */
void GCF::ObjectTreeNode::registerWithTree()
{
    Q_ASSERT(d->parent != nullptr);

    if(d->tree == nullptr)
        d->tree = d->parent->owningTree();

    if(d->tree)
        d->tree->mapNode(this, d->object);

    for(int i=0; i<d->children.count(); i++)
        d->children.at(i)->registerWithTree();
}

