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

import QtQuick 2.0

Rectangle {
    width: 100
    height: 62

    property string indicator: "Indicator1"
    property var a
    property var b
    property var c
    property var d

    property bool active: gComponent.active
    onActiveChanged: {
        if(active)
            gComponent.setString("active")
        else
            gComponent.setString("inactive")
    }

    Component.onCompleted: {
        a = gcf.findObject("Application.QmlComponent2.a")
        b = gcf.findObject("Application.QmlComponent2.b")
        c = gcf.findObject("Application.QmlComponent2.c")
        d = gcf.findObject("Application.QmlComponent2.d")
    }
}
