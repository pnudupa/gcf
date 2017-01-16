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

#include "AdvanceImageFilter.h"

#include <QColor>
#include <QtCore/qmath.h>

/*
  Source code for the following cuntions are taken from
  https://github.com/halcv/ImgEffect/blob/master/src/effect.cpp.

        QImage applyGrayEffect(const QImage &pImage) const;
        QImage applySepiaEffect(const QImage &pImage) const;
        QImage applyReverseEffect(const QImage &pImage) const;
        QImage applyPencil1Effect(const QImage &pImage) const;
        QImage applyPencil2Effect(const QImage &pImage) const;
        QImage applyPencil3Effect(const QImage &pImage) const;
        QImage applyColorPencil1Effect(const QImage &pImage) const;
        QImage applyColorPencil2Effect(const QImage &pImage) const;
        QImage applyColorPencil3Effect(const QImage &pImage) const;
        QImage applyNitika1Effect(const QImage &pImage) const;
        QImage applyNitika2Effect(const QImage &pImage) const;
        QImage applyHekigaEffect(const QImage &pImage) const;
        QImage applyGrayPencil1Effect(const QImage &pImage) const;
        QImage applyGrayPencil2Effect(const QImage &pImage) const;
        QImage applyGrayPencil3Effect(const QImage &pImage) const;
        QImage applyEmbossEffect(const QImage &pImage) const;
        QImage applyColorPaintEffect(const QImage &pImage) const;
        QImage applyGrayPaintEffect(const QImage &pImage) const;
        QImage applyGoldPaintEffect(const QImage &pImage) const;
        QImage applyLensEffect(const QImage &pImage) const;
        QImage applyNoizeEffect(const QImage &pImage) const;
        QImage applyLightColorEffect(const QImage &pImage) const;
        QImage applyVividColorEffect(const QImage &pImage) const;
        QImage applySharpnessEffect(const QImage &pImage) const;
        QImage applySoftFocusEffect(const QImage &pImage) const;
        int processGreenPencilFilter(const QImage &pImg,int t_x,int t_y,char* mask) const;
        void processSoftFocusFilter(const QImage &pImg,int t_x,int t_y,int* pRed,int* pGreen,int* pBlue) const;
        void processColorFilter(const QImage &pImg,int t_x,int t_y,char* mask,int* pRed,int* pGreen,int* pBlue) const;
        int calcTotalAverage(const QImage &pImg) const;
        int calcLineAverage(const QImage &pImg,int line) const;
        int processEnbosFilter(const QImage &pImg,int t_x,int t_y,char* mask) const;
        QImage processContrastEffect(const QImage &pImage,int LowParam,int HighParam) const;

  In the absense of any license terms stated for the code (link above)
  and the fact that code is available in a public repository for read
  access without any authorization requirements: we assume that the
  code can be used for free.
*/

///////////////////////////////////////////////////////////////////////////////
// Component class for AdvanceImageFilter

#include <GCF3/Component>

class AdvanceImageFilterComponent : public GCF::Component
{
public:
    AdvanceImageFilterComponent(QObject *parent=0) : GCF::Component(parent) { }
    ~AdvanceImageFilterComponent() { }

    QString name() const {
        return "Advanced Filters";
    }

    void initializeEvent(GCF::InitializeEvent *e) {
        if(e->isPostInitialize())
            this->addContentObject(new AdvanceImageFilter);
    }
};

GCF_EXPORT_COMPONENT(AdvanceImageFilterComponent)

///////////////////////////////////////////////////////////////////////////////

#define PI 3.14159265359
#define TWO_PI 6.28318530718

AdvanceImageFilter::AdvanceImageFilter(QObject *parent)
    : QObject(parent)
{
}

AdvanceImageFilter::~AdvanceImageFilter()
{
}

