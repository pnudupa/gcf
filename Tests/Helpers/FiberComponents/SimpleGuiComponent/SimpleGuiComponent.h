#ifndef APPWINDOW_H
#define APPWINDOW_H

#include "TestService.h"

#include <QWidget>
#include <GCF3/GuiComponent>

class SimpleGuiComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    SimpleGuiComponent(QObject *parent = 0)
        :GCF::GuiComponent(parent) {
        m_window = 0;
        m_testService = 0;
    }
    
    Q_INVOKABLE void dummyService() { }

protected:
    void initializeEvent(GCF::InitializeEvent *e) {
        if(e->isPreInitialize())
            return;

        GCF::ObjectTreeNode *node = gAppService->objectTree()->node(m_testService);
        if(node)
            node->writableInfo()["allowmetaaccess"] = true;
    }

    void contentLoadEvent(GCF::ContentLoadEvent *e) {
        if(e->isPreContentLoad())
            e->setContentFile(":/Content/SimpleGuiComponent.xml");
        GCF::GuiComponent::contentLoadEvent(e);
    }

    QWidget *loadWidget(const QString &name, const QVariantMap &info) {
        QWidget *w = GCF::GuiComponent::loadWidget(name, info);
        if(name == "Window")
            m_window = w;

        return w;
    }

    QObject *loadObject(const QString &name, const QVariantMap &) {
        if(name == "Core")
            return this;

        if(name == "TestService") {
            if(!m_testService)
                m_testService = new TestService(this);

            return m_testService;
        }

        return 0;
    }

    void activationEvent(GCF::ActivationEvent *) {
        // Don't do something like this at the server

        /*
        if(e->isPostActivation())
            m_window->show();
        */
    }

private:
    QWidget *m_window;
    TestService *m_testService;
};

#endif // APPWINDOW_H
