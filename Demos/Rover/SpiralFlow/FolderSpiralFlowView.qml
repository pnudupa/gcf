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
    Rectangle{
        anchors.fill: parent
        color: "white"

        PathView {
            id: spiralFlowView
            model: folderContentModel
            width : parent.width > 575? 575 : (parent.width-20)
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            currentIndex: -1
            pathItemCount: 30
            clip: true

            Keys.onRightPressed: spiralFlowView.decrementCurrentIndex()
            Keys.onLeftPressed: spiralFlowView.incrementCurrentIndex()

            onActiveFocusChanged: {
                if(!spiralFlowView.currentIndex)
                    currentIndex = -1
            }

            delegate: Item {
                id: spiralFlowViewItem
                width: spiralFlowView.width*0.65
                height: spiralFlowView.height*0.65
                scale:  PathView.itemScale === undefined? 0 : PathView.itemScale
                opacity: PathView.itemOpacity === undefined? 0 : PathView.itemOpacity
                z: PathView.itemZ === undefined? 0 : PathView.itemZ
                property real angle: PathView.itemAngle === undefined? 0 : PathView.itemAngle

                transform: Rotation {
                    origin.x: spiralFlowViewItem.width/2
                    origin.y: spiralFlowViewItem.height/2
                    angle: spiralFlowViewItem.angle
                    axis { x: 0; y: 1; z: 0 }
                }

                Image {
                    asynchronous: true
                    source: {
                        if(fileThumbnail.indexOf("qrc")>= 0)
                            return fileThumbnail
                        return "image://thumbnail/"+filePath
                    }
                    width: parent.width
                    height: parent.height
                    smooth: true
                    fillMode: Image.PreserveAspectFit
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
                PathAttribute { name: "itemScale"; value: 0.25 }
                startX: spiralFlowView.width
                startY: -spiralFlowView.height*0.2

                PathCurve { x: spiralFlowView.width; y: -spiralFlowView.height*0.2 }

                PathCurve { x: spiralFlowView.width*0.5; y: 0 }
                PathAttribute { name: "itemOpacity"; value: 0.1 }
                PathAttribute { name: "itemZ"; value: 0 }

                PathCurve { x: 0; y: spiralFlowView.height*0.2 }
                PathAttribute { name: "itemAngle"; value: 90 }

                PathCurve { x: spiralFlowView.width*0.5; y: spiralFlowView.height*0.3 }
                PathAttribute { name: "itemOpacity"; value: 1 }
                PathAttribute { name: "itemZ"; value: 50 }

                PathCurve { x: spiralFlowView.width; y: spiralFlowView.height*0.4 }
                PathAttribute { name: "itemAngle"; value: 270 }

                PathCurve { x: spiralFlowView.width*0.5; y: spiralFlowView.height*0.5 }
                PathAttribute { name: "itemOpacity"; value: 0.1; }
                PathAttribute { name: "itemZ"; value: 0 }

                PathCurve { x: 0; y: spiralFlowView.height*0.6 }
                PathAttribute { name: "itemAngle"; value: 450 }

                PathCurve { x: spiralFlowView.width*0.5; y: spiralFlowView.height*0.7 }
                PathAttribute { name: "itemOpacity";  value: 1; }
                PathAttribute { name: "itemZ"; value: 50 }

                PathCurve { x: spiralFlowView.width; y: spiralFlowView.height*0.8 }
                PathAttribute { name: "itemAngle"; value: 630 }

                PathCurve { x: spiralFlowView.width*0.5; y: spiralFlowView.height*0.9 }
                PathAttribute { name: "itemOpacity"; value: 0.1; }
                PathAttribute { name: "itemZ"; value: 0 }

                PathCurve { x: 0; y: spiralFlowView.height }
                PathAttribute { name: "itemAngle"; value: 810 }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(!spiralFlowView.activeFocus) {
                        spiralFlowView.forceActiveFocus()
                    }
                }
            }
        }
    }
}