QStringList AdvanceImageFilter::effectNames() const
{
    static QStringList retList;
    if(retList.isEmpty())
    {
        retList << "Gray";
        retList << "Sepia";
        retList << "Reverse";
        retList << "Pencil 1";
        retList << "Pencil 2";
        retList << "Pencil 3";
        retList << "Color Pencil 1";
        retList << "Color Pencil 2";
        retList << "Color Pencil 3";
        retList << "Nitika 1";
        retList << "Nitika 2";
        retList << "Hekiga";
        retList << "Gray Pencil 1";
        retList << "Gray Pencil 2";
        retList << "Gray Pencil 3";
        retList << "Emboss";
        retList << "Color Paint";
        retList << "Gray Paint";
        retList << "Gold Paint";
        retList << "Lens";
        retList << "Noise";
        retList << "Light Color";
        retList << "Vivid Color";
        retList << "Sharpness";
        retList << "Soft Focus";
    }

    return retList;
}

QImage AdvanceImageFilter::applyFilterEffect(const QString &filterName, const QImage &image) const
{
    int filterIndex = this->effectNames().indexOf(filterName);
    if(filterIndex < 0)
        return image;

    switch(filterIndex)
    {
    case 0:  return this->applyGrayEffect(image);
    case 1:  return this->applySepiaEffect(image);
    case 2:  return this->applyReverseEffect(image);
    case 3:  return this->applyPencil1Effect(image);
    case 4:  return this->applyPencil2Effect(image);
    case 5:  return this->applyPencil3Effect(image);
    case 6:  return this->applyColorPencil1Effect(image);
    case 7:  return this->applyColorPencil2Effect(image);
    case 8:  return this->applyColorPencil3Effect(image);
    case 9:  return this->applyNitika1Effect(image);
    case 10: return this->applyNitika2Effect(image);
    case 11: return this->applyHekigaEffect(image);
    case 12: return this->applyGrayPencil1Effect(image);
    case 13: return this->applyGrayPencil2Effect(image);
    case 14: return this->applyGrayPencil3Effect(image);
    case 15: return this->applyEmbossEffect(image);
    case 16: return this->applyColorPaintEffect(image);
    case 17: return this->applyGrayPaintEffect(image);
    case 18: return this->applyGoldPaintEffect(image);
    case 19: return this->applyLensEffect(image);
    case 20: return this->applyNoizeEffect(image);
    case 21: return this->applyLightColorEffect(image);
    case 22: return this->applyVividColorEffect(image);
    case 23: return this->applySharpnessEffect(image);
    case 24: return this->applySoftFocusEffect(image);
    default: break;
    }

    return image;
}

