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
    property alias imageSource: optionsImage.source
    property alias toolTipText: toolTip.text
    property int widthWithoutTooltip: 40
    property bool enableTooltip: false

    signal showToolTip ( string toolTip )
    signal hideToolTip
    signal clicked

    id: optionsButton
    width: widthWithoutTooltip
    height: 40
    color: "transparent"

    Image {
        id: optionsImage
        anchors.left: parent.left
        width: widthWithoutTooltip
        height: 40
        anchors.verticalCenter: parent.verticalCenter
        source: "qrc:///Icons/cancel-download.png"
    }

    Rectangle {
        id: toolTipRect
        color: "#f8e7c7"
        anchors.left: optionsImage.right
        visible: false
        width: toolTip.width
        height: optionsImage.height
        anchors.verticalCenter: parent.verticalCenter
        radius: 8

        Text {
            id: toolTip
            text: "default"
            anchors.centerIn: parent
            // font.bold: true
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            if(!enableTooltip)
                return;

            parent.width = widthWithoutTooltip + toolTipRect.width;
            toolTipRect.visible = true;
        }

        onExited: {
            parent.width = widthWithoutTooltip;
            toolTipRect.visible = false;
        }

        onClicked: {
            optionsButton.clicked();
        }
    }

    Behavior on width { NumberAnimation { duration: 200; } }
}
