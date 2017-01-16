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

#ifndef GENERICQMLCOMPONENT_H
#define GENERICQMLCOMPONENT_H

#include <GCF3/QmlComponent>
#include <QFileInfo>

struct LoadEvent
{
    LoadEvent() : QmlObject(0) { }

    QString Name;
    QUrl Url;
    QObject *QmlObject;
    QVariantMap Info;
};
typedef LoadEvent UnloadEvent;

class GenericQmlComponent : public GCF::QmlComponent
{
    Q_OBJECT

public:
    GenericQmlComponent(QObject *parent=0)
        : GCF::QmlComponent(parent), LoadEvents(0),
          UnloadEvents(0), MergeEventCount(0),
          UnmergeEventCount(0), ActivateEventCount(0),
          DeactivateEventCount(0) { }
    ~GenericQmlComponent() { }

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    void setContentFile(const QString &file) {
        m_contentFile = file;
        m_name = QFileInfo(m_contentFile).baseName();
    }
    QString contentFile() const { return m_contentFile; }

    QList<LoadEvent> *LoadEvents;
    QList<UnloadEvent> *UnloadEvents;
    int MergeEventCount;
    int UnmergeEventCount;
    int ActivateEventCount;
    int DeactivateEventCount;

    Q_INVOKABLE void setString(const QString &string) {
        this->String = string;
    }
    QString String;

protected:
    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(m_contentFile);
        GCF::QmlComponent::contentLoadEvent(e);
    }

    QObject *loadObject(const QString &name, const QVariantMap &info) {
        Q_UNUSED(info);
        Q_UNUSED(name);
        QObject *obj = new QObject(this);
        return obj;
    }

    QObject *loadQmlObject(const QString &name, const QUrl &url, const QVariantMap &info) {
        LoadEvent evt;
        evt.Name = name;
        evt.Url = url;
        evt.Info = info;
        evt.QmlObject = GCF::QmlComponent::loadQmlObject(name, url, info);
        if(LoadEvents)
            LoadEvents->append(evt);
        return evt.QmlObject;
    }

    bool unloadQmlObject(const QString &name, QObject *qmlObject, const QVariantMap &info) {
        UnloadEvent evt;
        evt.Name = name;
        evt.QmlObject = qmlObject;
        evt.Info = info;
        evt.Url = info.value("url").toUrl();
        if(this->UnloadEvents)
            this->UnloadEvents->append(evt);
        return GCF::QmlComponent::unloadQmlObject(name, qmlObject, info);
    }

    bool activateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        ++ActivateEventCount;
        return GCF::QmlComponent::activateObject(parent, child, parentInfo, childInfo);
    }
    bool deactivateObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        ++DeactivateEventCount;
        return GCF::QmlComponent::deactivateObject(parent, child, parentInfo, childInfo);
    }

    bool mergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        ++MergeEventCount;
        return GCF::QmlComponent::mergeObject(parent, child, parentInfo, childInfo);
    }
    bool unmergeObject(QObject *parent, QObject *child, const QVariantMap &parentInfo, const QVariantMap &childInfo) {
        ++MergeEventCount;
        return GCF::QmlComponent::unmergeObject(parent, child, parentInfo, childInfo);
    }

private:
    QString m_name;
    QString m_contentFile;
};

#endif // GENERICQMLCOMPONENT_H
