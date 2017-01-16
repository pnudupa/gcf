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
    property alias currentItemTitle: nameLineEdit.originalText
    property alias currentItemDescription: descriptionLineEdit.originalText
    property int  eachRowHeight: 30
    property bool itemIsFile: false
    property bool itemIsInMainDrive: true
    property bool itemIsSpecialItem: false
    property bool itemIsInTrash: false

    signal itemDescriptionChanged(string description)
    signal itemTitleChanged(string title)
    signal createDirectoryClicked()
    signal downloadClicked()
    signal restoreClicked()
    signal refreshClicked()
    signal uploadClicked()
    signal deleteClicked()
    signal trashClicked()
    signal shareClicked()
    signal moveClicked()

    id: editPropertyRect
    visible: false

    Row {
        id: nameEditRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width - parent.anchors.margins
        height: eachRowHeight
        anchors.margins: 20
        spacing: 20

        Item {
            id: nameRect
            width: 100
            height: eachRowHeight

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Name"
            }
        }


        LineEdit {
            id: nameLineEdit
            width: parent.width - parent.anchors.margins - nameRect.width
            height: eachRowHeight
            enableApplyEvent: true

            onApplyClicked: {
                itemPropertiesEditor.itemTitleChanged(nameLineEdit.text);
                originalText = nameLineEdit.text;
            }
        }
    }

    Row {
        id: descriptionEditRow
        anchors.top: nameEditRow.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width - parent.anchors.margins
        height: eachRowHeight
        anchors.margins: 20
        spacing: 20

        Item {
            id: descriptionRect
            width: 100
            height: eachRowHeight

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Description"
            }
        }

        LineEdit {
            id: descriptionLineEdit
            width: parent.width - parent.anchors.margins - descriptionRect.width
            height: eachRowHeight
            enableApplyEvent: true
            onApplyClicked: {
                itemPropertiesEditor.itemDescriptionChanged(descriptionLineEdit.text)
                originalText = descriptionLineEdit.text;
            }
        }
    }

    Item {
        id: buttonRowRect
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: childrenRect.width
        height: eachRowHeight
        anchors.margins: 20

        Row {
            spacing: 10

            Button {
                id: downloadButton
                text: "Download"
                visible: itemIsFile
                onClicked: itemPropertiesEditor.downloadClicked()
            }

            Button {
                id: uploadButton
                text: "Upload"
                visible: !itemIsFile && itemIsInMainDrive
                onClicked: itemPropertiesEditor.uploadClicked()
            }

            Button {
                id: moveButton
                text: "Move"
                onClicked: itemPropertiesEditor.moveClicked()
                visible: itemIsInMainDrive
            }

            Button {
                id: createDirectoryButton
                text: "Create Directory"
                visible: itemIsInMainDrive && !itemIsFile
                onClicked: itemPropertiesEditor.createDirectoryClicked()
            }

            Button {
                id:shareButton
                text: "Share"
                visible: !itemIsSpecialItem
                onClicked: itemPropertiesEditor.shareClicked()
            }

            Button {
                id: trashButton
                text: "Trash"
                visible: !itemIsInTrash && !itemIsSpecialItem
                onClicked: itemPropertiesEditor.trashClicked()
            }

            Button {
                id: deleteButton
                text: "Delete"
                visible: itemIsInTrash && !itemIsSpecialItem
                onClicked: itemPropertiesEditor.deleteClicked()
            }

            Button {
                id: restoreButton
                text: "Restore"
                visible: itemIsInTrash && !itemIsSpecialItem
                onClicked: itemPropertiesEditor.restoreClicked()
            }

            Button {
                id: refreshButton
                text: "Refresh"
                onClicked: itemPropertiesEditor.refreshClicked()
            }
        }
    }
}
