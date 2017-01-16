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

Item {
    id: buttonComponent
    property alias text: buttonTextLabel.text
    signal clicked()

    width: 150; height: 50

    Rectangle {
        id: button
        anchors.fill: parent
        anchors.margins: 4
        anchors.bottomMargin: 10
        border { width: 2; color: "gray" }
        radius: 6

        gradient: Gradient {
            GradientStop { position: 0; color: buttonMouseArea.pressed ? "black" : "darkgray" }
            GradientStop { position: 0.1; color: buttonMouseArea.pressed ? "black" : "gray" }
            GradientStop { position: 1; color: "black" }
        }

        Text {
            id: buttonTextLabel
            text: "Default"
            color: "white"
            anchors.fill: parent
            anchors.leftMargin: buttonMouseArea.pressed ? 2 : 0
            anchors.topMargin: buttonMouseArea.pressed ? 2 : 0
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: parent.height*0.4
        }

        MouseArea {
            id: buttonMouseArea
            anchors.fill: parent
            onClicked: buttonComponent.clicked()
        }
    }
}

