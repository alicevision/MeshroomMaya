import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1


Item {
    id: root

    property int borderWidth: 0
    property bool collapsed: false

    property alias title: titleText.text
    property int horizontalPadding: 26
    readonly property alias contentLayout: contentLayout

    default property alias data: contentLayout.data

    implicitHeight: column.height + column.spacing

    Rectangle {
        anchors.fill: parent
        color: Qt.darker(titleBar.color, 1.6)
        border.color: titleBar.color
        border.width: borderWidth
    }

    Column {
        id: column
        width: parent.width
        spacing: contentLayout.visible ? 6 : 0

        // Title Bar
        Rectangle {
            id: titleBar
            width: parent.width
            color: "#5D5D5D"
            height: titleLayout.height
            radius: 2
            RowLayout {
                id: titleLayout
                spacing: 8
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter

                Label {
                    id: collapseButton
                    text: "►"
                    rotation: collapsed ? 0 : 90
                    color: "white"
                    Layout.alignment: Qt.AlignVCenter
                }

                Label {
                    id: titleText
                    Layout.fillWidth: true
                    Layout.preferredHeight: paintedHeight + 10
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: collapsed = !collapsed
            }
        }

        // Content
        GridLayout {
            id: contentLayout
            visible: !collapsed
            width: parent.width - horizontalPadding
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 1
            rowSpacing: 2
        }
    }
}
