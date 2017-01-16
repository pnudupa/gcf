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
    // this tool bar id for special items only
    id: optionBar
    property int heightWithToolTip: 80
    property alias listViewLayoutDirection: optionBarListView.layoutDirection
    property bool isToolTipOn: false
    property bool isInMainDrive: true

    signal uploadClicked
    signal createDirClicked
    signal refreshClicked

    height: heightWithToolTip/2
    color: "transparent"

    function updateOptionModel()
    {
        optionBarModel.clear();

        if(isInMainDrive)
        {
            optionBarModel.append( { "name": "qrc:///Icons/upload.png",
                                        "toolTip": "Upload" } );

            optionBarModel.append( { "name": "qrc:///Icons/createDirectory.png",
                                        "toolTip": "Create Directory" } );
        }

        optionBarModel.append( { "name": "qrc:///Icons/refresh.png",
                                    "toolTip": "Refresh" } );

    }

    Component.onCompleted: {
        updateOptionModel();
    }

    ListModel {
        id: optionBarModel
        dynamicRoles: true
    }

    ListView {
        id: optionBarListView
        model: optionBarModel
        anchors.fill: parent
        orientation: ListView.Horizontal

        delegate: OptionsButton {
            imageSource: name
            toolTipText: toolTip
            height: parent.height

            onClicked: {
                if(toolTipText == "Upload")
                    optionBar.uploadClicked();
                else if(toolTipText == "Create Directory")
                    optionBar.createDirClicked();
                else if(toolTipText == "Refresh")
                    optionBar.refreshClicked();
            }
        }
    }
}
