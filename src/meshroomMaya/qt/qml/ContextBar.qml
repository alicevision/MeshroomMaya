import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

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
        property bool usingMVGTool: m.project.currentContext === "meshroomMayaTool1"
    }

    Row
    {
        anchors.fill: parent
        spacing: 4
        ToolButton {
            iconSource: "img/maya_logo.png"
            tooltip: "Selection mode"
            checked: !m.usingMVGTool
            onClicked: m.project.activeSelectionContext()
            ButtonCheckIndicator {}
        }
        ToolButton {
            iconSource: "img/create.png"
            tooltip: "MVG mode"
            checked: m.usingMVGTool && m.project.editMode === 0
            onClicked: m.project.setCreationMode()
            ButtonCheckIndicator {}
        }
        ToolButton {
            iconSource: "img/triangulation.png"
            tooltip: "Triangulation mode"
            checked: m.usingMVGTool && m.project.editMode === 1 && m.project.moveMode === 0
            onClicked: m.project.setTriangulationMode()
            ButtonCheckIndicator {}
        }
        ToolButton {
            iconSource: "img/pointCloud.png"
            tooltip: "Fit point cloud mode"
            checked: m.usingMVGTool && m.project.editMode === 1 && m.project.moveMode === 1
            onClicked: m.project.setPointCloudMode()
            ButtonCheckIndicator {}
        }
        ToolButton {
            iconSource: "img/adjacentPlane.png"
            tooltip: "Adjacent face mode"
            checked: m.usingMVGTool && m.project.editMode === 1 && m.project.moveMode === 2
            onClicked: m.project.setAdjacentPlaneMode()
            ButtonCheckIndicator {}
        }
        ToolButton {
            iconSource: "img/locatorMode.png"
            tooltip: "Locator mode"
            checked: m.usingMVGTool && m.project.editMode === 2
            onClicked: m.project.setLocatorMode()
            ButtonCheckIndicator {}
        }

    }
}
