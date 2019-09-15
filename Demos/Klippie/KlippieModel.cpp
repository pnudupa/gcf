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
#include <QUrl>
#include <QBuffer>
#include <QImage>

KlippieModel::KlippieModel(QObject *parent)
    : QAbstractListModel(parent),
      QQuickImageProvider(Image)
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

    QVariantMap item;
    if(mimeData->hasHtml())
    {
        item["type"] = "html";
        item["data"] = mimeData->html();
        item["textData"] = mimeData->text();
    }
    else if(mimeData->hasUrls())
    {
        item["type"] = "urls";
        QStringList urls;
        Q_FOREACH(QUrl url, mimeData->urls())
            urls << url.toString();
        item["data"] = urls;
    }
    else if(mimeData->hasImage())
    {
        item["type"] = "image";

        const QImage image = mimeData->imageData().value<QImage>();
        QByteArray imageData;
        QBuffer imageDevice(&imageData);
        imageDevice.open(QIODevice::WriteOnly);
        image.save(&imageDevice, "PNG");
        imageDevice.close();

        item["data"] = imageData;
    }
    else if(mimeData->hasText())
    {
        item["type"] = "text";
        item["data"] = mimeData->text();
    }
    else
    {
        const QStringList formats = mimeData->formats();
        QVariantList data;
        Q_FOREACH(QString format, formats)
        {
            QVariantMap entry;
            entry["mimetype"] = format;
            entry["data"] = mimeData->data(format);
            data.append(entry);
        }
        item["type"] = "mime-data";
        item["data"] = data;
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

    const QStringList types = QStringList() << "html" << "urls" << "image" << "text" << "mime-data";
    const int typeIndex = types.indexOf(item.value("type").toString());
    if(typeIndex < 0)
        return;

    QClipboard *clipboard = qApp->clipboard();

    switch(typeIndex)
    {
    case 0: {
        QMimeData *mimeData = new QMimeData;
        mimeData->setHtml(item.value("data").toString());
        mimeData->setText(item.value("textData").toString());
        clipboard->setMimeData(mimeData);
        } break;
    case 1: {
        QMimeData *mimeData = new QMimeData;
        const QStringList urlvs = item.value("data").toStringList();
        QList<QUrl> urls;
        Q_FOREACH(QString urlv, urlvs)
            urls << QUrl(urlv);
        mimeData->setUrls(urls);
        clipboard->setMimeData(mimeData);
        } break;
    case 2: {
        QByteArray imageData = item.value("data").toByteArray();
        QBuffer imageDevice(&imageData);
        imageDevice.open(QIODevice::ReadOnly);

        QImage image;
        image.load(&imageDevice, "PNG");
        clipboard->setImage(image);
        } break;
    case 3:
        clipboard->setText( item.value("data").toString() );
        break;
    case 4: {
        const QVariantList data = item.value("data").toList();
        QMimeData *mimeData = new QMimeData;
        QVariantList::const_iterator it = data.begin();
        QVariantList::const_iterator end = data.end();
        while(it != end)
        {
            const QVariantMap entry = (*it).toMap();
            mimeData->setData( entry.value("mimetype").toString(),
                               entry.value("data").toByteArray() );
            ++it;
        }
        clipboard->setMimeData(mimeData);
        } break;
    default:
        break;
    }
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

QImage KlippieModel::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const int row = id.toInt();
    if(row < 0 || row >= m_contents.size())
        return QImage();

    const QVariantMap item = m_contents.at(row).toMap();
    if(item.value("type").toString() != "image")
        return QImage();

    QByteArray imageData = item.value("data").toByteArray();
    QBuffer imageDevice(&imageData);
    imageDevice.open(QIODevice::ReadOnly);

    QImage image;
    image.load(&imageDevice, "PNG");
    if(size)
        *size = image.size();
    if(!requestedSize.isNull() && !requestedSize.isEmpty() && requestedSize.isValid())
        image = image.scaled(requestedSize);

    return image;
}

void KlippieModel::prependItem(const QVariant &item)
{
    this->beginInsertRows(QModelIndex(), 0, 0);
    m_contents.prepend(item);
    this->endInsertRows();

    emit sizeChanged();
}


