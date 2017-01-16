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

#ifndef APPWINDOWCOMPONENT_H
#define APPWINDOWCOMPONENT_H

#include <GCF3/GuiComponent>
#include <QWidget>

class AppWindowComponent : public GCF::GuiComponent
{
public:
    AppWindowComponent(QObject *parent=0) : GCF::GuiComponent(parent), m_window(0) { }

    QString name() const { return "AppWindowComponent"; }

protected:
    // Destructor is protected, because we want components
    // to be destroyed only via finalize() method and
    // we dont want instanes of this component to be created
    // on the stack.
    ~AppWindowComponent() { }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/SimpleGUI/AppWindowComponent.xml");
        GCF::GuiComponent::contentLoadEvent(e);
    }

    QWidget *loadWidget(const QString &name, const QVariantMap &info) {
        QWidget *w = GCF::GuiComponent::loadWidget(name, info);
        if(name == "Window")
            m_window = w;
        return w;
    }

    /*
     * At the post-activate event, we simply show the window
     * of this component.
     */
    void activationEvent(GCF::ActivationEvent *e) {
        if(e->isPostActivation())
            m_window->show();
    }

private:
    QWidget *m_window;
};

#endif // APPWINDOWCOMPONENT_H
