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
    width: 500
    height: 400

    gradient: Gradient {
        GradientStop { position: 0; color: "white" }
        GradientStop { position: 1; color: "lightgray" }
    }

    Text {
        anchors.fill: parent
        anchors.margins: 10
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 36
        text: "No uploads or downloads"
        color: "gray"
        visible: listView.count === 0
    }

    Rectangle {
        anchors.fill: listView
        anchors.margins: -4
        radius: 5
        border { width: 1.5; color: "gray" }
        color: Qt.rgba(0,0,0,0)
        visible: listView.visible
    }

    ListView {
        id: listView
        visible: count > 0
        anchors.fill: parent
        clip: true
        anchors.margins: 10
        anchors.bottomMargin: 60

        model: jobsModel
        spacing: 4

        section.criteria: ViewSection.FullString
        section.property: "jobKind"
        section.delegate: Rectangle {
            width: listView.width - 1
            height: sectionNameLabel.implicitHeight * 1.2
            color: "lightgray"
            border.width: 1; border.color: "black"
            Text {
                id: sectionNameLabel
                anchors.centerIn: parent
                text: section
            }
        }

        delegate: Rectangle {
            width: listView.width-1
            height: 50
            color: listView.currentIndex === index ? "lightsteelblue" : Qt.rgba(0,0,0,0)
            radius: 2

            MouseArea {
                anchors.fill: parent
                onClicked: listView.currentIndex = index
            }

            Item {
                anchors.fill: parent
                anchors.margins: 5

                Image {
                    id: iconImage
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    source: jobIconUrl
                    smooth: true
                }

                Column {
                    anchors.left: iconImage.right
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    Text {
                        id: fileNameLabel
                        font.pixelSize: 14
                        text: jobTitle
                        width: parent.width
                        elide: Text.ElideMiddle
                    }

                    Item {
                        width: parent.width*0.9
                        height: 20
                        visible: jobIsRunning

                        // Progress-bar
                        Rectangle {
                            width: parent.width - 40
                            height: parent.height
                            border { width: 1; color: "black" }

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.margins: 1
                                color: "green"
                                width: parent.width * jobProgress / 100.0
                            }

                            Text {
                                anchors.centerIn: parent
                                text: jobProgress + " %"
                            }
                        }

                        // Cancel button
                        Image {
                            width: 16; height: 16
                            source: "qrc:///cancel-download.png"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 2

                            MouseArea {
                                anchors.fill: parent
                                onClicked: jobsModel.cancelJobAt(index)
                            }
                        }
                    }

                    Text {
                        visible: jobIsComplete
                        color: jobHasError ? "red" : "blue"
                        font.pixelSize: 10
                        elide: Text.ElideMiddle
                        width: parent.width
                        text: {
                            if(jobHasError)
                                return "ERROR: " + jobError
                            if(jobKind === "GDrive Downloads")
                                return "Downloaded as <u>" + jobObject.downloadedFileName + "</u>"
                            return "Uploaded <u>" + jobObject.uploadFileName + "</u>"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if(!jobHasError) {
                                    if(jobKind === "GDrive Downloads")
                                        jobsApp.showFileLocation(jobObject.downloadedFileName);
                                    else
                                        jobsApp.showFileLocation(jobObject.uploadFileName);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: clearButton
        visible: listView.count > 0
        anchors.right: listView.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: 120; height: 30
        border { color: "black"; width: 1 }
        color: clearButtonMouseArea.pressed ? "gray" : "white"

        Text {
            anchors.centerIn: parent
            text: "Clear"
            color: "black"
        }

        MouseArea {
            id: clearButtonMouseArea
            anchors.fill: parent
            onClicked: jobsModel.clearCompletedJobs()
        }
    }
}
