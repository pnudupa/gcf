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


#ifndef THUMBNAILIMAGEPROVIDER_H
#define THUMBNAILIMAGEPROVIDER_H

#include <QQuickImageProvider>

struct ThumbnailImageProviderData;
class ThumbnailImageProvider : public QQuickImageProvider
{
public:
    ThumbnailImageProvider();
    ~ThumbnailImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize);

private:
    QImage fetchThumbnail(const QString &fileName, const QSize &requiredSize);
    QImage fetchScaledImage(const QString &fileName, const QSize &requiredSize);

private:
    ThumbnailImageProviderData *d;

};


#endif // THUMBNAILIMAGEPROVIDER_H
