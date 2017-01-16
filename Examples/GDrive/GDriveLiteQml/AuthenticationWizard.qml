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
import QtWebKit 3.0

Rectangle {
    width: 100
    height: 62
    color: Qt.rgba(0,0,0,0)

    Column {
        anchors.centerIn: parent
        spacing: 10

        Text {
            text: "Authentication Page"
            font.pixelSize: 22
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            width: 820; height: 620

            gradient: Gradient {
                GradientStop { color: Qt.rgba(1,1,1,0.8); position: 0 }
                GradientStop { color: Qt.rgba(0.6,0.6,0.6,0.8); position: 1 }
            }

            border { width: 1; color: "black" }
            radius: 8

            WebView {
                anchors.fill: parent
                anchors.margins: 10
                url: gDriveAuthenticator.authenticationPageUrl
                onTitleChanged: gDriveAuthenticator.authenticateFromPageTitle(title)
            }
        }
    }
}
