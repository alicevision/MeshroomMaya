import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: main
    color: "transparent"
    width: 458
    height: 397

    property string textColor: "white"
    property int thumbSize: 90


    ColumnLayout
    {
        id: mainLayout

        width: parent.width
        height: parent.height

        // ContextBar
        ContextBar
        {
            id: contextBar

            width: parent.width
            //height: 30
            implicitHeight: 30
        }

        // ContextSettings
        Params
        {
            id: params

            width: parent.width

            property int height_standard: 130
            implicitHeight: visible ? height_standard : 0
            Layout.maximumHeight: visible ? height_standard : 0
            visible: contextBar.settingsVisibility
        }

        PointCloudItem {
            id: pointCloud

            width: parent.width
            //height: 120
            implicitHeight: 120
        }
        CameraList {
            id: cameraList

            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding
        }

    } // ColumnLayout

    // Log
    TextBox {
        id: logArea
        readOnly: true
        width: parent.width - 20
        height: 120
        visible: false

        anchors {
            bottom: parent.bottom
            left: parent.left

            bottomMargin: 10
            leftMargin: 10

        }

        text: _project.logText
    }


}
