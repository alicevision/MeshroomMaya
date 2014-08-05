import QtQuick 1.1
import QtDesktop 0.1

Item {
   
    ColumnLayout
    {
        anchors.fill: parent
        ContextBar {
            id: contextBar
            implicitHeight: 35
            implicitWidth: parent.width
            project: _project
        }
        ProjectSettings {
            id: settings
            implicitWidth: parent.width
            Layout.minimumHeight: childrenRect.height
            Layout.maximumHeight: childrenRect.height
            Layout.verticalSizePolicy: Layout.Expanding
            isOpen: contextBar.settingsVisibility
            project: _project
        }
        // PointCloudItem {
        //     implicitWidth: parent.width
        //     Layout.minimumHeight: childrenRect.height
        //     Layout.maximumHeight: childrenRect.height
        //     Layout.verticalSizePolicy: Layout.Expanding
        //     thumbSize: settings.thumbSize
        // }
        CameraList {
            implicitWidth: parent.width
            Layout.verticalSizePolicy: Layout.Expanding
            thumbSize: settings.thumbSize
            project: _project
        }
    }

    // Log
    LogText {
        id: logText
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

    // Open/Close LogText
    Rectangle {
        width: 20
        height: 20
        color: "#fec04c"
        opacity: 0.7

        anchors {
            bottom: parent.bottom
            right: parent.right

        }
        ToolButton {
            anchors.fill: parent
            iconSource: "img/up_left_arrow.png"
            tooltip: "Open error log"
            onClicked: logText.visible = true
        }
    }
}
