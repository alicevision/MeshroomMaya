import QtQuick 1.1
import QtDesktop 0.1

Item {

    property alias settingsVisibility: m.settingsVisibility
    property alias project: m.project
    property string mode: m.mode
    signal showMeshSignal
    signal showCameraSignal
    QtObject {
        id: m
        property variant project
        property bool settingsVisibility
        property string mode: "camera"
    }
    RowLayout
    {
        anchors.fill: parent
        ToolButton {
            iconSource: "img/maya_logo.png"
            tooltip: "Selection mode"
            onClicked: m.project.activeSelectionContext();
            checked: (m.project.currentContext !== "mayaMVGTool1")
        }
        ToolButton {
            iconSource: "img/create.png"
            tooltip: "MVG mode"
            checked: (m.project.currentContext === "mayaMVGTool1") && (m.project.editMode === 0)
            onClicked: m.project.setCreationMode();
            iconSize: 20


        }
        ToolButton {
            iconSource: "img/triangulation.png"
            tooltip: "Triangulation mode"
            checked: (m.project.currentContext === "mayaMVGTool1") && (m.project.editMode === 1) && (m.project.moveMode === 0)
            onClicked: m.project.setTriangulationMode();
            iconSize: 20
        }
        ToolButton {
            iconSource: "img/pointCloud.png"
            tooltip: "Fit point cloud mode"
            checked: (m.project.currentContext === "mayaMVGTool1") && (m.project.editMode === 1) && (m.project.moveMode === 1)
            onClicked: m.project.setPointCloudMode();
            iconSize: 20
        }
        ToolButton {
            iconSource: "img/adjacentPlane.png"
            tooltip: "Adjacent face mode"
            checked: (m.project.currentContext === "mayaMVGTool1") && (m.project.editMode === 1) && (m.project.moveMode === 2)
            onClicked: m.project.setAdjacentPlaneMode();
            iconSize: 20
        }
        ToolButton {
            iconSource: "img/locatorMode.png"
            tooltip: "Locator mode"
            checked: (m.project.currentContext === "mayaMVGTool1") && (m.project.editMode === 2)
            onClicked: m.project.setLocatorMode();
            iconSize: 20
        }
        Rectangle {
            Layout.horizontalSizePolicy: Layout.Expanding
        }
        ToolButton {
            id: showCameraButton
            iconSource: "img/camera.png"
            tooltip: "Show cameras"
            iconSize: 20
            checked: (m.mode === "camera")
            onClicked: m.mode = "camera"
        }

        ToolButton {
            id: showMeshButton
            iconSource: "img/cube.png"
            iconSize: 20
            tooltip: "Show meshes"
            checked: (m.mode === "mesh")
            onClicked:
            {
                m.mode = "mesh";
                // Should not call this function here
                // Problem with callbacks and refresh of the list
                _project.reloadMVGMeshesFromMaya();
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
