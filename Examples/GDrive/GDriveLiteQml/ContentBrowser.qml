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
    property bool folderView: false
    property string errorString: ""
    property alias currentItemIndex: contentList.currentIndex
    property bool showThumbnailInSelectedItem: true

    signal currentItemPropertiesChanged(string title,
                                        string description,
                                        bool isFile,
                                        bool isInMainDrive,
                                        bool isInTrash,
                                        bool isSpecialItem)

    id: contentBrowser

    function changeCurrentItemTitle(title) {
        gDriveHelper.rename(gDriveContentListModel.getItemId(contentList.currentIndex), title);
    }

    function changeCurrentDescription(description) {
        gDriveHelper.describe(gDriveContentListModel.getItemId(contentList.currentIndex), description);
    }

    function moveCurrentItem() {
        gDriveHelper.showMoveWindow(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function downloadCurrentItem() {
        fileDialog.showFileDialog(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function upload() {
        fileDialog.selectFolder = false;
        fileDialog.title = "Select a file to upload";
        fileDialog.showFileDialog(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function trashCurrentItem() {
        gDriveHelper.trash(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function deleteCurrentItem() {
        gDriveHelper.deletePermanently(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function restoreCurrentItem() {
        gDriveHelper.restore(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function refreshCurrentItem() {
        gDriveHelper.refresh(gDriveContentListModel.getItemId(currentItemIndex));
    }

    function createDirectory() {
        gDriveHelper.mkdir(gDriveContentListModel.getItemId(contentList.currentIndex));
    }

    function shareCurrentItem() {
        gDriveHelper.share(gDriveContentListModel.getItemId(currentItemIndex));
    }

    FileDialog {
        property string itemId: ""

        id: fileDialog
        visible: false
        modality: Qt.WindowModal
        title: "Select a folder to download into"
        selectExisting: true
        selectFolder: true

        onAccepted: {
            if(selectFolder == true)
                gDriveHelper.download(itemId, fileUrl);
            else
                gDriveHelper.upload(itemId, fileUrls);
        }

        function showFileDialog(currentItemId) {
            itemId = currentItemId;
            visible = true;
        }
    }

    ListView {
        id: contentList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 10
        clip: true
        model: gDriveContentListModel
        spacing: {
            if(folderView)
                return 0;
            return 5;
        }

        onCurrentIndexChanged: gDriveContentListModel.setContentItem(currentIndex)

        header: Rectangle {
            id: goUpButton
            width: contentList.width - 1
            height: 30
            color: {
                if(goUpButtonMouseArea.pressed)
                    return "gray"
                return goUpButtonMouseArea.containsMouse ? Qt.rgba(1,1,1,0.2) : Qt.rgba(0,0,0,0)
            }
            border { color: "white"; width: 1 }
            radius: 6

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 5
                text: ".."
            }

            MouseArea {
                id: goUpButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    contentList.currentIndex = -1;
                    if(!gDriveContentListModel.cdUp())
                        errorString = "ERROR: Parent destination is not allowed.";
                    contentBrowser.currentItemPropertiesChanged("", "", false, false, false, false);
                }
            }
        }

        delegate: Rectangle {
            id: delegateRect
            color: contentList.currentIndex === index ? Qt.rgba(1,1,1,0.3) : Qt.rgba(0,0,0,0)
            border { width: contentList.currentIndex === index ? 1 : 0; color: "gray" }
            width: contentList.width - 1
            height: {
                if(folderView && !contentIsFolder)
                    return 0;

                if(!showThumbnailInSelectedItem)
                    return summary.requiredHeight

                if(!contentIsSpecialItem)
                    return contentList.currentIndex === index ? details.requiredHeight : summary.requiredHeight
                else
                    return contentList.currentIndex === index ? summary.requiredHeight * 3 : summary.requiredHeight
            }

            radius: 6
            clip: true
            visible: folderView ? contentIsFolder : true
            Behavior on height { NumberAnimation { duration: 200 } }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    errorString = "";
                    if(folderView && contentList.currentIndex == index)
                        contentList.currentIndex = -1;
                    else
                        contentList.currentIndex = index
                }

                onDoubleClicked: {
                    if(contentIsFolder) {
                        contentList.currentIndex = -1
                        if(!gDriveContentListModel.cd(index, folderView))
                            errorString = "ERROR: Item doesn't content any folder in it.";
                    }
                }
            }

            Row {
                property int requiredHeight: 40

                id: summary
                spacing: 5
                anchors.fill: parent
                anchors.margins: 4
                visible: {
                    if(folderView || !showThumbnailInSelectedItem)
                        return true

                    if(!contentIsSpecialItem)
                        return contentList.currentIndex !== index

                    return true
                }

                Rectangle {
                    id: iconRect
                    width: parent.height*0.9
                    height: width
                    radius: width/2
                    border { width: 1; color: "black" }
                    color: Qt.rgba(1,1,1,0.8)

                    Image {
                        id: icon
                        anchors.centerIn: parent
                        source: contentIconLink
                        smooth: true
                        width: parent.height*0.6; height: width
                    }
                }

                Text {
                    id: summaryText
                    font.pixelSize: 18
                    text: contentTitle
                    elide: Text.ElideMiddle
                    anchors.verticalCenter: parent.verticalCenter
                }

                OptionButtonsToolBar {
                    id: specialItemToolBar
                    width: parent.width - iconRect.width - summaryText.width - 2*parent.spacing
                    isInMainDrive: gDriveHelper.isInMainDrive(gDriveContentListModel.getItemIdForSpecialItem(index))
                    height: parent.height
                    listViewLayoutDirection: Qt.RightToLeft

                    visible: {
                        if(contentList.currentIndex === index && contentIsSpecialItem)
                            return true;
                        return false;
                    }

                    onUploadClicked: {
                        fileDialog.selectFolder = false;
                        fileDialog.title = "Select a file to upload";
                        fileDialog.showFileDialog(gDriveContentListModel.getItemIdForSpecialItem(currentItemIndex));
                    }

                    onCreateDirClicked: {
                        gDriveHelper.mkdir(gDriveContentListModel.getItemIdForSpecialItem(currentItemIndex));
                    }

                    onRefreshClicked: {
                        gDriveHelper.refresh(gDriveContentListModel.getItemIdForSpecialItem(currentItemIndex));
                    }
                }
            }

            Item {
                property int requiredHeight: 160

                id: details
                anchors.fill: parent
                anchors.margins: 4
                visible: !summary.visible

                onVisibleChanged: {
                    if(visible) {
                        contentBrowser.currentItemPropertiesChanged(contentTitle,
                                                                    contentDescription,
                                                                    contentIsFile,
                                                                    gDriveHelper.isInMainDrive(gDriveContentListModel.getItemId(index)),
                                                                    gDriveHelper.isInTrash(gDriveContentListModel.getItemId(index)),
                                                                    contentIsSpecialItem);
                    }
                }

                Rectangle {
                    id: detailRect
                    width: parent.width
                    height: parent.height
                    color: delegateRect.color
                    anchors.left: parent.left

                    Row {
                        id: detailsTitleRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 5
                        spacing: 10

                        Image {
                            source: contentIsStarred === true ? "qrc:///Icons/starOn.png" : "qrc:///Icons/starOff.png"
                            width: detailsTitle.height
                            height: detailsTitle.height

                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    if(!contentIsStarred)
                                        gDriveHelper.star(gDriveContentListModel.getItemId(currentItemIndex));
                                    else
                                        gDriveHelper.unstar(gDriveContentListModel.getItemId(currentItemIndex));
                                }
                            }
                        }

                        Text {
                            id: detailsTitle
                            text: contentTitle
                            font.pixelSize: 18
                            elide: Text.ElideMiddle
                        }
                    }

                    Image {
                        id: detailsThumbnail
                        anchors.left: parent.left
                        anchors.top: detailsTitleRow.bottom
                        anchors.bottom: parent.bottom
                        anchors.margins: 5
                        width: height
                        smooth: true
                        source: {
                            if(contentIsFolder)
                                return contentIconLink
                            return "image://thumbnail/" + contentThumbnailLink
                        }
                    }

                    Column {
                        id: detailsProperties
                        anchors.left: detailsThumbnail.right
                        anchors.right: parent.right
                        anchors.top: detailsTitleRow.bottom
                        anchors.bottom: parent.bottom
                        anchors.margins: 5
                        spacing: 5

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "<b>Owned by</b> " + contentOwnerNames
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "<b>Last modified by</b> " + contentLastModifyingUserName + " <b>on</b> " + Qt.formatDateTime(contentModifiedDate)
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "<b>File is of type</b> " + contentMimeType + " <b>and</b> " + contentFileSize + " <b>bytes</b>"
                        }
                    }
                }
            }
        }
    }
}
