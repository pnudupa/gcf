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
    id: board
    width: 500
    height: 500

    property int tile_NONE: 0
    property int tile_DOT: 1
    property int tile_CROSS: 2
    property var ticTacToe: gcf.findObject("Application.TicTacToe")

    gradient: Gradient {
        GradientStop { position: 0; color: "white" }
        GradientStop { position: 1; color: "aliceblue" }
    }

    Component {
        id: dot

        Item {
            property color color: "red"

            Rectangle {
                width: 30; height: 30
                anchors.centerIn: parent
                color: parent.color
                radius: width/2
            }
        }
    }

    Component {
        id: cross

        Item {
            property real diag: (Math.sqrt(width*width + height*height)) * 0.5
            property color color: "blue"

            Rectangle {
                width: parent.diag; height: 7
                anchors.centerIn: parent
                transformOrigin: Item.Center
                rotation: 45
                color: parent.color
            }

            Rectangle {
                width: parent.diag; height: 7
                anchors.centerIn: parent
                transformOrigin: Item.Center
                rotation: -45
                color: parent.color
            }
        }
    }

    Item {
        id: gridArea
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 3
        width: parent.width-40
        height: parent.height-40
        clip: true

        Grid {
            id: grid
            rows: 3; columns: 3
            anchors.fill: parent
            anchors.margins: -3

            Repeater {
                model: ticTacToe

                Rectangle {
                    id: square
                    width: grid.width/3
                    height: grid.height/3
                    border { width: 3; color: "black" }
                    color: Qt.rgba(0,0,0,0)

                    property Item marker: null
                    property int tileType: tile
                    onTileTypeChanged: {
                        if(marker) {
                            marker.visible = false
                            marker.destroy()
                        }

                        if( tileType === board.tile_DOT )
                            marker = dot.createObject(square)
                        else if(tileType === board.tile_CROSS)
                            marker = cross.createObject(square)
                        else
                            marker = null

                        if(marker) {
                            marker.anchors.fill = square
                            marker.anchors.margins = 10
                            marker.visible = true
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: ticTacToe.cross(index)
                    }
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "white"
            opacity: 0.75
            visible: (ticTacToe.enabled === false)

            MouseArea {
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        id: statusBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: gridArea.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: 5
        color: "lightsteelblue"
        opacity: 0.5
    }

    Text {
        anchors.fill: statusBar
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: ticTacToe.status
    }

    Rectangle {
        id: gameComplete
        anchors.fill: parent
        visible: false
        color: Qt.rgba(1,1,1,0.75)

        property string text

        Rectangle {
            height: 80
            width: parent.width
            anchors.centerIn: parent
            color: "mediumblue"

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.verticalCenter
                anchors.bottomMargin: 5
                text: gameComplete.text
                color: "white"
            }

            Rectangle {
                border { width: 1; color: "black" }
                color: "white"
                width: 120; height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.verticalCenter
                anchors.topMargin: 5

                Text {
                    anchors.centerIn: parent
                    text: "Okay"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        ticTacToe.clear()
                        gameComplete.visible = false
                    }
                }
            }
        }
    }

    function onGameComplete(msg) {
        gameComplete.text = msg
        gameComplete.visible = true
    }

    onTicTacToeChanged: ticTacToe.gameComplete.connect(onGameComplete)
}
