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

#ifndef KLIPPIEMODEL_H
#define KLIPPIEMODEL_H

#include <QVariantList>
#include <QAbstractListModel>
#include <QQuickImageProvider>

class KlippieModel : public QAbstractListModel, public QQuickImageProvider
{
    Q_OBJECT

public:
    KlippieModel(QObject *parent=nullptr);
    ~KlippieModel();

    Q_PROPERTY(int size READ size NOTIFY sizeChanged)
    int size() const { return m_contents.size(); }
    Q_SIGNAL void sizeChanged();

    Q_INVOKABLE void clear();
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void sync(); // copies from the system-clipboard to this model
    Q_INVOKABLE void use(int index);

    // QAbstractItemModel interface
    enum Roles { ContentRole = Qt::UserRole+1 };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

signals:
    void newItem(const QVariant &item);

public slots:
    void prependItem(const QVariant &item);

private:
    QVariantList m_contents;
};

#endif // KLIPPIEMODEL_H
