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
    width: 100
    height: 62

    PathView {
        id: coverFlowView
        model: folderContentModel
        anchors.fill: parent
        anchors.margins: 10
        currentIndex: -1
        pathItemCount: 6
        clip: true

        Keys.onLeftPressed: coverFlowView.decrementCurrentIndex()
        Keys.onRightPressed: coverFlowView.incrementCurrentIndex()

        onActiveFocusChanged: {
            if(!coverFlowView.currentIndex)
                currentIndex = -1
        }

        delegate: Item {
            id: coverFlowViewItem
            width: coverFlowView.width*0.65
            height: coverFlowView.height*0.65
            scale: PathView.itemScale === undefined? 0 : PathView.itemScale
            opacity: PathView.itemOpacity === undefined? 0 : PathView.itemOpacity
            z: PathView.itemZ === undefined? 0 : PathView.itemZ
            property real angle: PathView.itemAngle === undefined? 0 : PathView.itemAngle

            transform: Rotation {
                origin.x: coverFlowViewItem.width/2
                origin.y: coverFlowViewItem.height/2
                angle: coverFlowViewItem.angle
                axis { x: 0; y: 1; z: 0 }
            }

            Image {
                asynchronous: true
                source: fileThumbnail
                width: parent.width
                height: parent.height
                fillMode: Image.PreserveAspectFit
                smooth: true
                sourceSize.width: 1024
                sourceSize.height: 1024
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: fileName
                font.pixelSize: 14
                anchors.bottom: parent.bottom
            }
        }

        path: Path {
            // "Start zone"
            startX: -25
            startY: coverFlowView.height / 2
            PathAttribute { name: "itemZ"; value: 0 }
            PathAttribute { name: "itemAngle"; value: 70 }
            PathAttribute { name: "itemScale"; value: 0.25 }
            PathAttribute { value: 0.3; name: "itemOpacity" }

            // Just before middle
            PathLine { x: coverFlowView.width * 0.35; y: coverFlowView.height / 2;  }
            PathAttribute { name: "itemZ"; value: 50 }
            PathAttribute { name: "itemAngle"; value: 45 }
            PathAttribute { name: "itemScale"; value: 0.55 }
            PathAttribute { value: 0.6; name: "itemOpacity" }
            PathPercent { value: 0.40 }

            // Middle
            PathLine { x: coverFlowView.width * 0.5; y: coverFlowView.height / 2;  }
            PathAttribute { name: "itemZ"; value: 100 }
            PathAttribute { name: "itemAngle"; value: 0 }
            PathAttribute { name: "itemScale"; value: 1.0 }
            PathAttribute { value: 1; name: "itemOpacity" }

            // Just after middle
            PathLine { x: coverFlowView.width * 0.65; y: coverFlowView.height / 2; }
            PathAttribute { name: "itemZ"; value: 50 }
            PathAttribute { name: "itemAngle"; value: -45 }
            PathAttribute { name: "itemScale"; value: 0.55 }
            PathAttribute { value: 0.6; name: "itemOpacity" }
            PathPercent { value: 0.60 }

            // Final stop
            PathLine { x: coverFlowView.width + 25; y: coverFlowView.height / 2; }
            PathAttribute { name: "itemZ"; value: 0 }
            PathAttribute { name: "itemAngle"; value: -70 }
            PathAttribute { name: "itemScale"; value: 0.25 }
            PathAttribute { value: 0.3; name: "itemOpacity" }
            PathPercent { value: 1.0 }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if(!coverFlowView.activeFocus) {
                    coverFlowView.forceActiveFocus()
                }
            }
        }
    }
}
