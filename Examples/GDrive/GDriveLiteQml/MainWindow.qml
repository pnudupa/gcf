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

    AuthenticationWizard {
        visible: !gDriveAuthenticator.authenticated
        anchors.fill: parent
    }

    DriveExplorer {
        opacity: gDriveAuthenticator.authenticated ? 100 : 0
        anchors.fill: parent
        anchors.margins: gDriveAuthenticator.authenticated ? 0 : 20

        Behavior on anchors.margins {
            NumberAnimation { duration: 500 }
        }
        Behavior on opacity {
            NumberAnimation { duration: 500 }
        }
    }

}
