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

#ifndef SPIRALFLOWCOMPONENT_H
#define SPIRALFLOWCOMPONENT_H

#include <GCF3/GuiComponent>

struct SpiralFlowComponentData;
class SpiralFlowComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    SpiralFlowComponent();

protected:
    ~SpiralFlowComponent();
    void contentLoadEvent(GCF::ContentLoadEvent *e);
    QObject *loadObject(const QString &name, const QVariantMap &info);

private:
    SpiralFlowComponentData *d;
};

#endif // SPIRALFLOWCOMPONENT_H
