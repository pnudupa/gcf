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
    property bool checkedStatus: false
    property string text: ""

    id: radioButton

    Rectangle {
        id: radioButtonUiRect
        width: 20
        height: width
        radius: width/2
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        border { width: 1; color: "black" }
        color: Qt.rgba(1,1,1,0.8)

        Rectangle {
            width: parent.width/2
            height: width
            radius: width/2
            anchors.centerIn: parent
            color: checkedStatus ? Qt.rgba(0,0,0,0.8) : Qt.rgba(1,1,1,0.8)
        }
    }

    Text {
        id: radioButtonText
        text: radioButton.text
        anchors.left: radioButtonUiRect.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
    }

    MouseArea {
        anchors.fill: parent
        onClicked: checkedStatus = !checkedStatus;
    }
}
