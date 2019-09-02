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

#include "BasicImageFilter.h"

#include <QColor>
#include <QtCore/qmath.h>
#include <QDebug>

/*
    Impementation of the following functions are taken from
    http://api.plexyplanet.org/blitz_8cpp-source.html

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

    Copyright of the code in the above functions is as follows

        Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005, 2007
           Daniel M. Duley <daniel.duley@verizon.net>
        (C) 2004 Zack Rusin <zack@kde.org>
        (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
        (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
        (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
        (C) 1998, 1999 Dirk Mueller <mueller@kde.org>

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions
        are met:

        1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
        IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
        OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
        IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
        INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
        NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
        THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
        THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The authors of the code have also mentioned

        Portions of this software are were originally based on ImageMagick's
        algorithms. ImageMagick is copyrighted under the following conditions:

        Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
        making software imaging solutions freely available.

        Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files ("ImageMagick"), to deal
        in ImageMagick without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
        copies of ImageMagick, and to permit persons to whom the ImageMagick is
        furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of ImageMagick.

        The software is provided "as is", without warranty of any kind, express or
        implied, including but not limited to the warranties of merchantability,
        fitness for a particular purpose and noninfringement.  In no event shall
        ImageMagick Studio be liable for any claim, damages or other liability,
        whether in an action of contract, tort or otherwise, arising from, out of or
        in connection with ImageMagick or the use or other dealings in ImageMagick.

        Except as contained in this notice, the name of the ImageMagick Studio shall
        not be used in advertising or otherwise to promote the sale, use or other
        dealings in ImageMagick without prior written authorization from the
        ImageMagick Studio.
 */

///////////////////////////////////////////////////////////////////////////////
// Component class for BasicImageFilter

#include <GCF3/Component>

class BasicImageFilterComponent : public GCF::Component
{
public:
    BasicImageFilterComponent(QObject *parent=nullptr) : GCF::Component(parent) { }
    ~BasicImageFilterComponent() { }

    QString name() const {
        return "Basic Filters";
    }

    void initializeEvent(GCF::InitializeEvent *e) {
        if(e->isPostInitialize())
            this->addContentObject(new BasicImageFilter);
    }
};

GCF_EXPORT_COMPONENT(BasicImageFilterComponent)

///////////////////////////////////////////////////////////////////////////////

#define PI 3.14159265359
#define TWO_PI 6.28318530718

BasicImageFilter::BasicImageFilter(QObject *parent)
    : QObject(parent)
{
}

BasicImageFilter::~BasicImageFilter()
{
}

QStringList BasicImageFilter::effectNames() const
{
    static QStringList retList;
    if(retList.isEmpty())
    {
        retList << "Invert RGB";
        retList << "Horizontal Mirror";
        retList << "Vertical Mirror";
        retList << "Convert to B&W";
        retList << "Blur";
        retList << "Despeckle";
        retList << "Edge Detection";
        retList << "Oilify";
        retList << "Coffee Stain";
        retList << "Ripple";
    }

    return retList;
}

QImage BasicImageFilter::applyFilterEffect(const QString &filterName, const QImage &image) const
{
    int filterIndex = this->effectNames().indexOf(filterName);
    if(filterIndex < 0)
        return image;

    switch(filterIndex)
    {
    case 0: return this->applyInvertedRgbEffect(image);
    case 1: return this->applyHorizontalMirrorEffect(image);
    case 2: return this->applyVerticalImageEffect(image);
    case 3: return this->applyBlackAndWhiteEffect(image);
    case 4: return this->applyBlurEffect(image);
    case 5: return this->applyDespecleEffect(image);
    case 6: return this->applyEdgeEffect(image);
    case 7: return this->applyOilifyEffect(image);
    case 8: return this->applyCoffeeStainEffect(image);
    case 9: return this->applyRippleEffect(image);
    default: break;
    }

    return image;
}

QImage BasicImageFilter::applyInvertedRgbEffect(const QImage &image) const
{
    return image.rgbSwapped();
}

QImage BasicImageFilter::applyHorizontalMirrorEffect(const QImage &image) const
{
    return image.mirrored(true, false);
}

QImage BasicImageFilter::applyVerticalImageEffect(const QImage &image) const
{
    return image.mirrored(false, true);
}