QImage AdvanceImageFilter::applyGrayEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            QRgb rgb = newImage.pixel(x,y);
            int gray = qGreen(rgb);
            newImage.setPixel(x,y,qRgb(gray,gray,gray));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applySepiaEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int r;
    int g;
    int b;

    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            QRgb rgb = newImage.pixel(x,y);
            int gray = qGreen(rgb);
            r = gray * 240 / 255;
            g = gray * 200 / 255;
            b = gray * 145 / 255;
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyReverseEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            QRgb rgb = newImage.pixel(x,y);
            int r = 255 - qRed(rgb);
            int g = 255 - qGreen(rgb);
            int b = 255 - qBlue(rgb);
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyPencil1Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -2 , 0, 2,
        -4 , 0, 4,
        -2 , 0, 2
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -2 , -4, -2,
         0 ,  0,  0,
         2 ,  4,  2
    };

    for(int y=0; y<pImage.height(); y++)
    {
        for(int x=0; x<pImage.width(); x++)
        {
            green1 = processGreenPencilFilter(pImage,x,y,mask_x);
            green2 = processGreenPencilFilter(pImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            newImage.setPixel(x,y,qRgb(green,green,green));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyPencil2Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -1 , 0, 1,
        -2 , 0, 2,
        -1 , 0, 1
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -2 , -4, -2,
         0 ,  0,  0,
         2 ,  4,  2
    };

    for(int y=0; y<pImage.height(); y++)
    {
        for(int x=0; x<pImage.width(); x++)
        {
            green1 = processGreenPencilFilter(pImage,x,y,mask_x);
            green2 = processGreenPencilFilter(pImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            newImage.setPixel(x,y,qRgb(green,green,green));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyPencil3Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage softImage = newImage;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -1 , 0, 1,
        -2 , 0, 2,
        -1 , 0, 1
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -1 , -2, -1,
        0 ,  0,  0,
        1 ,  2,  1
    };

    for(int i=0; i<3; i++)
    {
        softImage = applySoftFocusEffect(softImage);
    }

    for(int y=0; y<softImage.height(); y++)
    {
        for(int x=0; x<softImage.width(); x++)
        {
            green1 = processGreenPencilFilter(softImage,x,y,mask_x);
            green2 = processGreenPencilFilter(softImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            newImage.setPixel(x,y,qRgb(green,green,green));
        }
    }
    return newImage;
}

QImage AdvanceImageFilter::applyColorPencil1Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = newImage;
    char r;
    char g;
    char b;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -1 , 0, 1,
        -2 , 0, 2,
        -1 , 0, 1
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -1 , -2, -1,
         0 ,  0,  0,
         1 ,  2,  1
    };

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green1 = processGreenPencilFilter(tempImage,x,y,mask_x);
            green2 = processGreenPencilFilter(tempImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            QRgb rgb = newImage.pixel(x,y);
            if (qRed(rgb) <= 50 && qGreen(rgb) <= 50 && qBlue(rgb) <= 50)
            {
                r = qRed(rgb);
                g = qGreen(rgb);
                b = qBlue(rgb);
            }
            else
            {
                r = (qRed(rgb) + green) / 2;
                g = (qGreen(rgb) + green) / 2;
                b = (qBlue(rgb) + green) / 2;
            }
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyColorPencil2Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = newImage;
    int r;
    int g;
    int b;

    int green,green1,green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -1 , 0, 1,
        -2 , 0, 2,
        -1 , 0, 1
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -1 , -2, -1,
         0 ,  0,  0,
         1 ,  2,  1
    };
    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green1 = processGreenPencilFilter(tempImage,x,y,mask_x);
            green2 = processGreenPencilFilter(tempImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            if (green < 127)
                green = 0;

            green = 255 - green;

            QRgb rgb = newImage.pixel(x,y);
            r = (qRed(rgb) * green) / 255;
            g = (qGreen(rgb) * green) / 255;
            b = (qBlue(rgb) * green) / 255;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyColorPencil3Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage softImage = newImage;
    char r;
    char g;
    char b;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -2 , 0, 2,
        -4 , 0, 4,
        -2 , 0, 2
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -2 , -4, -2,
         0 ,  0,  0,
         2 ,  4,  2
    };

    for(int i=0; i<3; i++)
    {
        softImage = applySoftFocusEffect(softImage);
    }

    for(int y=0; y<softImage.height(); y++)
    {
        for(int x=0; x<softImage.width(); x++)
        {
            green1 = processGreenPencilFilter(softImage,x,y,mask_x);
            green2 = processGreenPencilFilter(softImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            QRgb rgb = softImage.pixel(x,y);
            r = (qRed(rgb) + green) / 2;
            g = (qGreen(rgb) + green) / 2;
            b = (qBlue(rgb) + green) / 2;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyNitika1Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int totalavg;
    int threshold;
    int nitika;

    totalavg = calcTotalAverage(tempImage);

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            QRgb rgb = tempImage.pixel(x,y);
            threshold = totalavg;
            if (qGreen(rgb) > threshold)
                nitika = 255;
            else
                nitika = 0;

            newImage.setPixel(x,y,qRgb(nitika,nitika,nitika));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyNitika2Effect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int totalavg;
    int threshold;
    int lineavg;
    char nitika;

    totalavg = calcTotalAverage(tempImage);

    for(int y=0; y<tempImage.height(); y++)
    {
        lineavg = calcLineAverage(tempImage,y);
        for(int x=0; x<tempImage.width(); x++)
        {
            threshold = totalavg;
            if (totalavg > lineavg)
                threshold = lineavg;

            QRgb rgb = tempImage.pixel(x,y);
            if (qGreen(rgb) > threshold)
                nitika = (char)255;
            else
                nitika = (char)0;

            newImage.setPixel(x,y,qRgb(nitika,nitika,nitika));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyHekigaEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int red;
    int green;
    int blue;

    for(int y = 0;y < tempImage.height();y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            processSoftFocusFilter(tempImage,x,y,&red,&green,&blue);
            QRgb rgb = newImage.pixel(x,y);
            if (qGreen(rgb) > green)
            {
                green = 255;
            }
            else
            {
                green = 0;
            }
            red = (qRed(rgb) + green) >> 1;
            blue = (qBlue(rgb) + green) >> 1;
            green = (qGreen(rgb) + green) >> 1;
            newImage.setPixel(x,y,qRgb(red,green,blue));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyGrayPencil1Effect(const QImage &pImage) const
{
    QImage newImage = applyGrayEffect(pImage);
    QImage tempImage = newImage;
    char r;
    char g;
    char b;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -2 , 0, 2,
        -4 , 0, 4,
        -2 , 0, 2
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -2 , -4, -2,
         0 ,  0,  0,
         2 ,  4,  2
    };

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green1 = processGreenPencilFilter(tempImage,x,y,mask_x);
            green2 = processGreenPencilFilter(tempImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            QRgb rgb = newImage.pixel(x,y);
            r = (qRed(rgb) + green) / 2;
            g = (qGreen(rgb) + green) / 2;
            b = (qBlue(rgb) + green) / 2;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }
    return newImage;
}

QImage AdvanceImageFilter::applyGrayPencil2Effect(const QImage &pImage) const
{
    QImage newImage = applyGrayEffect(pImage);
    QImage tempImage = newImage;
    int r;
    int g;
    int b;
    int max;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -1 , 0, 1,
        -2 , 0, 2,
        -1 , 0, 1
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -1 , -2, -1,
         0 ,  0,  0,
         1 ,  2,  1
    };

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green1 = processGreenPencilFilter(tempImage,x,y,mask_x);
            green2 = processGreenPencilFilter(tempImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            QRgb rgb = newImage.pixel(x,y);
            r = (qRed(rgb) * green) / 255;
            g = (qGreen(rgb) * green) / 255;
            b = (qBlue(rgb) * green) / 255;
            if (r >= g)
                max = r;
            else
                max = g;

            if (b >= max)
                max = b;

            max /= 2;
            r += max;
            if (r > 255)
                r = 255;

            g += max;
            if (g > 255)
                g = 255;

            b += max;
            if (b > 255)
                b = 255;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }
    return newImage;
}

QImage AdvanceImageFilter::applyGrayPencil3Effect(const QImage &pImage) const
{
    QImage newImage = applyGrayEffect(pImage);
    QImage softImage = newImage;
    char r;
    char g;
    char b;

    int green;
    int green1;
    int green2;
    /* Sobel Operator (X) */
    char mask_x[] = {
        -2 , 0, 2,
        -4 , 0, 4,
        -2 , 0, 2
    };
    /* Sobel Operator (Y) */
    char mask_y[] = {
        -2 , -4, -2,
         0 ,  0,  0,
         2 ,  4,  2
    };

    for(int i=0; i<3; i++)
    {
        softImage = applySoftFocusEffect(softImage);
    }

    for(int y=0; y<softImage.height(); y++)
    {
        for(int x=0; x<softImage.width(); x++)
        {
            green1 = processGreenPencilFilter(softImage,x,y,mask_x);
            green2 = processGreenPencilFilter(softImage,x,y,mask_y);
            green = sqrt(double((green1 * green1) + (green2 * green2)));
            if (green > 255)
                green = 255;

            green = 255 - green;
            QRgb rgb = softImage.pixel(x,y);
            r = (qRed(rgb) + green) / 2;
            g = (qGreen(rgb) + green) / 2;
            b = (qBlue(rgb) + green) / 2;
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyEmbossEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int green;
    char mask[] = {
        -3 , 0, 0,
         0 , 0, 0,
         0 , 0, 3
    };

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green = processEnbosFilter(tempImage,x,y,mask);
            newImage.setPixel(x,y,qRgb(green,green,green));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyColorPaintEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int green;
    int r;
    int g;
    int b;

    char mask[] = {
         -6 ,  0, 0,
          0 ,  0, 0,
          0 ,  0, 6
    };

    tempImage = applySoftFocusEffect(tempImage);

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green = processEnbosFilter(tempImage,x,y,mask);
            QRgb rgb = tempImage.pixel(x,y);
            r = ((qRed(rgb) + green) / 2);
            g = ((qGreen(rgb) + green) / 2);
            b = ((qBlue(rgb) + green) / 2);
            if (r < 0)
                r = 0;

            if (g < 0)
                g = 0;

            if (b < 0)
                b = 0;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    newImage = processContrastEffect(newImage,60,60);

    return newImage;
}

QImage AdvanceImageFilter::applyGrayPaintEffect(const QImage &pImage) const
{
    QImage newImage = applyGrayEffect(pImage);
    int green;
    int r,g,b;

    newImage = applyGrayEffect(newImage);

    QImage tempImage = newImage;

    char mask[] = {
         -6 ,  0, 0,
          0 ,  0, 0,
          0 ,  0, 6
    };


    tempImage = applySoftFocusEffect(tempImage);

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green = processEnbosFilter(tempImage,x,y,mask);
            QRgb rgb = tempImage.pixel(x,y);
            r = ((qRed(rgb) + green) / 2);
            g = ((qGreen(rgb) + green) / 2);
            b = ((qBlue(rgb) + green) / 2);
            if (r < 0)
                r = 0;

            if (g < 0)
                g = 0;

            if (b < 0)
                b = 0;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    newImage = processContrastEffect(newImage,60,60);

    return newImage;
}

QImage AdvanceImageFilter::applyGoldPaintEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int green;
    int r;
    int g;
    int b;

    newImage = applyGrayEffect(newImage);

    QImage tempImage = pImage;
    char mask[] = {
         -3 ,  0, 0,
          0 ,  0, 0,
          0 ,  0, 3
    };

    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            green = processEnbosFilter(tempImage,x,y,mask);
            r = green * 255 / 255;
            g = green * 215 / 255;
            b = 0;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyLensEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    QImage tempImage = pImage;
    int lensD;
    int r;
    int sx;
    int sy;
    QRgb rgb;

    if (tempImage.height() > tempImage.width())
    {
        lensD = tempImage.width();
        sx = 0;
        sy = (tempImage.height() - lensD) / 2;
    }
    else
    {
        lensD = tempImage.height();
        sx = (tempImage.width() - lensD) / 2;
        sy = 0;
    }
    r = lensD / 2;

    for(int y=sy; y<sy+lensD; y++)
    {
        for(int x=sx; x<sx+lensD; x++)
        {
            int tx = x - sx - r;
            int ty = y - sy - r;
            float l = sqrt(double(tx*tx + ty*ty));
            int x0,y0;
            if (l > (float)r)
            {
                x0 = x;
                y0 = y;
            }
            else
            {
                float z = sqrt(double(r*r - tx*tx - ty*ty));

                x0 = (((2*r-z)*tx) / (2*r)) + sx + r;
                y0 = (((2*r-z)*ty) / (2*r)) + sy + r;
            }
            rgb = tempImage.pixel(x0,y0);
            newImage.setPixel(x,y,rgb);
        }
    }
    return newImage;
}

QImage AdvanceImageFilter::applyNoizeEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int r;
    int g;
    int b;
    int random;
    QRgb rgb;
    srand(100);

    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            rgb = newImage.pixel(x,y);
            r = qRed(rgb);
            g = qGreen(rgb);
            b = qBlue(rgb);
            random = rand() % 10;

            switch (random)
            {
                case 0:
                    r += 64;
                    if (r > 255)
                        r = 255;

                    g += 32;
                    if (g > 255)
                        g = 255;

                    b += 32;
                    if (b > 255)
                        b = 255;
                    break;
                case 1:
                    r -= 64;
                    if (r < 0)
                        r = 0;

                    g -= 32;
                    if (g < 0)
                        g = 0;

                    b -= 32;
                    if (b < 0)
                        b = 0;
                    break;
            }

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyLightColorEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int r;
    int g;
    int b;
    int avg;
    QRgb rgb;

    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x <newImage.width(); x++)
        {
            rgb = newImage.pixel(x,y);
            avg = (qRed(rgb) + qGreen(rgb) + qBlue(rgb)) / 3;
            r = (qRed(rgb) + avg) / 2;
            g = (qGreen(rgb) + avg) / 2;
            b = (qBlue(rgb) + avg) / 2;
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applyVividColorEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int r;
    int g;
    int b;
    int maxcolor;
    int minicolor;
    int temp;
    QRgb rgb;

    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            rgb = newImage.pixel(x,y);

            if (qRed(rgb) > qGreen(rgb))
                maxcolor = qRed(rgb);
            else
                maxcolor = qGreen(rgb);

            if (qBlue(rgb) > maxcolor)
                maxcolor = qBlue(rgb);

            if (qRed(rgb) < qGreen(rgb))
                minicolor = qRed(rgb);
            else
                minicolor = qGreen(rgb);

            if (minicolor > qBlue(rgb))
                minicolor = qBlue(rgb);

            temp = (maxcolor - minicolor) / 2;
            r = qRed(rgb) + temp;
            if (r > 255)
                r = 255;

            g = qGreen(rgb) + temp;
            if (g > 255)
                g = 255;

            b = qBlue(rgb) + temp;
            if (b > 255)
                b = 255;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applySharpnessEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int r;
    int g;
    int b;

    QImage tempImage = pImage;

    char mask[] = {
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    };

    tempImage = applySoftFocusEffect(tempImage);
    for(int y=0; y<tempImage.height(); y++)
    {
        for(int x=0; x<tempImage.width(); x++)
        {
            processColorFilter(tempImage,x,y,mask,&r,&g,&b);
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}

QImage AdvanceImageFilter::applySoftFocusEffect(const QImage &pImage) const
{
    QImage newImage = pImage;
    int red;
    int green;
    int blue;

    for(int y=0; y<pImage.height(); y++)
    {
        for(int x=0; x<pImage.width(); x++)
        {
            processSoftFocusFilter(newImage,x,y,&red,&green,&blue);
            newImage.setPixel(x,y,qRgb(red,green,blue));
        }
    }

    return newImage;
}

int AdvanceImageFilter::processGreenPencilFilter(const QImage &pImg, int t_x, int t_y, char *mask) const
{
    int s_x;
    int s_y;
    int green = 0;
    int m_index;
    int temp;

    for(int y=-1; y<=1; y++)
    {
        s_y = t_y + y;
        if (s_y < 0)

            s_y = 0;
        else if (s_y >= pImg.height())
            s_y = pImg.height() - 1;

        for(int x=-1; x<=1; x++)
        {
            s_x = t_x + x;
            if (s_x < 0)
                s_x = 0;
            else if (s_x >= pImg.width())
                s_x = pImg.width() - 1;

            m_index = (y + 1) * 3 + (x + 1);
            QRgb rgb = pImg.pixel(s_x,s_y);
            temp = qGreen(rgb);
            green += (temp * (*(mask + m_index)));
        }
    }

    if (green > 255)
        green = 255;

    if (green < 0)
        green *= -1;

    return green;
}

void AdvanceImageFilter::processSoftFocusFilter(const QImage &pImg, int t_x, int t_y, int *pRed, int *pGreen, int *pBlue) const
{
    int s_x;
    int s_y;
    int r = 0;
    int g = 0;
    int b = 0;

    for(int y=-1; y<=1; y++)
    {
        s_y = t_y + y;
        if (s_y < 0)
            s_y = 0;
        else if (s_y >= pImg.height())
            s_y = pImg.height() - 1;

        for(int x=-1; x<=1; x++)
        {
            s_x = t_x + x;
            if (s_x < 0)
                s_x = 0;
            else if (s_x >= pImg.width())
                s_x = pImg.width() - 1;

            QRgb rgb = pImg.pixel(s_x,s_y);
            r += qRed(rgb);
            g += qGreen(rgb);
            b += qBlue(rgb);
        }
    }

    *pRed   = r / 9;
    *pGreen = g / 9;
    *pBlue  = b / 9;
}

int AdvanceImageFilter::calcTotalAverage(const QImage &pImg) const
{
    int sum = 0;
    int avg = 0;

    for(int y=0; y<pImg.height(); y++)
    {
        for(int x=0; x<pImg.width(); x++)
        {
            QRgb rgb = pImg.pixel(x,y);
            sum += qGreen(rgb);
        }
    }
    avg = sum / (pImg.width() * pImg.height());

    return avg;
}

int AdvanceImageFilter::calcLineAverage(const QImage &pImg, int line) const
{
    int avg = 0;
    int sum = 0;

    for(int x=0; x<pImg.width(); x++)
    {
        QRgb rgb = pImg.pixel(x,line);
        sum += qGreen(rgb);
    }
    avg = sum / pImg.width();

    return avg;
}

int AdvanceImageFilter::processEnbosFilter(const QImage &pImg, int t_x, int t_y, char *mask) const
{
    int s_x;
    int s_y;
    int value = 0;
    int m_index;
    int temp;

    for(int y=-1; y<=1; y++)
    {
        s_y = t_y + y;
        if (s_y < 0)
            s_y = 0;
        else if (s_y >= pImg.height())
            s_y = pImg.height() - 1;

        for(int x=-1; x<=1; x++)
        {
            s_x = t_x + x;
            if (s_x < 0)
                s_x = 0;
            else if (s_x >= pImg.width())
                s_x = pImg.width() - 1;

            m_index = (y + 1) * 3 + (x + 1);
            QRgb rgb = pImg.pixel(s_x,s_y);
            temp = qGreen(rgb);
            value += (temp * (*(mask + m_index)));
        }
    }

    value += 127;
    if (value > 255)
        value = 255;
    else if (value < 0)
        value = 0;

    return value;
}


void AdvanceImageFilter::processColorFilter(const QImage &pImg, int t_x, int t_y, char *mask, int *pRed, int *pGreen, int *pBlue) const
{
    int s_x;
    int s_y;
    int m_index;
    int temp;

    *pRed = 0;
    *pGreen = 0;
    *pBlue = 0;

    for(int y=-1; y<=1; y++)
    {
        s_y = t_y + y;
        if (s_y < 0)
            s_y = 0;
        else if (s_y >= pImg.height())
            s_y = pImg.height() - 1;

        for(int x=-1; x<=1; x++)
        {
            s_x = t_x + x;
            if (s_x < 0)
                s_x = 0;
            else if (s_x >= pImg.width())
                s_x = pImg.width() - 1;

            m_index = (y + 1) * 3 + (x + 1);
            QRgb rgb = pImg.pixel(s_x,s_y);
            temp = qRed(rgb);
            *pRed += (temp * (*(mask + m_index)));
            temp = qGreen(rgb);
            *pGreen += (temp * (*(mask + m_index)));
            temp = qBlue(rgb);
            *pBlue += (temp * (*(mask + m_index)));
        }
    }
    if (*pRed > 255)
        *pRed = 255;

    if (*pRed < 0)
        *pRed = 0;

    if (*pGreen > 255)
        *pGreen = 255;

    if (*pGreen < 0)
        *pGreen = 0;

    if (*pBlue > 255)
        *pBlue = 255;

    if (*pBlue < 0)
        *pBlue = 0;
}

QImage AdvanceImageFilter::processContrastEffect(const QImage &pImage, int LowParam, int HighParam) const
{
    int r;
    int g;
    int b;
    QRgb rgb;

    QImage newImage = pImage;
    for(int y=0; y<newImage.height(); y++)
    {
        for(int x=0; x<newImage.width(); x++)
        {
            rgb = newImage.pixel(x,y);
            r = qRed(rgb);
            if (r < LowParam)
                r = 0;
            else if (r > (255 - HighParam))
                r = 255;
            else
                r = (int)(((double)r - (double)LowParam) * (256.0 / (256.0 - ((double)LowParam + (double)HighParam))));

            if (r > 255)
                r = 255;

            if (r < 0)
                r = 0;

            g = qGreen(rgb);
            if (g < LowParam)
                g = 0;
            else if (g > (255 - HighParam))
                g = 255;
            else
                g = (int)(((double)g - (double)LowParam) * (256.0 / (256.0 - ((double)LowParam + (double)HighParam))));

            if (g > 255)
                g = 255;

            if (g < 0)
                g = 0;

            b = qBlue(rgb);
            if (b < LowParam)
                b = 0;
            else if (b > (255 - HighParam))
                b = 255;
            else
                b = (int)(((double)b - (double)LowParam) * (256.0 / (256.0 - ((double)LowParam + (double)HighParam))));

            if (b > 255)
                b = 255;

            if (b < 0)
                b = 0;

            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }

    return newImage;
}
