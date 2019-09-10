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

#include <GCF3/QmlApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include "Klippie.h"

int main(int argc, char **argv)
{
    GCF::QmlApplication a(argc, argv);

    Klippie klippie;

    QQuickView qmlView;
    a.setQmlEngine(qmlView.engine());
    qmlView.setTitle("Klippie - Your Shared Clipboard");
    qmlView.engine()->rootContext()->setContextProperty("klippieModel", klippie.model());
    qmlView.engine()->rootContext()->setContextProperty("klippie", &klippie);
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView.setSource(QUrl("qrc:/Klippie.qml"));
    qmlView.show();

    return a.exec();
}
