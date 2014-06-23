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

}
