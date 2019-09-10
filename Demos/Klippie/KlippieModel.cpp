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

#include "KlippieModel.h"

#include <QtDebug>
#include <QMimeData>
#include <QClipboard>
#include <QGuiApplication>

KlippieModel::KlippieModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

KlippieModel::~KlippieModel()
{

}

void KlippieModel::clear()
{
    this->beginResetModel();
    m_contents.clear();
    this->endResetModel();
    emit sizeChanged();
}

void KlippieModel::removeAt(int index)
{
    if(index < 0 || index >= m_contents.size())
        return;

    this->beginRemoveRows(QModelIndex(), index, index);
    m_contents.removeAt(index);
    this->endRemoveRows();
}

void KlippieModel::sync()
{
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if(mimeData == nullptr)
        return;

    const QStringList formats = mimeData->formats();

    QVariantMap item;
    item["formats"] = formats;
    Q_FOREACH(QString format, formats)
    {
        const QByteArray data = mimeData->data(format);
        item[format] = data;
    }

    this->prependItem(item);
    emit newItem(item);
}

void KlippieModel::use(int index)
{
    if(index < 0 || index >= m_contents.size())
        return;

    const QVariantMap item = m_contents.at(index).toMap();
    if(item.isEmpty())
        return;

    const QStringList formats = item.value("formats").toStringList();
    if(formats.isEmpty())
        return;

    QMimeData *mimeData = new QMimeData;
    Q_FOREACH(QString format, formats)
        mimeData->setData(format, item.value(format).toByteArray());
    qApp->clipboard()->setMimeData(mimeData);
}

int KlippieModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_contents.size();
}

QVariant KlippieModel::data(const QModelIndex &index, int role) const
{
    if(role == ContentRole)
    {
        if(index.row() < 0 || index.row() >= m_contents.size())
            return QVariant();

        return m_contents.at(index.row());
    }

    return QVariant();
}

QHash<int, QByteArray> KlippieModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[ContentRole] = "content";
    return roles;
}

void KlippieModel::prependItem(const QVariant &item)
{
    this->beginInsertRows(QModelIndex(), 0, 0);
    m_contents.prepend(item);
    this->endInsertRows();

    emit sizeChanged();
}


