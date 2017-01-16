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
    property int propertiesListItemCount: propertiesList.count

    ListView {
        id: propertiesList
        anchors.fill: parent
        anchors.margins: 10
        model: gDriveContentListModel.contentItemProperties
        clip: true

        delegate: Rectangle {
            width: propertiesList.width-1
            height: propertyKey[0] === '_' ? 0 : 25
            opacity: height > 0 ? 100 : 0
            color: propertiesList.currentIndex === index ? Qt.rgba(1,1,1,0.3) : Qt.rgba(0,0,0,0)
            border { width: propertiesList.currentIndex === index ? 1 : 0; color: "gray" }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                spacing: 10

                Text {
                    width: parent.width * 0.4 - 10
                    text: propertyKey
                    elide: Text.ElideMiddle
                }

                Text {
                    width: parent.width * 0.6 - 10
                    elide: Text.ElideMiddle
                    text: propertyValue
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: propertiesList.currentIndex = index
            }
        }
    }
}
