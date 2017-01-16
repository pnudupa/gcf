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

#ifndef CALENDARCOMPONENT_H
#define CALENDARCOMPONENT_H

#include <GCF3/GuiComponent>
#include <QCalendarWidget>

class CalendarComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    CalendarComponent(QObject *parent=0) : GCF::GuiComponent(parent) { }

protected:
    ~CalendarComponent() { }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/SimpleGUI/CalendarComponent.xml");
        GCF::Component::contentLoadEvent(e);
    }

    QWidget *loadWidget(const QString &name, const QVariantMap &info) {
        if(name == "Calendar")
            return new QCalendarWidget;
        return GCF::GuiComponent::loadWidget(name, info);
    }
};


#endif // CALENDARCOMPONENT_H
