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

import QtQuick 1.1

Rectangle {
    width: 640
    height: 480

    gradient: Gradient {
        GradientStop { position: 0; color: "white" }
        GradientStop { position: 1; color: "lightgray" }
    }

    Column {
        id: column1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        Row {
            Text {
                font.pixelSize: 22
                text: "Item: "
            }

            Text {
                font.pixelSize: 22
                color: "blue"
                text: sharingItemTitle
            }
        }

        Item { width: parent.width; height: 10 }

        Text {
            text: "Link to share (only accessible by collaborators)"
        }

        Rectangle {
            width: parent.width
            height: txtShareLink.implicitHeight + 16
            border { width: 1; color: "black" }
            color: "white"
            clip: true
            TextInput {
                id: txtShareLink
                text: sharingModel.shareLink()
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                cursorPosition: 0
                readOnly: true
            }
        }

        Item { width: parent.width; height: 10 }
    }

    Rectangle {
        anchors.top: column1.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        color: Qt.rgba(1, 1, 1, 0.4)
        border { width: 1; color: "black" }
        radius: 6

        Rectangle {
            border { width: 1; color: "black" }
            width: whoHasAccessLabel.implicitWidth * 1.2
            height: whoHasAccessLabel.implicitHeight * 1.2
            color: "white"
            anchors.top: parent.top
            anchors.topMargin: -height/2
            anchors.left: parent.left
            anchors.leftMargin: 15

            Text {
                id: whoHasAccessLabel
                text: "Who has access"
                anchors.centerIn: parent
            }
        }

        Flickable {
            id: gridViewFlickable
            anchors.fill: parent
            anchors.margins: 10
            anchors.topMargin: 15
            contentWidth: width
            contentHeight: gridView.implicitHeight
            clip: true

            Grid {
                id: gridView
                clip: true
                spacing: 5
                width: gridViewFlickable.width
                columns: 3

                property int itemWidth: (width / columns)-spacing
                property int itemHeight: 100

                Repeater {
                    model: sharingModel
                    delegate: Rectangle {
                        width: gridView.itemWidth
                        height: gridView.itemHeight
                        color: Qt.rgba(0,0,0,0)
                        border { width: 1; color: "black" }
                        radius: 4
                        clip: true

                        Column {
                            id: column
                            spacing: 5
                            anchors.fill: parent
                            anchors.rightMargin: 30
                            anchors.margins: 5

                            Text {
                                font.pixelSize: 16
                                text: shareTitle
                                width: parent.width
                                wrapMode: Text.WordWrap
                            }

                            Text {
                                text: shareRoles
                                font.underline: index > 0
                                font.bold: true
                                color: index === 0 ? "black" : "#0066FF"
                                width: parent.width
                                wrapMode: Text.WordWrap

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: index > 0
                                    onClicked: shrApp.updateSharePermisssion(sharingModel, index)
                                }
                            }
                        }

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: column.right
                            anchors.leftMargin: 10
                            source: "qrc:///unshare.png"
                            visible: index > 0 // index === 0 corresponds to the owner / authenticated user.

                            MouseArea {
                                anchors.fill: parent
                                onClicked: shrApp.removeSharePermission(sharingModel, index)
                            }
                        }
                    }
                }

                Rectangle {
                    width: gridView.itemWidth
                    height: gridView.itemHeight
                    border { width: 1; color: "black" }
                    radius: 6
                    opacity: shareButtonMouseArea.containsMouse ? 1 : 0.5
                    color: shareButtonMouseArea.pressed ? Qt.lighter("#0066FF") : "#0066FF"

                    Text {
                        id: shareButtonLabel
                        anchors.centerIn: parent
                        text: "+"
                        color: "white"
                        font.bold: true
                        font.pixelSize: parent.height * 0.5
                    }

                    MouseArea {
                        id: shareButtonMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        onClicked: shrApp.addSharePermission(sharingModel)
                    }
                }
            }
        }
    }
}
