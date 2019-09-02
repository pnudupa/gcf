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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "GCFGlobal.h"
#include "ObjectTree.h"
#include "AbstractJob.h"

#include <QDateTime>
#include <QStringList>
#include <QCoreApplication>

namespace GCF
{

class Component;

struct ApplicationServicesData;
class GCF_EXPORT ApplicationServices
{
public:
    static ApplicationServices *instance();
    QDateTime launchTimestamp() const;
    ObjectTree *objectTree() const;
    QObject *findObject(const QString &path) const;

    // Component management
    void loadComponent(Component *component);
    void unloadComponent(Component *component);
    QObjectList components() const;
    void activateComponent(Component *component);
    void deactivateComponent(Component *component);
    bool isLoaded(const Component *component) const;
    bool isActive(const Component *component) const;
    void unloadAllComponents();

    // Loading components from shared libraries
    Component *instantiateComponent(const QString &library);
    Component *loadComponent(const QString &library);
    Component *loadComponent(const QString &library, const QList< QPair<QByteArray,QVariant> >& properties);
    QList<Component*> loadComponents(const QStringList &libraries) {
        GCF::LogMessageBranch branch( QString("Loading %1 components").arg(libraries.count()) );
        QList<Component*> retList;
        Q_FOREACH(QString library, libraries)
            retList << this->loadComponent(library);
        return retList;
    }

    QVariantMap argumentsMap() const;
    void processArguments(const QStringList &additionalArgs=QStringList());

    // Method invocation helprs
    GCF::Result invokeMethod(const QString &path, const QString &method, const QVariantList &args, bool secureCall=true) const;
    static GCF::Result invokeMethod(QObject *object, const QString &method, const QVariantList &args, bool secureCall=true);
    static GCF::Result invokeMethod(QObject *object, const QMetaMethod &method, const QVariantList &args, bool secureCall=true);
    static GCF::Result isMethodInvokable(const QMetaMethod &method, QObject *object=nullptr);

    // Global list of jobs in the application
    GCF::JobListModel *jobs() const;

    QString translate(const QString &string) const {
        return QCoreApplication::translate("GCF", string.toLatin1());
    }

protected:
    ApplicationServices();
    virtual ~ApplicationServices();

private:
    ApplicationServicesData *d;
};

class GCF_EXPORT Application : public QCoreApplication,
                               public ApplicationServices
{
    Q_OBJECT

public:
    static Application *instance();
    Application(int &argc, char **argv);
    ~Application();

    int processArgumentsAndExec(const QStringList &additionalArgs=QStringList()) {
        this->processArguments(additionalArgs);
        return this->exec();
    }

private slots:
    void onAppAboutToQuit();
};

}

#ifndef gApp
#define gApp GCF::Application::instance()
#endif

#define gAppService GCF::ApplicationServices::instance()

inline QObject *gFindObject(const QString &path) {
    return gAppService->findObject(path);
}

template <class T>
inline T *gFindObject(const QString &path, GCF::ObjectTreeNode **objectNode=nullptr) {
    GCF::ObjectTreeNode *node = gAppService->objectTree()->node(path);
    if(objectNode)
        *objectNode = node;
    return node ? qobject_cast<T*>(node->object()) : nullptr;
}

template <class T>
inline T *gFindObject(GCF::ObjectTreeNode **objectNode=nullptr) {
    GCF::ObjectTreeNode *node = gAppService->objectTree()->findObjectNode<T>();
    if(objectNode)
        *objectNode = node;
    if(node)
        return qobject_cast<T*>(node->object());
    return nullptr;
}

template <class T>
inline QList<T*> gFindObjects(QList<GCF::ObjectTreeNode*> *objectNodes=nullptr) {
    QList<GCF::ObjectTreeNode*> nodes = gAppService->objectTree()->findObjectNodes<T>();
    QList<T*> retList;
    for(int i=0; i<nodes.count(); i++)
        retList.append( qobject_cast<T*>(nodes.at(i)->object()) );
    if(objectNodes)
        *objectNodes = nodes;
    return retList;
}

#endif // APPLICATION_H
