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

#ifndef TEXTVIEWERCOMPONENT_H
#define TEXTVIEWERCOMPONENT_H

#include <GCF3/GuiComponent>
#include <QWidget>

struct TextViewerComponentData;
class TextViewerComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    TextViewerComponent(QObject *parent=0);

protected:
    // Destructor is protected, because we want components
    // to be destroyed only via finalize() method and
    // we dont want instanes of this component to be created
    // on the stack.
    ~TextViewerComponent();

    void contentLoadEvent(GCF::ContentLoadEvent *e);
    QWidget *loadWidget(const QString &name, const QVariantMap &info);

    /*
     * At the post-activate event, we simply show the window
     * of this component.
     */
    void activationEvent(GCF::ActivationEvent *e);

public slots:
    void openFile(const QString &fileName);

private:
    TextViewerComponentData *d;
};

#endif // TEXTEDITORWINDOW_H
