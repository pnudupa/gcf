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

#include "AgentScriptRunner.h"

#include <QtDebug>
#include <QWidget>
#include <QVariant>
#include <QApplication>
#include <QStringList>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QItemSelectionModel>
#include <QListView>
#include <QTreeView>
#include <QTableView>

QWidgetList topLevelWindows()
{
    QWidgetList topLevelWindowList = qApp->topLevelWidgets();
    return topLevelWindowList;
}

QObject *findRootObject(const QString &objectName)
{
    if(objectName.startsWith("Application.") || objectName == "Application")
        return gAppService->objectTree()->object(objectName);

    return nullptr;
}

QObject *findQChild(const QObject *parent, const QString &childObjectName)
{
    // Qt performs a depth-first search in qFindChild() and QObject::findChild()
    // We need to perform a breadth-first search. Hence this custom implementation

    const QObjectList &children = parent->children();
    Q_FOREACH(QObject *child, children)
    {
        if(child->objectName() == childObjectName)
            return child;
    }

    // We are here - only if none of the direct children of parent
    // had its object-name as childObjectName
    Q_FOREACH(QObject *child, children)
    {
        child = ::findQChild(child, childObjectName);
        if(child)
            return child;
    }

    return nullptr;
}

QObjectList findChildrenByClassName(const QObject *parent, const QString &className)
{
    // This function __doesnt__ do a recursive search.
    // It only searches for direct children of parent

    QObjectList retList;

    const QObjectList &children = parent->children();

    const QMetaObject *classMetaObject = nullptr;
    Q_FOREACH(QObject *child, children)
    {
        if(classMetaObject)
        {
            if(child->metaObject() == classMetaObject)
                retList.append(child);
        }
        else if( QString::fromLatin1(child->metaObject()->className()) == className )
        {
            retList.append(child);
            classMetaObject = child->metaObject();
        }
    }

    return retList;
}

QObject *findClassInstance(const QObject *parent, const QString &classNameIndex)
{
    int index = 0;
    QString className;
    if(classNameIndex.contains('['))
    {
        bool indexOk = false;
        index = classNameIndex.section('[', 1, 1).section(']', 0, 0).toInt(&indexOk);
        if(!indexOk) // Format error!!!
            return nullptr;

        className = classNameIndex.section('[', 0, 0);
    }
    else
        className = classNameIndex;

    QObjectList children = findChildrenByClassName(parent, className);
    if(index >= 0 && index < children.count())
        return children.at(index);

    return nullptr;
}

#if QT_VERSION >= 0x050000
QObject *findQmlContextProperty(const QObject *parent, const QString& contextPropName)
{
    const QQuickView *qmlView = qobject_cast<const QQuickView*>(parent);
    const QQmlEngine *qmlEngine = qmlView ? qmlView->engine() : qobject_cast<const QQmlEngine*>(parent);

    // FIXME: #pragma message("write test case for validating engine access in QQuickView")
    if(contextPropName == "engine")
        return const_cast<QQmlEngine*>(qmlEngine);

    if(qmlEngine)
    {
        QQmlContext *qmlContext = qmlEngine->rootContext();
        QVariant contextProp = qmlContext ? qmlContext->contextProperty(contextPropName) : QVariant();

        QMetaType::Type contextPropType = QMetaType::Type(contextProp.type());
        if( contextPropType == QMetaType::QObjectStar )
            return contextProp.value<QObject*>();
    }

    return nullptr;
}
#else
QObject *findQmlContextProperty(const QObject *parent, const QString& contextPropName)
{
    const QDeclarativeView *qmlView = qobject_cast<const QDeclarativeView*>(parent);
    const QDeclarativeEngine *qmlEngine = qmlView ? qmlView->engine() : qobject_cast<const QDeclarativeEngine*>(parent);
    if(qmlEngine)
    {
        QDeclarativeContext *context = qmlEngine->rootContext();
        QVariant contextProp;
        if(context)
            contextProp = context->contextProperty(contextPropName);

        QMetaType::Type contextPropType = (QMetaType::Type)(contextProp.type());
        if( contextPropType == QMetaType::QObjectStar )
            return contextProp.value<QObject*>();
    }

    return 0;
}
#endif

