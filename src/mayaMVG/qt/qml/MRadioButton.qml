import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: root

    property string text
    property alias tooltip: tooltip.text
    property alias checked: radiobutton.checked
    property color color: "white"
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
