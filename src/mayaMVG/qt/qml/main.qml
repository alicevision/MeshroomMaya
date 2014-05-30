import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: main
    color: "transparent"

    property string textColor: "white"
    property int thumbSize: 90

    ColumnLayout
    {
        id: mainLayout
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 10
        height: parent.height

        // ContextBar
        ContextBar
        {
            id: contextBar
            width: parent.width
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
