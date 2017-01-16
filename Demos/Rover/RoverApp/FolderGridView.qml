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

    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: 10
        width: parent.width
        clip: true
        cellWidth: delegateWidth
        cellHeight: 150
        boundsBehavior : GridView.StopAtBounds
        currentIndex: -1
        model: folderContentModel

        property int standardCellsWidth : 120
        property int delegateWidth : standardCellsWidth

        onWidthChanged: {
            var noOfCloumns = Math.floor(width / standardCellsWidth)
            var horizantallyAllCellsGap = (width - (noOfCloumns) * standardCellsWidth)
            var horizantallyCellsGap = Math.floor(horizantallyAllCellsGap / noOfCloumns)
            delegateWidth = (standardCellsWidth + horizantallyCellsGap)
        }

        Keys.onLeftPressed: {
            if(currentIndex > 0)
                currentIndex--;
        }

        Keys.onRightPressed: {
            if(currentIndex < count-1)
                currentIndex++;
        }

        onActiveFocusChanged: {
            if(!gridView.activeFocus)
                currentIndex = -1
        }

        delegate: Rectangle {
            width: gridView.delegateWidth
            height: 145
            color: gridView.currentIndex === index ? "lightsteelblue" : "white"
            radius: 8

            Column {
                anchors.centerIn: parent
                spacing: 5
                clip: true

                Image {
                    id: image
                    asynchronous: true
                    source: {
                        if(fileThumbnail.indexOf("qrc")>= 0)
                            return fileThumbnail
                        else
                            return "image://thumbnail/"+filePath
                    }
                    smooth: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    fillMode: Image.PreserveAspectFit
                    height: 100; width: 100
                    sourceSize.width: 100
                    sourceSize.height: 100
                }

                Text {
                    height: 30
                    width: parent.width
                    text: fileName
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Keys.onReturnPressed: {
                var currentIndexOfGridView = 0;
                if(fileIsFolder) {
                    if(fileName === "..") {
                        if(stackModel.count > 0) {
                            var stackModelElement = stackModel.get(stackModel.count - 1)
                            currentIndexOfGridView = stackModelElement.modelIndex
                            stackModel.remove(stackModel.count - 1)
                        } else
                            currentIndexOfGridView = 0
                    } else {
                        stackModel.append({"modelIndex" : gridView.currentIndex } )
                        currentIndexOfGridView = 0
                    }

                    gridView.currentIndex = currentIndexOfGridView
                    folderContentModel.folder = filePath
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    gridView.currentIndex = index
                    if(!gridView.activeFocus) {
                        gridView.forceActiveFocus()
                    }
                }

                onDoubleClicked: {
                    var currentIndexOfGridView = 0;
                    if(fileIsFolder) {
                        if(fileName === "..") {
                            if(stackModel.count > 0) {
                                var stackModelElement = stackModel.get(stackModel.count - 1)
                                currentIndexOfGridView = stackModelElement.modelIndex
                                stackModel.remove(stackModel.count - 1)
                            } else
                                currentIndexOfGridView = 0
                        } else {
                            stackModel.append({"modelIndex" : gridView.currentIndex } )
                            currentIndexOfGridView = 0
                        }

                        gridView.currentIndex = currentIndexOfGridView
                        folderContentModel.folder = filePath
                    }
                }
            }
        }
    }
}
