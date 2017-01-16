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

#include "GDriveLiteContentModel_p.h"

void ItemInfoFetcher::apiDone(const QVariant &val, const GCF::Result &result)
{
    if(result.isSuccess() == false)
    {
        m_treeNode->writableInfo()["title"] = "ERROR!";
        m_treeNode->writableInfo()["_error"]
                = QString("ERROR: %1. Please right click and explicitly refresh").arg(result.message());
        this->deleteLater();
        return;
    }

    m_treeNode->writableInfo() = val.toMap();

    GCF::GDriveContent::Item item(m_treeNode->info());
    if(item.isFolder())
    {
        this->beginInsertNodes(m_treeNode, 0, 0);

        // Create a dummy child node, which will get deleted and
        // responded to later on.
        QVariantMap info;
        info["title"] = "Fetching contents...";
        info["_listing"] = false;
        info["_folder"] = true;

        QString dummyName = QString("%1_dummy").arg(m_treeNode->name());
        new GCF::ObjectTreeNode(m_treeNode, dummyName, m_model, info);

        this->endInsertNodes();
    }

    this->deleteLater();
}

void FolderLister::apiDone(const QVariant &val, const GCF::Result &result)
{
    if(!result.isSuccess())
    {
        this->deleteLater();
        return;
    }

    if(m_treeNode->children().count() == 1)
    {
        GCF::ObjectTreeNode *childNode = m_treeNode->children().first();
        if(childNode->info().contains("_folder"))
        {
            this->beginRemoveNodes(m_treeNode, 0, 0);
            delete childNode;
            this->endRemoveNodes();
        }
    }

    QVariantList items = val.toList();
    if(items.count())
    {
        int first = m_treeNode->children().count();
        int last = first + items.count()-1;
        this->beginInsertNodes(m_treeNode, first, last);

        Q_FOREACH(QVariant item, items)
        {
            GCF::GDriveContent::Child child(item.toMap());
            if(!child.id().isEmpty())
            {
                QVariantMap info;
                info["title"] = "Fetching item...";
                ItemInfoFetcher *itemInfoFetcher = new ItemInfoFetcher(child.id(), m_gDriveLite, m_model);
                GCF::ObjectTreeNode *childNode = new GCF::ObjectTreeNode(m_treeNode, child.id(), itemInfoFetcher, info);
                itemInfoFetcher->setTreeNode(childNode);
            }
        }

        this->endInsertNodes();

        // Fetch the next set
        if(m_api->nextPageToken().isEmpty() || m_api->nextPageToken() == m_api->pageToken())
        {
            this->deleteLater();
            return;
        }

        m_api->setPageToken( m_api->nextPageToken() );
        if( m_api->execute().isSuccess() == false )
        {
            this->deleteLater();
            return;
        }

        return;
    }

    if(!m_treeNode->children().count())
    {
        int first = m_treeNode->children().count();
        int last = first + 1;
        this->beginInsertNodes(m_treeNode, first, last);

        QVariantMap info;
        info["title"] = "Folder empty";
        info["_fileInfoLoaded"] = true;
        new GCF::ObjectTreeNode(m_treeNode, "null", m_model, info);

        this->endInsertNodes();
    }

    this->deleteLater();
}

void FileLister::apiDone(const QVariant &val, const GCF::Result &result)
{
    if(!result.isSuccess())
    {
        this->deleteLater();
        return;
    }

    if(m_treeNode->children().count() == 1)
    {
        GCF::ObjectTreeNode *childNode = m_treeNode->children().first();
        if(childNode->info().contains("_folder"))
        {
            this->beginRemoveNodes(m_treeNode, 0, 0);
            delete childNode;
            this->endRemoveNodes();
        }
    }

    QVariantList items = val.toList();
    if(items.count())
    {
        int first = m_treeNode->children().count();
        int last = first + items.count()-1;
        QList<GCF::ObjectTreeNode*> folderNodes;

        this->beginInsertNodes(m_treeNode, first, last);

        Q_FOREACH(QVariant val, items)
        {
            GCF::GDriveContent::Item item(val.toMap());
            GCF::ObjectTreeNode *childNode = new GCF::ObjectTreeNode(m_treeNode, item.id(), m_model, item.data());
            if(item.isFolder())
                folderNodes.append(childNode);
        }

        this->endInsertNodes();

        Q_FOREACH(GCF::ObjectTreeNode *folderNode, folderNodes)
        {
            this->beginInsertNodes(folderNode, 0, 0);

            // Create a dummy child node, which will get deleted and
            // responded to later on.
            QVariantMap info;
            info["title"] = "Fetching contents...";
            info["_listing"] = false;
            info["_folder"] = true;

            QString dummyName = QString("%1_dummy").arg(folderNode->name());
            new GCF::ObjectTreeNode(folderNode, dummyName, m_model, info);

            this->endInsertNodes();
        }

        for(int i=first; i<=last; i++)
            this->finishFileLoad( m_treeNode->children().at(i) );

        // Fetch the next set
        if(m_api->nextPageToken().isEmpty() || m_api->nextPageToken() == m_api->pageToken())
        {
            this->deleteLater();
            return;
        }

        m_api->setPageToken( m_api->nextPageToken() );
        if( m_api->execute().isSuccess() == false )
            this->deleteLater();

        return;
    }

    this->deleteLater();
}


