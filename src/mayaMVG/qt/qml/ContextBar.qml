import QtQuick 1.1
import QtDesktop 0.1

Item {

    property alias settingsVisibility: m.settingsVisibility
    property alias project: m.project
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
            iconSource: "img/settings.png"
            checked: m.settingsVisibility
            tooltip: (m.settingsVisibility ? "Close settings" : "Show settings")
            onClicked: m.settingsVisibility = !m.settingsVisibility
        }
    }
}
