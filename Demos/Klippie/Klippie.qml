import QtQuick 2.6
import QtQuick.Controls 2.12

Item {
    width: 480
    height: 640

    ListView {
        id: klippieView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: buttonsRow.top
        anchors.margins: 10
        clip: true
        model: klippieModel
        highlight: Rectangle {
            color: "lightsteelblue"
            radius: 5
        }
        highlightMoveDuration: 50
        delegate: Item {
            width: klippieView.width-1
            height: klippieItemRow.height*1.11

            Rectangle {
                anchors.fill: parent
                anchors.margins: 3
                radius: 5
                border { width: 0.5; color: "gray" }
                color: Qt.rgba(0,0,0,0)
            }

            Column {
                id: klippieItemRow
                width: parent.width*0.9
                anchors.centerIn: parent

                spacing: 10

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    text: content.formats.join(", ");
                    font.pixelSize: 14
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    maximumLineCount: 4
                    elide: Text.ElideNone
                    font.pixelSize: 11
                    textFormat: Text.RichText
                    text: {
                        if( content["text/plain"] )
                            return content["text/plain"]
                        if( content["text/html"] )
                            return content["text/html"]
                        if( content["text/uri-list"] )
                            return content["text/uri-list"]
                        return "[[ Binary Data ]]"
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: klippieView.currentIndex = index
                onDoubleClicked: useButton.click()
            }
        }
    }

    Row {
        id: buttonsRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: 10
        spacing: 10

        Button {
            id: syncButton
            text: "Sync"
            onClicked: klippieModel.sync()
        }

        Button {
            id: useButton
            text: "Use"
            onClicked: click()

            function click() {
                klippieModel.use(klippieView.currentIndex)
            }
        }
    }

    Connections {
        target: klippieModel
        onSizeChanged: klippieView.currentIndex = -1
    }

    Connections {
        target: klippie
        onNotice: notificationBar.noticeMessage = message
    }

    Loader {
        id: notificationBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        height: 80

        property string noticeMessage
        active: noticeMessage !== ""

        sourceComponent: Item {
            Rectangle {
                id: noticeRect
                width: parent.width * 0.8
                height: noticeText.contentHeight * 1.25
                border { width: 1; color: "black" }
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    id: noticeText
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    width: parent.width-10
                    text: noticeMessage
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                    elide: Text.ElideMiddle
                }
            }

            SequentialAnimation {
                running: true

                PropertyAnimation {
                    target: noticeRect
                    properties: "y"
                    from: -noticeRect.height
                    to: 0
                    duration: 250
                }

                PauseAnimation {
                    duration: 2000
                }

                PropertyAnimation {
                    target: noticeRect
                    properties: "y"
                    from: 0
                    to: -noticeRect.height
                    duration: 250
                }

                ScriptAction {
                    script: notificationBar.noticeMessage = ""
                }
            }
        }
    }
}
