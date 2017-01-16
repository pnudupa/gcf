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

#ifndef OBJECTTREE_H
#define OBJECTTREE_H

#include "GCFGlobal.h"
#include "ObjectMap.h"

#include <QObject>

namespace GCF
{

class ObjectTreeNode;

struct ObjectTreeData;
class GCF_EXPORT ObjectTree : public QObject, public ObjectMapEventListener
{
    Q_OBJECT

public:
    ObjectTree(QObject *parent=0);
    ~ObjectTree();

    const QVariantMap &info() const;
    QVariantMap &writableInfo();

    ObjectTreeNode *rootNode() const;
    ObjectTreeNode *node(const QString &path) const;
    ObjectTreeNode *node(QObject *object) const;
    QObject *object(const QString &path) const;

    template <class T>
    ObjectTreeNode *findObjectNode() const {
        QMap<QObject*,ObjectTreeNode*>::const_iterator it = this->begin();
        QMap<QObject*,ObjectTreeNode*>::const_iterator end = this->end();
        while( it != end ) {
            T *ptr = qobject_cast<T*>( it.key() );
            if(ptr)
                return it.value();
            ++it;
        }
        return 0;
    }

    template <class T>
    QList<ObjectTreeNode*> findObjectNodes() const {
        QList<ObjectTreeNode*> retList;
        QMap<QObject*,ObjectTreeNode*>::const_iterator it = this->begin();
        QMap<QObject*,ObjectTreeNode*>::const_iterator end = this->end();
        while( it != end ) {
            T *ptr = qobject_cast<T*>( it.key() );
            if(ptr)
                retList << it.value();
            ++it;
        }
        return retList;
    }

    ObjectTreeNode *findObjectNode(const QString &className) const;
    QList<ObjectTreeNode*> findObjectNodes(const QString &className) const;

signals:
    void nodeAdded(GCF::ObjectTreeNode *parent, GCF::ObjectTreeNode *child);
    void nodeRemoved(GCF::ObjectTreeNode *parent, GCF::ObjectTreeNode *child);
    void nodeObjectDestroyed(GCF::ObjectTreeNode *node);

private:
    void objectRemoved(QObject *object);
    void mapNode(ObjectTreeNode *node, QObject *object);
    void unmapNode(ObjectTreeNode *node);
    QMap<QObject*,ObjectTreeNode*>::const_iterator begin() const;
    QMap<QObject*,ObjectTreeNode*>::const_iterator end() const;

private:
    friend class ObjectTreeNode;
    ObjectTreeData *d;
};

struct ObjectTreeNodeData;
class GCF_EXPORT ObjectTreeNode
{
public:
    ObjectTreeNode(ObjectTreeNode *parent,
                   const QString &name,
                   QObject *object,
                   const QVariantMap &info=QVariantMap());
    ObjectTreeNode(const QString &name,
                   QObject *object,
                   const QVariantMap &info=QVariantMap());
    virtual ~ObjectTreeNode();

    bool setParent(ObjectTreeNode *parentNode); // works only if parent() returns NULL

    QString name() const;
    QObject *object() const;
    const QVariantMap &info() const;
    QVariantMap &writableInfo();

    ObjectTreeNode *parent() const;
    QList<ObjectTreeNode*> children() const;
    ObjectTree *owningTree() const;

    ObjectTreeNode *node(const QString &path) const;
    ObjectTreeNode *node(QObject *object) const;
    QObject *object(const QString &path) const;
    QString path() const;

private:
    void resetObjectPointer();
    QString uniqueName(const QString &name) const;
    void registerWithTree();

private:
    friend class ObjectTree;
    ObjectTreeNodeData *d;
};

}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::ObjectTreeNode*)

#endif // OBJECTTREE_H
