import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4


Rectangle {
    id: meshItem
    color: "#303030"
    height: 65
    property alias project: m.project
    property alias mesh: m.mesh
    signal multipleSelection(int index)
    signal selection(int index)

    QtObject {
        id: m
        property variant project
        property variant mesh
    }

    StateGroup {
        id: selectionState
        states: [
            State {
                name: "SELECTED"
                when: m.mesh.isSelected
                PropertyChanges { target: meshItem; color: "#004161"; }
            }

        ]
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if(mouse.modifiers & Qt.ShiftModifier)
                multipleSelection(index)
            else {
                selection(index)
            }
        }
    }

    RowLayout {
        id: meshItemRow
        anchors.fill: parent
        anchors.margins: 10
        spacing: 4
        // Active CheckBox
        MCheckBox {
            id: meshActiveCheckBox
            checked: m.mesh.isActive
            onClicked: m.mesh.isActive = !m.mesh.isActive
        }
        // Data
        Label {
            id: meshName
            Layout.fillWidth: true
            anchors.verticalCenter: parent.verticalCenter
            text: m.mesh.name
            font.pointSize: 11
            color: "white"
        }
    }
}
