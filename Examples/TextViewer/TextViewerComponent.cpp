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

#include "TextViewerComponent.h"

#include <QFile>
#include <QAction>
#include <QTextEdit>
#include <QHBoxLayout>

struct TextViewerComponentData
{
    TextViewerComponentData():
        window(0),
        textEdit(0) {}

    QWidget *window;
    QTextEdit *textEdit;
};

TextViewerComponent::TextViewerComponent(QObject *parent) :
     GCF::GuiComponent(parent)
{
    d = new TextViewerComponentData;
}

TextViewerComponent::~TextViewerComponent()
{
    delete d;
}

void TextViewerComponent::contentLoadEvent(GCF::ContentLoadEvent *e)
{
    if(e->isPreContentLoad())
        e->setContentFile(":/TextViewerComponent/TextViewerComponent.xml");

    GCF::GuiComponent::contentLoadEvent(e);
}

QWidget *TextViewerComponent::loadWidget(const QString &name, const QVariantMap &info)
{
    Q_UNUSED(info);

    if(name == "TextEdit")
    {
        d->textEdit = new QTextEdit;
#ifdef Q_OS_MAC
        d->textEdit->setFont(QFont("Monaco", 12));
#else
        d->textEdit->setFont(QFont("Courier New", 12));
#endif
        d->textEdit->setWordWrapMode(QTextOption::NoWrap);
        return d->textEdit;
    }

    QWidget *w = GCF::GuiComponent::loadWidget(name, info);
    if(name == "Window")
        d->window = w;

    return w;
}

void TextViewerComponent::activationEvent(GCF::ActivationEvent *e)
{
    if(e->isPostActivation() && d->window)
        d->window->show();
}

void TextViewerComponent::openFile(const QString &fileName)
{
    d->textEdit->clear();

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
        d->textEdit->setText(file.readAll());
}
