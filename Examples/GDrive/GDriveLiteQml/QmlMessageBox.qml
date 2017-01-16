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
    width: 400
    height: 150
    anchors.margins: 20
    color: "lightgray"

    Text {
        id: infoText
        x: 10
        anchors.top: parent.top
        text: messageBoxText
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        width: parent.width - parent.anchors.margins - 10

        height: parent.height - buttonRow.height - parent.anchors.margins
    }

    Row {
        id: buttonRow
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 20
        anchors.rightMargin: 10
        anchors.bottomMargin: 10
        height: childrenRect.height
        spacing: 10

        Button {
            id: okButton
            visible: isOkButtonVisible
            text: "Ok"
            onClicked: window.ok()
        }

        Button {
            id: cancelButton
            visible: isCancelButtonVisible
            text: "Cancel"
            onClicked: window.cancel()
        }
    }
}
