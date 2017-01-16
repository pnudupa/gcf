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

#ifndef FILTEREDIMAGEPROVIDER_H
#define FILTEREDIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include "FiltersModel.h"

class FilteredImageProvider : public QQuickImageProvider
{
public:
    FilteredImageProvider(FiltersModel *filtersModel)
        : QQuickImageProvider(Image),
          m_filtersModel(filtersModel) {}
    ~FilteredImageProvider() { }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        QString fileUrl = id.section('?', 0, 0);
        QString fileName = QUrl(fileUrl).toLocalFile();
        QString indexStr = id.section('?', 1, 1);
        int filterIndex = indexStr.isEmpty() ? -1 : indexStr.toInt();
        QImage image(fileName);
        image = m_filtersModel->applyFilterAt(filterIndex, image);
        if(!requestedSize.isEmpty())
            image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if(size)
            *size = image.size();
        return image;
    }

private:
    FiltersModel *m_filtersModel;
};

#endif // FILTEREDIMAGEPROVIDER_H
