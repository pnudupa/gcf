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

#ifndef QMLCOMPONENT_H
#define QMLCOMPONENT_H

#include "QuickCommon.h"
#include "../Core/Component.h"

#include <QUrl>
#include <QQmlComponent>

namespace GCF
{

struct QmlComponentData;
class GCF_QUICK_EXPORT QmlComponent : public GCF::Component
{
    Q_OBJECT

public:
    QmlComponent(QObject *parent=nullptr);

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    bool isActive() const;
    Q_SIGNAL void activeChanged(bool val);

    QObject *addQml(const QString &name, const QUrl &url);
    QObject *addQml(const QUrl &url);

    Q_INVOKABLE void addQmlItem(const QString &name, QObject *object) {
        this->addContentObject(name, object);
    }
    Q_INVOKABLE void removeQmlItem(QObject *object) {
        this->removeContentObject(object);
    }
    Q_INVOKABLE void removeQmlItem(const QString &name) {
        this->removeContentObject(name);
    }
    Q_INVOKABLE QObject *qmlItem(const QString &name) const {
        QString path = QString("Application.%1.%2").arg(this->name()).arg(name);
        return gFindObject<QObject>(path);
    }

protected:
    ~QmlComponent();
    void initializeEvent(GCF::InitializeEvent *e);
    void activationEvent(GCF::ActivationEvent *e);
    void deactivationEvent(GCF::DeactivationEvent *e);
    void contentObjectLoadEvent(GCF::ContentObjectLoadEvent *e);
    void contentObjectMergeEvent(GCF::ContentObjectMergeEvent *e);
    void contentObjectUnloadEvent(GCF::ContentObjectUnloadEvent *e);
    void contentObjectUnmergeEvent(GCF::ContentObjectUnmergeEvent *e);
    void activateContentObjectEvent(GCF::ActivateContentObjectEvent *e);
    void deactivateContentObjectEvent(GCF::DeactivateContentObjectEvent *e);

    virtual QObject *loadQmlObject(const QString &name, const QUrl &url, const QVariantMap &info);
    virtual bool unloadQmlObject(const QString &name, QObject *qmlObject, const QVariantMap &info);

private slots:
    void onComponentStatusChanged(QQmlComponent::Status status);

private:
    QmlComponentData *d;
};

}

#endif // QMLCOMPONENT_H