QObject *findScrollAreaObject(const QObject *parent, const QString &objName)
{
    const QAbstractScrollArea *scrollArea = qobject_cast<const QAbstractScrollArea*>(parent);
    if(scrollArea)
    {
        if(scrollArea->viewport() && objName.toLower() == "viewport")
            return scrollArea->viewport();

        if(scrollArea->horizontalScrollBar() && objName.toLower() == "horizontalscrollbar")
            return scrollArea->horizontalScrollBar();

        if(scrollArea->verticalScrollBar() && objName.toLower() == "verticalscrollbar")
            return scrollArea->verticalScrollBar();

        // For other children, findQChild does a good job. And it would have been called before
    }

    return nullptr;
}

QObject *findItemViewObject(const QObject *parent, const QString &objName)
{
    const QAbstractItemView *itemView = qobject_cast<const QAbstractItemView *>(parent);
    if(itemView)
    {
        if(itemView->model() && objName.toLower() == "model")
            return itemView->model();

        if(itemView->itemDelegate() && objName.toLower() == "delegate")
            return itemView->itemDelegate();

        if(itemView->selectionModel() && objName.toLower() == "selectionmodel")
            return itemView->selectionModel();

        if(objName.toLower() == "horizontalheader")
        {
            const QTableView *tableView = qobject_cast<const QTableView *>(parent);
            if(tableView)
                return tableView->horizontalHeader();

            const QTreeView *treeView = qobject_cast<const QTreeView *>(parent);
            if(treeView)
                return treeView->header();
        }

        if(objName.toLower() == "verticalheader")
        {
            const QTableView *tableView = qobject_cast<const QTableView *>(parent);
            if(tableView)
                return tableView->verticalHeader();
        }
    }

    return nullptr;
}

QObject *findGraphicsObject(const QObject *parent, const QString& gObjName)
{
    const QGraphicsView *graphicsView = qobject_cast<const QGraphicsView*>(parent);
    const QGraphicsScene *graphicsScene = graphicsView ? graphicsView->scene() : qobject_cast<const QGraphicsScene*>(parent);

    // FIXME: #pragma message("write test case for validating scene access in QGraphicsView")
    if(gObjName == "scene")
        return const_cast<QGraphicsScene*>(graphicsScene);

    if(graphicsScene)
    {
        QList<QGraphicsItem*> items = graphicsScene->items();

        Q_FOREACH(QGraphicsItem *item, items)
        {
            QGraphicsObject *gObj = item->toGraphicsObject();
            if(!gObj)
                continue;

            if(gObj->objectName() == gObjName)
                return gObj;

            gObj = gObj->findChild<QGraphicsObject*>(gObjName);
            if(gObj)
                return gObj;
        }
    }

    return nullptr;
}

QObject *findChildObject(QObject *parent, const QString &childObjectName)
{
    QObject *childObject = nullptr;
    if(!parent)
        return childObject;

    childObject = ::findQChild(parent, childObjectName);
    if(childObject)
        return childObject;

    childObject = ::findClassInstance(parent, childObjectName);
    if(childObject)
        return childObject;

    childObject = ::findScrollAreaObject(parent, childObjectName);
    if(childObject)
        return childObject;

    childObject = ::findItemViewObject(parent, childObjectName);
    if(childObject)
        return childObject;

    childObject = ::findQmlContextProperty(parent, childObjectName);
    if(childObject)
        return childObject;

    childObject = ::findGraphicsObject(parent, childObjectName);
    if(childObject)
        return childObject;

    return nullptr;
}

QObject *AgentScriptRunner::findObject(const QString &path)
{
    QStringList comps = path.split('/', QString::SkipEmptyParts);

    QObject *object = nullptr;
    if(comps.length() == 0)
        return object;

    QString rootObjectName = comps.takeFirst();
    object = ::findRootObject(rootObjectName);

    if(object)
    {
        // Recursively dig through paths and find an object.
        Q_FOREACH(const QString &comp, comps)
        {
            object = ::findChildObject(object, comp);
            if(!object)
                return nullptr;
        }
    }

    return object;
}

QObject *AgentScriptRunner::findObject(QObject *parent, const QString &name)
{
    return ::findChildObject(parent, name);
}

