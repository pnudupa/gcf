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

#ifndef GDRIVELITECONTENTMODEL_P_H
#define GDRIVELITECONTENTMODEL_P_H

#include "GDriveLite.h"
#include "GDriveLiteAPI.h"
#include "GDriveLiteContentModel.h"

#include <GCF3/ObjectTree>
#include <GCF3/MapToObject>

class GDriveLiteContentNodeObject : public QObject
{
public:
    ~GDriveLiteContentNodeObject() { }

    void setTreeNode(GCF::ObjectTreeNode *node) {
        if(m_treeNode)
            return;
        m_treeNode = node;
        if(m_treeNode)
            this->execute();
    }

    GCF::ObjectTreeNode *treeNode() const { return m_treeNode; }

protected:
    GDriveLiteContentNodeObject(GDriveLite *gDriveLite,
                                GDriveLiteContentModel *model)
        : QObject(model), m_gDriveLite(gDriveLite), m_model(model), m_treeNode(0) {  }

    virtual void execute() = 0;

    void beginInsertNodes(GCF::ObjectTreeNode *parent, int first, int last) {
        Q_ASSERT(parent);
        Q_ASSERT(last >= first);
        QModelIndex index = m_model->index(parent);
        m_model->beginInsertRows(index, first, last);
    }
    void endInsertNodes() { m_model->endInsertRows(); }

    void beginRemoveNodes(GCF::ObjectTreeNode *parent, int first, int last) {
        Q_ASSERT(parent);
        Q_ASSERT(last >= first);
        QModelIndex index = m_model->index(parent);
        m_model->beginRemoveRows(index, first, last);
    }
    void endRemoveNodes() { m_model->endRemoveRows(); }

    void finishFileLoad(GCF::ObjectTreeNode *node) {
        QModelIndex index = m_model->index(node, 0);
        QModelIndex index2 = m_model->index(node, m_model->columnCount()-1);
        node->writableInfo()["_fileInfoLoaded"] = true;
        node->writableInfo()["_loading"] = false;
        emit m_model->fileItemLoaded(index);
        emit m_model->dataChanged(index, index2);
    }

    void finishFolderLoad(GCF::ObjectTreeNode *node) {
        QModelIndex index = m_model->index(node);
        node->writableInfo()["_folderContentsLoaded"] = true;
        node->writableInfo()["_loading"] = false;
        emit m_model->folderItemLoaded(index);
    }

    GDriveLite *m_gDriveLite;
    GDriveLiteContentModel *m_model;
    GCF::ObjectTreeNode *m_treeNode;
};

class ItemInfoFetcher : public GDriveLiteContentNodeObject
{
    Q_OBJECT

public:
    ItemInfoFetcher(const QString &id, GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : GDriveLiteContentNodeObject(gDriveLite, model), m_id(id), m_api(0) { }
    ItemInfoFetcher(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : GDriveLiteContentNodeObject(gDriveLite, model) { }
    ~ItemInfoFetcher() { }

    void execute() {
        m_api = new GDriveLiteAPI::Files::GetRequest(m_gDriveLite, this);
        m_api->setFileId(m_id);
        connect(m_api, SIGNAL(done(QVariant,GCF::Result)),
                this, SLOT(apiDone(QVariant,GCF::Result)));
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();
    }

private slots:
    void apiDone(const QVariant &val, const GCF::Result &result);

protected:
    bool event(QEvent *e) {
        if(e->type() == QEvent::DeferredDelete) {
            if(m_treeNode && m_api && !m_api->isActive())
                this->finishFileLoad(m_treeNode);
        }
        return GDriveLiteContentNodeObject::event(e);
    }

private:
    QString m_id;
    GDriveLiteAPI::Files::GetRequest *m_api;
};

class FolderLister : public GDriveLiteContentNodeObject
{
    Q_OBJECT

public:
    FolderLister(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : GDriveLiteContentNodeObject(gDriveLite, model), m_api(0) { }
    FolderLister(const QString &id, GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : GDriveLiteContentNodeObject(gDriveLite, model), m_folderId(id), m_api(0) { }
    ~FolderLister() { }

    void execute() {
        m_api = new GDriveLiteAPI::Children::ListRequest(m_gDriveLite, this);
        m_api->setFolderId(m_folderId);
        m_api->setQuery("trashed = false");
        connect(m_api, SIGNAL(done(QVariant,GCF::Result)),
                this, SLOT(apiDone(QVariant,GCF::Result)));
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();
    }

private slots:
    void apiDone(const QVariant &val, const GCF::Result &result);

protected:
    bool event(QEvent *e) {
        if(e->type() == QEvent::DeferredDelete) {
            if(m_treeNode && m_api && !m_api->isActive())
                this->finishFolderLoad(m_treeNode);
        }
        return GDriveLiteContentNodeObject::event(e);
    }

private:
    QString m_folderId;
    GDriveLiteAPI::Children::ListRequest *m_api;
};

class FileLister : public GDriveLiteContentNodeObject
{
    Q_OBJECT

public:
    ~FileLister() { }

protected:
    FileLister(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : GDriveLiteContentNodeObject(gDriveLite, model), m_api(0) { }

    bool event(QEvent *e) {
        if(e->type() == QEvent::DeferredDelete) {
            if(m_treeNode && m_api && !m_api->isActive())
                this->finishFolderLoad(m_treeNode);
        }
        return GDriveLiteContentNodeObject::event(e);
    }

private slots:
    void apiDone(const QVariant &val, const GCF::Result &result);

protected:
    GDriveLiteAPI::Files::ListRequest *m_api;
};

class SharedWithMeLister : public FileLister
{
    Q_OBJECT

public:
    SharedWithMeLister(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : FileLister(gDriveLite, model) { }
    ~SharedWithMeLister() { }

    void execute() {
        m_api = new GDriveLiteAPI::Files::ListRequest(m_gDriveLite, this);
        m_api->setQuery("sharedWithMe and trashed = false");
        connect(m_api, SIGNAL(done(QVariant,GCF::Result)),
                this, SLOT(apiDone(QVariant,GCF::Result)));
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();
    }
};

class TrashLister : public FileLister
{
    Q_OBJECT

public:
    TrashLister(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : FileLister(gDriveLite, model) { }
    ~TrashLister() { }

    void execute() {
        m_api = new GDriveLiteAPI::Files::ListRequest(m_gDriveLite, this);
        m_api->setQuery("trashed = true");
        connect(m_api, SIGNAL(done(QVariant,GCF::Result)),
                this, SLOT(apiDone(QVariant,GCF::Result)));
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();
    }
};

class StarredLister : public FileLister
{
    Q_OBJECT

public:
    StarredLister(GDriveLite *gDriveLite, GDriveLiteContentModel *model)
        : FileLister(gDriveLite, model) { }
    ~StarredLister() { }

    void execute() {
        m_api = new GDriveLiteAPI::Files::ListRequest(m_gDriveLite, this);
        m_api->setQuery("trashed = false and starred = true");
        connect(m_api, SIGNAL(done(QVariant,GCF::Result)),
                this, SLOT(apiDone(QVariant,GCF::Result)));
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();
    }
};

#endif // GDRIVELITECONTENTMODEL_P_H
