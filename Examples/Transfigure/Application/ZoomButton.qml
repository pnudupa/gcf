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
    id: zoomButton
    border { width: 2; color: "gray" }
    radius: 8
    color: zoomButtonArea.pressed ? "white" : Qt.rgba(1,1,1,0.4)

    signal clicked()
    property alias text: textLabel.text

    Text {
        id: textLabel
        anchors.centerIn: parent
        font.pixelSize: parent.height*0.7
        font.bold: true
        color: zoomButtonArea.pressed ? "black" : "white"
        text: "+"
    }

    MouseArea {
        id: zoomButtonArea
        anchors.fill: parent
        onClicked: zoomButton.clicked()
    }
}
