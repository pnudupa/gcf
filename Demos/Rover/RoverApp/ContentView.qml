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
    id: contentView
    property int currentViewIndex: -1
    border { width: 1; color: "gray" }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle {
            id: toolBarArea
            width: parent.width
            height: 50
            border { width: 1; color: "black" }
            radius: 8

            Row {
                id: toolBar
                spacing: 5
                anchors.centerIn: parent

                Repeater {
                    model: contentItemModel
                    Rectangle {
                        width: 100
                        height: 30
                        color: contentView.currentViewIndex === index ? "lightsteelblue" : "white"
                        radius: 8

                        Text {
                            id: contentTitleLabel
                            anchors.centerIn: parent
                            text: contentTitle
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: contentView.currentViewIndex = index
                        }
                    }
                }
            }
        }

        Rectangle {
            id: workspace
            width: parent.width
            height: parent.height - toolBarArea.height - parent.spacing
            border { width: 1; color: "black" }
            radius: 8

            ListView {
                id: workspaceList
                anchors.fill: parent
                model: contentItemModel
                // snapMode: ListView.SnapToItem
                orientation: ListView.Horizontal
                boundsBehavior: Flickable.StopAtBounds
                interactive: false
                highlightFollowsCurrentItem: true
                highlightMoveDuration: 10
                highlightRangeMode: ListView.ApplyRange
                currentIndex: contentView.currentViewIndex
                clip: true

                delegate: Rectangle {
                    id: workspaceListItem
                    width: workspaceList.width
                    height: workspaceList.height
                    color: Qt.rgba(0,0,0,0)
                    clip: true

                    Component.onCompleted: {
                        contentItem.parent = workspaceListItem
                        contentItem.anchors.fill = workspaceListItem
                        contentItem.anchors.margins = 5
                        contentItem.visible = true
                    }

                    Component.onDestruction: {
                        if(contentItem) {
                            contentItem.parent = workspace
                            contentItem.visible = false
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        contentView.currentViewIndex = 0;
    }
}
