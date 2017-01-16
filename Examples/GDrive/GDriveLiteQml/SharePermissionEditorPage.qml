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
    property bool isUpdateSharePermissionMode: false
    property alias sharedUser: emailLineEdit.text

    signal applyPermission(variant permission)
    signal done()

    id: shareFileFormRect
    width: 300
    height: 300

    function setDefaultvalues() {
        emailLineEdit.text = "";
        notifyImage.source = "qrc:///Icons/checkBoxOn.png";
        descriptionTextEdit.text = "";
        canWriteRadioButton.checkedStatus = true;
    }

    onIsUpdateSharePermissionModeChanged: {
        notifyRow.visible = !isUpdateSharePermissionMode;
        msgRow.visible = !isUpdateSharePermissionMode;
    }

    Column {
        spacing: 5
        anchors.fill: parent

        Row {
            width: parent.width
            height: 40
            spacing: 5

            Rectangle {
                id: emailRect
                width: 120
                height: parent.height
                color: "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Share with (email) "
                }
            }

            Rectangle {
                width: parent.width - parent.spacing - emailRect.width
                height: parent.height - 5
                color: "transparent"

                LineEdit {
                    id: emailLineEdit
                    anchors.fill: parent
                    anchors.centerIn: parent
                    font.pixelSize: 18

                    onTextChanged: {
                        errorText.text = "";
                    }
                }
            }
        }

        Row {
            id: notifyRow
            anchors.left: parent.left
            spacing: 5
            width: childrenRect.width
            height: 40

            Rectangle {
                width: emailRect.width
                height: parent.height
                color: "transparent"
                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Notify by email "
                }
            }

            Rectangle {
                id: notifyRect
                width: 40
                height: 40
                color: "transparent"

                Image {
                    id: notifyImage
                    width: 20
                    height: 20
                    anchors.centerIn: parent
                    source: "qrc:///Icons/checkBoxOn.png"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(notifyImage.source == "qrc:///Icons/checkBoxOff.png")
                            notifyImage.source = "qrc:///Icons/checkBoxOn.png";
                        else
                            notifyImage.source = "qrc:///Icons/checkBoxOff.png"
                    }
                }
            }
        }

        Row {
            id: msgRow
            width: parent.width
            height: childrenRect.height
            spacing: 5

            Rectangle {
                id: emailMsg
                width: emailRect.width
                height: 40
                color: "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "EMail Message"
                }
            }

            Rectangle {
                id: descriptionTextRect
                width: parent.width - emailMsg.width - parent.spacing
                height: 160
                color: "white"
                radius: 8
                border.color: "black"
                border.width: 1

                TextEdit {
                    id: descriptionTextEdit
                    anchors.margins: 10
                    anchors.fill: parent
                }
            }
        }

        Row {
            width: parent.width
            height: 135
            spacing: 5

            Rectangle {
                id: permisionRect
                width: emailRect.width
                height: 40
                color: "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Permission"
                }
            }

            Column {
                width: parent.width - permisionRect.width
                height: parent.height
                spacing: 5

                RadioButton {
                    id: canReadRadioButton
                    width: parent.width
                    height: 40
                    text: "Can Read"

                    onCheckedStatusChanged: {
                        if(checkedStatus) {
                            canWriteRadioButton.checkedStatus = false;
                            canCommentRadioButton.checkedStatus = false;
                        }
                    }
                }

                RadioButton {
                    id: canWriteRadioButton
                    width: parent.width
                    height: 40
                    checkedStatus: true
                    text: "Can Write"

                    onCheckedStatusChanged: {
                        if(checkedStatus) {
                            canReadRadioButton.checkedStatus = false;
                            canCommentRadioButton.checkedStatus = false;
                        }
                    }
                }

                RadioButton {
                    id: canCommentRadioButton
                    width: parent.width
                    height: 40
                    text: "Can Comment"

                    onCheckedStatusChanged: {
                        if(checkedStatus) {
                            canReadRadioButton.checkedStatus = false;
                            canWriteRadioButton.checkedStatus = false;
                        }
                    }
                }
            }
        }
    }

    Row {
        id: buttonRow
        width: parent.width
        height: 30
        anchors.bottom: parent.bottom
        spacing: 5

        Rectangle {
            id: errorRect
            width: parent.width - shareRect.width - cancelRect.width - 2*parent.spacing
            height: 30
            color: "transparent"

            Text {
                id: errorText
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: ""
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
            id: shareRect

            text: {
                if(isUpdateSharePermissionMode)
                    return "Apply"
                else
                    return "Share"
            }

            onClicked: {
                if(emailLineEdit.text == "" && !isUpdateSharePermissionMode) {
                    errorText.text = "ERROR: Email address cann't be empty."
                } else {
                    var permission =  0;

                    if(!isUpdateSharePermissionMode) {
                        var emailAddress = emailLineEdit.text;
                        var notiyEmail = true;
                        var emailMsg = descriptionTextEdit.text;

                        if(notifyImage.source == "qrc:///Icons/checkBoxOff.png")
                            notiyEmail = false;

                        if(canReadRadioButton.checkedStatus == true)
                            permission = 1;
                        else if(canCommentRadioButton.checkedStatus == true)
                            permission = 3;
                        else
                            permission = 2;

                        shrApp.addSharePermission(sharingModel, emailAddress, notiyEmail, emailMsg, permission);
                        shareFileFormRect.done();
                    } else {
                        if(canReadRadioButton.checkedStatus == true)
                            permission = 1;
                        else if(canCommentRadioButton.checkedStatus == true)
                            permission = 3;
                        else
                            permission = 2;

                        shareFileFormRect.applyPermission(permission);
                        shareFileFormRect.done();
                    }
                }
            }
        }

        Button {
            id: cancelRect
            text:"Cancel"
            onClicked: shareFileFormRect.done()
        }
    }
}
