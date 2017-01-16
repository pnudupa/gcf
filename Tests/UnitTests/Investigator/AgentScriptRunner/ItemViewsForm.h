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

#ifndef ITEMVIEWSFORM_H
#define ITEMVIEWSFORM_H

#include <QtDebug>
#include <QListView>
#include <QTreeView>
#include <QTableView>
#include <QScrollBar>
#include <QScrollArea>
#include <QGridLayout>
#include <QHeaderView>
#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
public:
    TableModel(QObject *parent=0) : QAbstractTableModel(parent) { }
    ~TableModel() { }

    int rowCount(const QModelIndex &parent) const {
        if(parent.isValid()) return 0;
        return 1000;
    }
    int columnCount(const QModelIndex &parent) const {
        if(parent.isValid()) return 0;
        return 1000;
    }

    QVariant data(const QModelIndex &index, int role) const {
        if(role == Qt::DisplayRole)
            return (index.row()+1)*(index.column()+1);
        return QVariant();
    }
};

class ItemViewsForm : public QWidget
{
    Q_OBJECT

public:
    ItemViewsForm(QWidget *parent=0) : QWidget(parent) {
        QGridLayout *layout = new QGridLayout(this);

        m_treeView = new QTreeView(this);
        m_listView = new QListView(this);
        m_tableView = new QTableView(this);
        m_treeView->setModel(&m_model);
        m_listView->setModel(&m_model);
        m_tableView->setModel(&m_model);

        m_scrollArea = new QScrollArea(this);
        m_scrollArea->setViewport(new QWidget);
        m_scrollArea->viewport()->setFixedSize(5000, 5000);

        layout->addWidget(m_treeView, 0, 0);
        layout->addWidget(m_listView, 0, 1);
        layout->addWidget(m_tableView, 1, 0);
        layout->addWidget(m_scrollArea, 1, 1);
    }

    Q_INVOKABLE bool checkTreeViewViewport(QObject *obj) { return obj == m_treeView->viewport(); }
    Q_INVOKABLE bool checkTreeViewHorizontalScrollBar(QObject *obj) { return obj == m_treeView->horizontalScrollBar(); }
    Q_INVOKABLE bool checkTreeViewVerticalScrollBar(QObject *obj) { return obj == m_treeView->verticalScrollBar(); }
    Q_INVOKABLE bool checkTreeViewHorizontalHeader(QObject *obj) { return obj == m_treeView->header(); }

    Q_INVOKABLE bool checkTableViewViewport(QObject *obj) { return obj == m_tableView->viewport(); }
    Q_INVOKABLE bool checkTableViewHorizontalScrollBar(QObject *obj) { return obj == m_tableView->horizontalScrollBar(); }
    Q_INVOKABLE bool checkTableViewVerticalScrollBar(QObject *obj) { return obj == m_tableView->verticalScrollBar(); }
    Q_INVOKABLE bool checkTableViewHorizontalHeader(QObject *obj) { return obj == m_tableView->horizontalHeader(); }
    Q_INVOKABLE bool checkTableViewVerticalHeader(QObject *obj) { return obj == m_tableView->verticalHeader(); }

    Q_INVOKABLE bool checkListViewViewport(QObject *obj) { return obj == m_listView->viewport(); }
    Q_INVOKABLE bool checkListViewVerticalScrollBar(QObject *obj) { return obj == m_listView->verticalScrollBar(); }

    Q_INVOKABLE bool checkScrollAreaViewport(QObject *obj) { return obj == m_scrollArea->viewport(); }
    Q_INVOKABLE bool checkScrollAreaHorizontalScrollBar(QObject *obj) { return obj == m_scrollArea->horizontalScrollBar(); }
    Q_INVOKABLE bool checkScrollAreaVerticalScrollBar(QObject *obj) { return obj == m_scrollArea->verticalScrollBar(); }

private:
    QTreeView *m_treeView;
    QTableView *m_tableView;
    QListView *m_listView;
    QScrollArea *m_scrollArea;
    TableModel m_model;
};

#endif // ITEMVIEWSFORM_H