QImage BasicImageFilter::applyBlackAndWhiteEffect(const QImage &image) const
{
    QImage img = image;
    if (!image.isNull())
    {
        int pixels = img.width() * img.height();
#if QT_VERSION >= 0x050D00
        if (pixels*(int)sizeof(QRgb) <= img.sizeInBytes())
#else
        if (pixels*(int)sizeof(QRgb) <= img.byteCount())
#endif
        {
            QRgb *data = (QRgb *)img.bits();
            for (int i = 0; i < pixels; i++)
            {
                int val = qGray(data[i]);
                data[i] = qRgba(val, val, val, qAlpha(data[i]));
            }
        }
    }
    return img;
}

QImage BasicImageFilter::applyBlurEffect(const QImage &image) const
{
    //int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = 1;

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int rectTopY = image.rect().top();
    int rectBottomY = image.rect().bottom();
    int rectLeftX = image.rect().left();
    int rectRightX = image.rect().right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* scanPerLine;

    int i1 = 0;
    int i2 = 3;

    //    if (alphaOnly)
    //    i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

    for (int col = rectLeftX; col <= rectRightX; col++)
    {
        scanPerLine = result.scanLine(rectTopY) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = scanPerLine[i] << 4;

        scanPerLine += bpl;
        for (int j = rectTopY; j < rectBottomY; j++, scanPerLine += bpl)
            for (int i = i1; i <= i2; i++)
                scanPerLine[i] = (rgba[i] += ((scanPerLine[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = rectTopY; row <= rectBottomY; row++)
    {
        scanPerLine = result.scanLine(row) + rectLeftX * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = scanPerLine[i] << 4;

        scanPerLine += 4;
        for (int j = rectLeftX; j < rectRightX; j++, scanPerLine += 4)
            for (int i = i1; i <= i2; i++)
                scanPerLine[i] = (rgba[i] += ((scanPerLine[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = rectLeftX; col <= rectRightX; col++)
    {
        scanPerLine = result.scanLine(rectBottomY) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = scanPerLine[i] << 4;

        scanPerLine -= bpl;
        for (int j = rectTopY; j < rectBottomY; j++, scanPerLine -= bpl)
            for (int i = i1; i <= i2; i++)
                scanPerLine[i] = (rgba[i] += ((scanPerLine[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = rectTopY; row <= rectBottomY; row++)
    {
        scanPerLine = result.scanLine(row) + rectRightX * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = scanPerLine[i] << 4;

        scanPerLine -= 4;
        for (int j = rectLeftX; j < rectRightX; j++, scanPerLine -= 4)
            for (int i = i1; i <= i2; i++)
                scanPerLine[i] = (rgba[i] += ((scanPerLine[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

QImage BasicImageFilter::applyDespecleEffect(const QImage &img) const
{
    if(img.isNull())
        return(img);

    QImage newImage = img;
    int length, x, y, j, i;
    QRgb *src, *dest;
    unsigned char *buffer, *pixels;
    int imageWidth = newImage.width();
    int imageHeight = newImage.height();

    static const int
            X[4]= {0, 1, 1,-1},
            Y[4]= {1, 0, 1, 1};

    length = (newImage.width()+2)*(newImage.height()+2);
    pixels = new unsigned char[length];
    buffer = new unsigned char[length];

    if(newImage.depth() != 32)
    {
        newImage = newImage.convertToFormat(newImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    }
    else if(newImage.format() == QImage::Format_ARGB32_Premultiplied)
        newImage = newImage.convertToFormat(QImage::Format_ARGB32);

    // Do each channel. This originally was in one loop with an "if" statement
    // for each channel, but I unrolled it because a lot goes on and it needs
    // any speed help it can get >:/ (mosfet)

    // Red
    (void)memset(pixels, 0, length);
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        src = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++src, ++j)
            pixels[j] = qRed(*src);
    }
    (void)memset(buffer, 0, length);
    for(i=0; i < 4; ++i)
    {
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, -1);
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, -1);
    }
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        dest = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++dest, ++j)
            *dest = qRgba(pixels[j], qGreen(*dest), qBlue(*dest),
                          qAlpha(*dest));
    }

    // Green
    (void)memset(pixels, 0, length);
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        src = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++src, ++j)
            pixels[j] = qGreen(*src);
    }
    (void)memset(buffer, 0, length);
    for(i=0; i < 4; ++i)
    {
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, -1);
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, -1);
    }
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        dest = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++dest, ++j)
            *dest = qRgba(qRed(*dest), pixels[j], qBlue(*dest),
                          qAlpha(*dest));
    }

    // Blue
    (void)memset(pixels, 0, length);
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        src = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++src, ++j)
            pixels[j] = qBlue(*src);
    }
    (void)memset(buffer, 0, length);
    for(i=0; i < 4; ++i)
    {
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, 1);
        hull(-X[i], -Y[i], imageWidth, imageHeight, pixels, buffer, -1);
        hull(X[i], Y[i], imageWidth, imageHeight, pixels, buffer, -1);
    }
    j = imageWidth+2;
    for(y=0; y < imageHeight; ++y, ++j)
    {
        dest = (QRgb *)newImage.scanLine(y);
        ++j;
        for(x=imageWidth-1; x >= 0; --x, ++dest, ++j)
            *dest = qRgba(qRed(*dest), qGreen(*dest), pixels[j],
                          qAlpha(*dest));
    }

    delete[] pixels;
    delete[] buffer;
    return(newImage);
}

void BasicImageFilter::hull(int x_offset, int y_offset, int w, int h,
                            unsigned char *f, unsigned char *g,
                            int polarity) const
{
    int x, y, v;
    unsigned char *p, *q, *r, *s;
    p = f+(w+2); q = g+(w+2);
    r = p+(y_offset*(w+2)+x_offset);
    for(y=0; y < h; ++y, ++p, ++q, ++r)
    {
        ++p; ++q; ++r;
        if(polarity > 0)
        {
            for(x=w; x > 0; --x, ++p, ++q, ++r)
            {
                v = (*p);
                if((int)*r >= (v+2)) v += 1;
                *q = (unsigned char)v;
            }
        }
        else
        {
            for(x=w; x > 0; --x, ++p, ++q, ++r)
            {
                v = (*p);
                if((int)*r <= (v-2)) v -= 1;
                *q = (unsigned char)v;
            }
        }
    }
    p = f+(w+2); q = g+(w+2);
    r = q+(y_offset*(w+2)+x_offset); s = q-(y_offset*(w+2)+x_offset);

    for(y=0; y < h; ++y, ++p, ++q, ++r, ++s)
    {
        ++p; ++q; ++r; ++s;
        if(polarity > 0)
        {
            for(x=w; x > 0; --x, ++p, ++q, ++r, ++s){
                v = (*q);
                if(((int)*s >= (v+2)) && ((int)*r > v)) v+=1;
                *p = (unsigned char)v;
            }
        }
        else
        {
            for(x=w; x > 0; --x, ++p, ++q, ++r, ++s){
                v = (int)(*q);
                if (((int)*s <= (v-2)) && ((int)*r < v)) v -= 1;
                *p = (unsigned char)v;
            }
        }
    }
}

QImage BasicImageFilter::applyEdgeEffect(const QImage &image) const
{
    QImage newImage = image;
    int sizeX, sizeY;
    int i, j, k;
    int pixel, grayScaleX, grayScaleY, grayScaleG;

    QColor * color;
    QRgb nPixel;
    QImage resultat, SobelX, SobelY;

    sizeX = newImage.width();
    sizeY = newImage.height();

    if(!newImage.isGrayscale())
    {
        k = 0;
        for(i = 0; i < sizeX; i++)
        {
            for(j = 0; j < sizeY; j++)
            {
                pixel = newImage.pixel(i, j);
                color = new QColor(
                            qGray(pixel), qGray(pixel), qGray(pixel)
                            );

                nPixel = color->rgb();
                newImage.setPixel(i, j, nPixel);
                delete color;
                k++;
            }
        }
    }

    resultat = newImage.copy(0, 0, sizeX, sizeY);
    SobelX = newImage.copy(0, 0, sizeX, sizeY);
    SobelY = newImage.copy(0, 0, sizeX, sizeY);


    k = 0;
    for(i = 1; i < sizeX-1; i++)
    {
        for(j = 1; j < sizeY-1; j++)
        {
            /****** Gx *******/
            /**
             * -1 0 +1
             * -2 0 +2
             * -1 0 +1
             **/
            grayScaleX =
                    (
                        -1*qGray(newImage.pixel(i-1, j-1)) + 1*qGray(newImage.pixel(i+1, j-1))
                        -2*qGray(newImage.pixel(i-1, j))   + 2*qGray(newImage.pixel(i+1, j))
                        -1*qGray(newImage.pixel(i-1, j+1)) + 1*qGray(newImage.pixel(i+1, j+1))
                        );

            /****** Gy *******/
            /**
             * +1  2  +1
             * 0   0   0
             * -1 -2  -1
             **/
            grayScaleY =
                    (
                        qGray(newImage.pixel(i-1, j-1))   + 2*qGray(newImage.pixel(i, j-1)) + qGray(newImage.pixel(i+1, j-1))
                        // 0 + 0 + 0
                        - qGray(newImage.pixel(i-1, j+1))	- 2*qGray(newImage.pixel(i, j+1)) - qGray(newImage.pixel(i+1, j+1))
                        );

            grayScaleG = qSqrt(grayScaleX*grayScaleX + grayScaleY*grayScaleY);

            if(grayScaleG < 0)
                grayScaleG = 0;

            if(grayScaleG > 255)
                grayScaleG = 255;

            color = new QColor(grayScaleG, grayScaleG, grayScaleG);
            nPixel = color->rgb();

            resultat.setPixel
                    (
                        i, j,
                        nPixel
                        );

            delete color;

            k++;
        }
    }

    return resultat;
}

QImage BasicImageFilter::applyOilifyEffect(const QImage &image) const
{
    QImage img = image;
    if (!image.isNull())
    {
        int cols = img.width()-1;
        int rows = img.height()-1;
        for ( int col = 0; col<cols; col++ )
        {
            for ( int row =0; row<rows; row++ )
            {
                img.setPixel(col, row, calcCommonSurroundingRgb(image, col, row).rgb());
            }
        }

    }
    return img;
}

QColor BasicImageFilter::calcCommonSurroundingRgb(const QImage &image, int col, int row ) const
{
    QColor mostCommonColor = QColor::fromRgb(255, 255, 255);
    if ( image.isNull() ) return mostCommonColor;

    QImage convertedImage = image;
    int cols = convertedImage.width()-1;
    int rows = convertedImage.height()-1;

    if ( row == 0 || row >= rows || col == 0 || col >= cols ) return image.pixel(col, row);

    QMap<QRgb, int> colorFoundMap;
    for ( int i=-1; i<=1; i++ )
    {
        QColor  aboveRowColr = QColor::fromRgb(image.pixel(col+i, row-1));
        if ( colorFoundMap.contains(aboveRowColr.rgb()) )
        {
            int counter = colorFoundMap[aboveRowColr.rgb()];
            colorFoundMap[aboveRowColr.rgb()] = counter+1;
        } else
            colorFoundMap[aboveRowColr.rgb()] = 1;

        QColor  belowRowColr = QColor::fromRgb(image.pixel(col+i, row+1));
        if ( colorFoundMap.contains(belowRowColr.rgb()) )
        {
            int counter = colorFoundMap[belowRowColr.rgb()];
            colorFoundMap[belowRowColr.rgb()] = counter+1;
        } else
            colorFoundMap[belowRowColr.rgb()] = 1;
    }

    QColor leftColr = QColor::fromRgb(image.pixel(col-1, row));
    if ( colorFoundMap.contains(leftColr.rgb()) )
    {
        int counter = colorFoundMap[leftColr.rgb()];
        colorFoundMap[leftColr.rgb()] = counter+1;
    } else
        colorFoundMap[leftColr.rgb()] = 1;

    QColor rightColr = QColor::fromRgb(image.pixel(col+1, row));
    if ( colorFoundMap.contains(rightColr.rgb()) )
    {
        int counter = colorFoundMap[rightColr.rgb()];
        colorFoundMap[rightColr.rgb()] = counter+1;
    } else
        colorFoundMap[rightColr.rgb()] = 1;

    QMap<QRgb, int>::Iterator iter = colorFoundMap.begin();
    int count = 0;
    for ( ; iter != colorFoundMap.end(); iter++ )
    {
        QColor curColor = QColor::fromRgb(iter.key());
        int curCount = iter.value();

        if ( count < curCount )
        {
            count = curCount;
            mostCommonColor = curColor;
        }
    }

    return mostCommonColor;
}

QImage BasicImageFilter::applyCoffeeStainEffect(const QImage &image) const
{
    return image;
}

QImage BasicImageFilter::applyRippleEffect(const QImage &image) const
{
    QImage newImage = image;
    double w = TWO_PI / 50.0;
    QRgb rgb;

    for (int y = 0;y < image.height();y++)
    {
        for (int x = 0;x < image.width();x++)
        {
            double y0 = y + 5 * sin(w * x + 3);
            double x0 = x + 5 * cos(w * y + 3);
            if ((y0 < 0) || (y0 >= image.height()) ||
                    (x0 < 0) || (x0 >= image.width()))
            {
                rgb = qRgb(255, 255, 255);
            }
            else
            {
                rgb = image.pixel((int)x0,(int)y0);
            }

            newImage.setPixel(x,y,rgb);
        }
    }
    return newImage;
}
