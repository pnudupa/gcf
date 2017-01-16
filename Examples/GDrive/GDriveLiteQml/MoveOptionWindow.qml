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
    width: 600
    height: 500
    anchors.leftMargin: 10
    anchors.rightMargin: 10
    color: "lightgray"

    Column {
        width: parent.width - parent.anchors.leftMargin - parent.anchors.rightMargin
        height: parent.height
        anchors.centerIn: parent
        spacing: 5

        Item {
            id: titleRect
            width: parent.width - parent.spacing
            height: 60

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: parent.height*0.5
                text: "Title: " + fileName
            }
        }

        ContentBrowser {
            id: contentBrowser
            folderView: true
            showThumbnailInSelectedItem: false
            width: parent.width - parent.spacing
            height: parent.height - ( titleRect.height + noteRect.height + buttonRow.height )
            color: Qt.rgba(1,1,1,0.3)
            border { width: 1; color: "black" }
            currentItemIndex: -1
            radius: 6
        }

        Item {
            id: noteRect
            width: parent.width - parent.spacing
            height: 40

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Note: if no folder is selected, then we will move \"" + fileName + "\" to MyDrive";
            }
        }


        Row {
            id: buttonRow
            width: parent.width
            height: 60
            spacing: 5

            Item {
                id: errorRect
                width: parent.width - selectButton.width - cancelButton.width - 2*parent.spacing
                height: 40

                Text {
                    id: errorText
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: contentBrowser.errorString
                    color: "darkred"
                    elide: Text.ElideRight

                    onTextChanged: {
                        if(text == "")
                            visible = false;
                        else
                            visible = true;
                    }
                }
            }

            Button {
                id: selectButton
                text: "Select"
                onClicked: {
                    gDriveHelper.move(gDriveContentListModel.getItemId(contentBrowser.currentItemIndex));
                }
            }

            Button {
                id: cancelButton
                text: "Cancel"
                onClicked: gDriveHelper.closeMoveWindow();
            }
        }
    }
}
