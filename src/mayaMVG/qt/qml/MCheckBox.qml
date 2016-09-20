import QtQuick 1.1
import QtDesktop 0.1


/*
  An improved QtQuick 1.1 CheckBox, not breaking the binding for "checked" property
  when clicked. Also gives control over text color and tooltip's text.
*/
Item {
    id: root

    property string text
    property alias tooltip: tooltip.text
    property alias checked: checkbox.checked
    property color color: "white"
    signal clicked()

    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    TooltipArea {
        id: tooltip

        height: childrenRect.height
        width: childrenRect.width

        CheckBox {
            id: checkbox
            text: "<span style='color:"+ color +"'>" + root.text + "</span>"
            MouseArea {
                anchors.fill: parent
                onClicked: root.clicked()
            }
        }
    }
}
