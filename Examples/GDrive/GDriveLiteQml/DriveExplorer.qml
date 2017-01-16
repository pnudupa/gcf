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
    property int editPropertyWindowHeight: 170

    id: driveExplorer
    width: 1024
    height: 768
    color: "lightgray"

    // file browser on left side
    ContentBrowser {
        id: contentBrowser
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        width: parent.width*0.35
        border { width: 2; color: "black" }
        color: Qt.rgba(1,1,1,0.3)
        radius: 6

        onCurrentItemPropertiesChanged: {
            itemPropertiesEditor.currentItemTitle = title;
            itemPropertiesEditor.currentItemDescription = description;
            itemPropertiesEditor.itemIsFile = isFile;
            itemPropertiesEditor.itemIsInMainDrive = isInMainDrive;
            itemPropertiesEditor.itemIsInTrash = isInTrash;
            itemPropertiesEditor.itemIsSpecialItem = isSpecialItem;
        }
    }

    ItemPropertiesEditor {
        id: itemPropertiesEditor
        anchors.left: contentBrowser.right
        anchors.top: parent.top
        anchors.right: parent.right
        height: editPropertyWindowHeight
        border { width: 2; color: "black" }
        color: Qt.rgba(1,1,1,0.3)
        radius: 6

        onItemDescriptionChanged: contentBrowser.changeCurrentDescription(description)
        onItemTitleChanged: contentBrowser.changeCurrentItemTitle(title)
        onCreateDirectoryClicked: contentBrowser.createDirectory()
        onDownloadClicked: contentBrowser.downloadCurrentItem()
        onRestoreClicked: contentBrowser.restoreCurrentItem()
        onRefreshClicked: contentBrowser.refreshCurrentItem()
        onUploadClicked: contentBrowser.upload()
        onDeleteClicked: contentBrowser.deleteCurrentItem()
        onTrashClicked: contentBrowser.trashCurrentItem()
        onShareClicked: contentBrowser.shareCurrentItem()
        onMoveClicked: contentBrowser.moveCurrentItem()
    }

    ContentItemProperties {
        id: propertiesArea
        anchors.left: contentBrowser.right
        anchors.top: itemPropertiesEditor.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        border { width: 2; color: "black" }
        radius: 6
        color: Qt.rgba(1,1,1,0.3)
    }

    states: [
        State {
            when: propertiesArea.propertiesListItemCount === 0
            PropertyChanges { target: itemPropertiesEditor; height: 0; visible: false; anchors.margins: 0 }
            PropertyChanges { target: propertiesArea; height: parent.height }
        },
        State {
            when: propertiesArea.propertiesListItemCount !== 0
            PropertyChanges { target: itemPropertiesEditor; height: editPropertyWindowHeight; visible: true; anchors.margins: 10 }
            PropertyChanges { target: propertiesArea; height: parent.height - editPropertyWindowHeight }
        }
    ]
}
