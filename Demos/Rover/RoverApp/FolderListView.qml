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

Item {

    ListModel {
        id: stackModel
        dynamicRoles: true
    }

    ListView {
        id: listView
        model: folderContentModel
        spacing: 5
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        currentIndex: -1

        Keys.onLeftPressed: listView.decrementCurrentIndex()
        Keys.onRightPressed: listView.incrementCurrentIndex()

        onActiveFocusChanged: {
            if(!listView.activeFocus)
                currentIndex = -1
        }

        delegate: Rectangle {
            width: listView.width-1
            height: 80
            color: listView.currentIndex === index ? "lightsteelblue" : "white"
            radius: 8

            Row {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10

                Image {
                    asynchronous: true
                    source: {
                        if(fileThumbnail.indexOf("qrc")>= 0)
                            return fileThumbnail
                        return "image://thumbnail/"+filePath
                    }
                    smooth: true
                    height: parent.height; width: height
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 100
                    sourceSize.height: 100
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: fileName
                }
            }

            Keys.onReturnPressed: {
                var currentIndexOfListView = 0;
                if(fileIsFolder) {
                    if(fileName === "..") {
                        if(stackModel.count > 0) {
                            var stackModelElement = stackModel.get(stackModel.count - 1)
                            currentIndexOfListView = stackModelElement.modelIndex
                            stackModel.remove(stackModel.count - 1)
                        } else
                            currentIndexOfListView = 0
                    } else {
                        stackModel.append({"modelIndex" : listView.currentIndex } )
                        currentIndexOfListView = 0
                    }

                    listView.currentIndex = currentIndexOfListView
                    folderContentModel.folder = filePath
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                    if(!listView.activeFocus) {
                        listView.forceActiveFocus()
                    }
                }
                onDoubleClicked: {
                    var currentIndexOfListView = 0;
                    if(fileIsFolder) {
                        if(fileName === "..") {
                            if(stackModel.count > 0) {
                                var stackModelElement = stackModel.get(stackModel.count - 1)
                                currentIndexOfListView = stackModelElement.modelIndex
                                stackModel.remove(stackModel.count - 1)
                            } else
                                currentIndexOfListView = 0
                        } else {
                            stackModel.append({"modelIndex" : listView.currentIndex } )
                            currentIndexOfListView = 0
                        }

                        listView.currentIndex = currentIndexOfListView
                        folderContentModel.folder = filePath
                    }
                }
            }
        }
    }
}
