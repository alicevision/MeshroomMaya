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
        ButtonRow {
            height: parent.height
            Layout.horizontalSizePolicy: Layout.Expanding
            exclusive: true
            ToolButton {
                iconSource: "img/mouse_select.png"
                tooltip: "Selection mode"
                onClicked: m.project.onSelectContextButtonClicked();
            }
            ToolButton {
                iconSource: "img/mouse_place.png"
                tooltip: "Creation mode"
                onClicked: m.project.onPlaceContextButtonClicked();
            }
        }
        ToolButton {
            iconSource: (m.settingsVisibility ? "img/down_arrow.png" : "img/right_arrow.png")
            tooltip: (m.settingsVisibility ? "Close settings" : "Show settings")
            onClicked: m.settingsVisibility = !m.settingsVisibility
        }
    }
}
