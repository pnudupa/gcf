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

import QtQuick 1.1

ListView {
    id: listView
    model: jobsList
    delegate: Rectangle {
        width: listView.width-1
        height: column.height+10
        border { width: 1; color: "black" }
        Column {
            id: column
            width: parent.width
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2
            Text { text: "Kind : " + jobKind }
            Text { text: "Title : " + jobTitle }
            Text { text: "Description : " + jobDescription }
            Text { text: "IconUrl : " + jobIconUrl }
            Text { text: "Progress : " + jobProgress }
            Text { text: "Status : " + jobStatus }
            Text { text: "IsStarted : " + jobIsStarted }
            Text { text: "IsSuspended : " + jobIsSuspended }
            Text { text: "IsComplete : " + jobIsComplete }
            Text { text: "HasError : " + jobHasError }
            Text { text: "Error : " + jobError }
        }
    }
}
