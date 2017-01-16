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

#ifndef ADVANCEIMAGEFILTER_H
#define ADVANCEIMAGEFILTER_H

#include <QObject>
#include "IImageFilter.h"

class AdvanceImageFilter : public QObject, public IImageFilter
{
    Q_OBJECT
    Q_INTERFACES(IImageFilter)

public:
    AdvanceImageFilter(QObject *parent = 0);
    ~AdvanceImageFilter();

    // IImageFilter implementation
    QStringList effectNames() const;
    QImage applyFilterEffect(const QString &, const QImage &) const;

private:
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
};

#endif // ADVANCEIMAGEFILTER_H
