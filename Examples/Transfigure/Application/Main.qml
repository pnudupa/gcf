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
import QtQuick.Dialogs 1.0

Rectangle {
    id: window
    width: 1024; height: 768
    color: Qt.lighter("#272727")

    property string imageFileName
    property int imageFilterIndex: -1

    FileDialog {
        id: fileDialog
        onAccepted: {
            window.imageFileName = fileDialog.fileUrl
            imageFlick.zoomFit()
        }
    }

    Rectangle {
        id: imageArea
        anchors.left: parent.left
        anchors.right: filtersListArea.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 20
        border { width: 2; color: "gray" }
        radius: 6
        color: Qt.rgba(0, 0, 0, 0)

        Flickable {
            id: imageFlick
            anchors.fill: parent
            anchors.margins: 10
            contentWidth: imageItem.width
            contentHeight: imageItem.height
            clip: true

            property real imageFlickSize: Math.max(width, height)

            function zoomIn() {
                imageItem.imageScale = imageItem.imageScale*1.1
            }

            function zoomOut() {
                imageItem.imageScale = imageItem.imageScale*0.9
            }

            function zoomFit() {
                imageItem.imageScale = imageFlickSize / imageItem.imageSize
            }

            Item {
                id: imageItem

                property real imageSize: Math.max(image.implicitWidth, image.implicitHeight)
                property real imageScale: 1
                Behavior on imageScale {
                    NumberAnimation { duration: 200 }
                }

                width: image.implicitWidth * imageScale
                height: image.implicitHeight * imageScale

                Image {
                    id: image
                    anchors.fill: parent
                    smooth: true
                    source: {
                        if(window.imageFileName.length)
                            return "image://filteredImage/" + window.imageFileName + "?" + window.imageFilterIndex
                        return ""
                    }
                }
            }
        }

        Item {
            id: zoomControls
            visible: window.imageFileName.length > 0
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 20
            height: 140
            width: 40
            opacity: zoomControlsMouseArea.containsMouse ? 1.0 : 0.5

            MouseArea {
                id: zoomControlsMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }

            ZoomButton {
                width: parent.width; height: width
                anchors.top: parent.top
                text: "+"
                onClicked: imageFlick.zoomIn()
            }

            ZoomButton {
                width: parent.width; height: width
                anchors.centerIn: parent
                text: "O"
                onClicked: imageFlick.zoomFit()
            }

            ZoomButton {
                width: parent.width; height: width
                anchors.bottom: parent.bottom
                text: "-"
                onClicked: imageFlick.zoomOut()
            }
        }

        Text {
            visible: window.imageFileName.length === 0
            text: "Please load an image file and apply filters on the right"
            anchors.centerIn: parent
            color: "white"
        }
    }

    Rectangle {
        id: filtersListArea
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Math.min(parent.width*0.25, 200)
        anchors.margins: 20
        border { width: 2; color: "gray" }
        radius: 6
        color: Qt.rgba(0, 0, 0, 0)

        ListView {
            id: filtersList
            anchors.fill: parent
            anchors.margins: 2
            clip: true

            model: filtersModel

            header: Button {
                width: filtersList.width
                text: "Open"
                onClicked: fileDialog.visible = true
            }

            delegate: Button {
                width: filtersList.width
                text: filterName
                onClicked: window.imageFilterIndex = index
            }

            section.property: "filterGroup"
            section.criteria: ViewSection.FullString
            section.delegate: Item {
                width: filtersList.width
                height: 40

                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: parent.height-10
                    border { width: 2; color: "gray" }
                    // color: Qt.darker("gray")
                    color: "#002147"

                    Text {
                        anchors.centerIn: parent
                        text: section
                        color: "white"
                    }
                }
            }
        }
    }
}
