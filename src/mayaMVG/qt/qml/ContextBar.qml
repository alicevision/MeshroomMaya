import QtQuick 1.1
import QtDesktop 0.1

Item {

    property bool settingsVisibility
    property alias project: m.project
    QtObject {
        id: m
        property variant project
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
            iconSource: (settingsVisibility ? "img/down_arrow.png" : "img/right_arrow.png")
            onClicked: contextBar.settingsVisibility = !contextBar.settingsVisibility
        }
    }

}
