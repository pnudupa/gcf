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

#include <QtGui/QGuiApplication>
#include <QQuickView>

#include "FiltersModel.h"
#include "FilteredImageProvider.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <GCF3/QmlApplication>

int main(int argc, char *argv[])
{
    GCF::QmlApplication a(argc, argv);
    a.processArguments();

    FiltersModel filtersModel;

    QQuickView qmlView;
    qmlView.setTitle("Transfigure - GCFQuick3 Example");
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView.engine()->rootContext()->setContextProperty("filtersModel", &filtersModel);
    qmlView.engine()->addImageProvider("filteredImage", new FilteredImageProvider(&filtersModel));
    qmlView.setSource(QUrl("qrc:///Qml/Main.qml"));
    qmlView.show();

    return a.processArgumentsAndExec();
}
