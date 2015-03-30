import QtQuick 1.1
import QtDesktop 0.1

Item {

    property alias settingsVisibility: m.settingsVisibility
    property alias project: m.project
    signal showMeshSignal
    signal showCameraSignal
    QtObject {
        id: m
        property variant project
        property bool settingsVisibility
    }
    RowLayout
    {
        anchors.fill: parent
        ToolButton {
            iconSource: "img/mouse_select.png"
            tooltip: "Selection mode"
            onClicked: m.project.activeSelectionContext();
            checked: (m.project.currentContext === "selectSuperContext")
        }
        ToolButton {
            iconSource: "img/mouse_place.png"
            tooltip: "MVG mode"
            checked: (m.project.currentContext === "mayaMVGTool1")
            onClicked: m.project.activeMVGContext();
        }
        Rectangle {
            Layout.horizontalSizePolicy: Layout.Expanding
        }
        ToolButton {
            id: showCameraButton
            iconSource: "img/camera.png"
            tooltip: "Show cameras"
            iconSize: 20
            checked: !showMeshButton.checked
            onClicked:
            {
                showCameraSignal();
                showMeshButton.checked = false;
            }
        }

        ToolButton {
            id: showMeshButton
            iconSource: "img/cube.png"
            iconSize: 20
            tooltip: "Show meshes"
            checked: !showCameraButton.checked
            onClicked:
            {
                showMeshSignal();
                // Should not call this function here
                // Problem with callbacks and refresh of the list
                _project.reloadMVGMeshesFromMaya();
                showMeshButton.checked = true;
            }
        }

        ToolButton {
            iconSource: "img/settings.png"
            checked: m.settingsVisibility
            tooltip: (m.settingsVisibility ? "Close settings" : "Show settings")
            onClicked: m.settingsVisibility = !m.settingsVisibility
        }
    }
}
