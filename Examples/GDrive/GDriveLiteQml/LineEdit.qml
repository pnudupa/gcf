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
    property alias font: textInput.font
    property alias text: textInput.text
    property string originalText: ""
    property bool isDisable: false
    property bool enableApplyEvent: false

    signal lineEditTextChanged(string text)
    signal applyClicked()

    function editingFinished() {
        if(enableApplyEvent) {
            applyClicked();
            originalText = textInput.text;
            applyImage.visible = false;
        }

        lineEditTextChanged(textInput.text)
    }

    id: lineEdit
    color: "transparent"
    onOriginalTextChanged: text = originalText;

    Row {
        width: parent.width
        height: parent.height
        anchors.fill: parent
        spacing: 10

        Rectangle {
            id: inputRect
            width: applyImage.visible ? parent.width - applyImage.width - parent.spacing : parent.width
            height: parent.height
            color: isDisable ? "lightgray" : "white"
            border.color: "black"
            border.width: 1
            radius: 4

            TextInput {
                id: textInput
                x: 8
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                color: "black"
                activeFocusOnPress: !isDisable

                onTextChanged: {
                    if(enableApplyEvent && text != originalText)
                        applyImage.visible = true;
                    else
                        applyImage.visible = false;
                }
            }

            Behavior on width { NumberAnimation { duration: 200 } }
        }


        Image {
            id: applyImage
            width: 25
            height: 25
            anchors.verticalCenter: parent.verticalCenter
            source: "qrc:///Icons/apply.png"
            visible: false

            MouseArea {
                anchors.fill: parent
                onClicked: lineEdit.editingFinished()
            }
        }
    }

    Keys.onReturnPressed: {
        lineEdit.editingFinished();
        inputRect.focus = true
    }

    Keys.onEnterPressed: {
        lineEdit.editingFinished();
        inputRect.focus = true
    }

    Keys.onEscapePressed: {
        textInput.text = text
        inputRect.focus = true
    }
}
