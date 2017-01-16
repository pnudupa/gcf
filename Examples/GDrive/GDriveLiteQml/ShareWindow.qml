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
    color: "lightgray"
    width: 640
    height: 480

    Flipable {
        property bool flipped: false

        id: flipable
        anchors.fill: parent
        anchors.margins: 10

        front: SharePermissionPage {
            id: sharePermissionPage
            width: flipable.width
            height: flipable.height

            onAddSharePermission: {
                sharePermissionEditorPage.enabled = true;
                sharePermissionPage.enabled = false;
                sharePermissionEditorPage.setDefaultvalues();
                sharePermissionEditorPage.isUpdateSharePermissionMode = false;
                flipable.flipped = !flipable.flipped;
            }

            onUpdateSharePermission: {
                sharePermissionEditorPage.enabled = true;
                sharePermissionPage.enabled = false;
                sharePermissionEditorPage.setDefaultvalues();
                sharePermissionEditorPage.sharedUser = sharedUser;
                sharePermissionEditorPage.isUpdateSharePermissionMode = true;
                flipable.flipped = !flipable.flipped;
            }
        }

        back: SharePermissionEditorPage {
            id: sharePermissionEditorPage
            width: flipable.width
            height: flipable.height

            onDone: {
                sharePermissionEditorPage.enabled = false;
                sharePermissionPage.enabled = true;
                flipable.flipped = !flipable.flipped;
            }

            onApplyPermission: {
                shrApp.updateSharePermisssion(sharingModel, sharePermissionPage.currentIndex, permission);
            }
        }

        transform: Rotation {
            id: rotation
            origin.x: flipable.width/2
            origin.y: flipable.height/2
            axis.x: 1; axis.y: 0; axis.z: 0
            angle: 0    // the default angle
        }

        states: State {
            name: "back"
            PropertyChanges { target: rotation; angle: 180 }
            when: flipable.flipped
        }

        transitions: Transition {
            NumberAnimation { target: rotation; property: "angle"; duration: 200 }
        }
    }
}
