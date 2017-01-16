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

#ifndef BASICIMAGEFILTER_H
#define BASICIMAGEFILTER_H

#include <QObject>
#include "IImageFilter.h"

class BasicImageFilter : public QObject, public IImageFilter
{
    Q_OBJECT
    Q_INTERFACES(IImageFilter)

public:
    BasicImageFilter(QObject *parent = 0);
    ~BasicImageFilter();

    // IImageFilter implementation
    QStringList effectNames() const;
    QImage applyFilterEffect(const QString &, const QImage &) const;

private:
    QImage applyInvertedRgbEffect(const QImage &) const;
    QImage applyHorizontalMirrorEffect(const QImage &) const;
    QImage applyVerticalImageEffect(const QImage &) const;
    QImage applyBlackAndWhiteEffect(const QImage &) const;
    QImage applyBlurEffect(const QImage &) const;
    QImage applyDespecleEffect(const QImage &) const;
    QImage applyEdgeEffect(const QImage &) const;
    QImage applyOilifyEffect(const QImage &) const;
    QImage applyCoffeeStainEffect(const QImage &) const;
    QImage applyRippleEffect(const QImage &) const;

    void hull(int x_offset, int y_offset, int w, int h,
              unsigned char *f, unsigned char *g,
              int polarity) const;
    QColor calcCommonSurroundingRgb(const QImage & image, int col, int row) const;
};

#endif // BASICIMAGEFILTER_H
