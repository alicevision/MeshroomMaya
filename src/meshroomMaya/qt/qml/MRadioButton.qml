import QtQuick 2.5
import QtQuick.Controls 1.4

Item {
    id: root

    property string text
    property alias tooltip: tooltip.text
    property alias checked: radiobutton.checked
    property alias enabled: radiobutton.enabled
    property color color: enabled ? "white" : "#BBB"
    signal clicked()

    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    TooltipArea {
        id: tooltip

        height: childrenRect.height
        width: childrenRect.width

        RadioButton {
            id: radiobutton
            text: "<span style='color:"+ color +"'>" + root.text + "</span>"
            MouseArea {
                anchors.fill: parent
                onClicked: root.clicked()
            }
        }
    }
}
