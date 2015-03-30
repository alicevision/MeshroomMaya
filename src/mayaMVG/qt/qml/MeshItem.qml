import QtQuick 1.1
import QtDesktop 0.1


Rectangle {
    id: meshItem
    border.color: "black"
    height: 75
    property alias project: m.project
    property alias mesh: m.mesh
    QtObject {
        id: m
        property variant project
        property variant mesh
    }

    RowLayout {
        id: meshItemRow
        anchors.fill: parent
        anchors.margins: 10
        spacing: 0

        // Data
        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            Text {
                id: meshName
                anchors.verticalCenter: parent.verticalCenter
                text: m.mesh.name
                font.pointSize: 12
                color: "white"
            }
        }

        // MVG CheckBox
        CheckBox {
            id: meshActiveCheckBox
            implicitHeight: parent.height
            implicitWidth: 20
            checked: m.mesh.isActive
            MouseArea {
                anchors.fill: parent
                onClicked: m.mesh.isActive = !m.mesh.isActive
            }
        }
        Item {
            implicitWidth: active.width
            height: parent.height
            Text {
                id: active
                anchors.verticalCenter: parent.verticalCenter
                text: "Active"
                color: "white"
                font.pointSize: 11
                MouseArea {
                    anchors.fill: parent
                    onClicked: m.mesh.isActive = !m.mesh.isActive
                }
            }
        }

    }
}
