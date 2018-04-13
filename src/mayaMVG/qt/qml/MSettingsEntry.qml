import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

/*
  A simple row layout displaying a label with a fixed size on the left
  and everything instantiated as children in an expanding row on the right.
  Mimics Maya AttributeEditor look and feel.
*/
RowLayout {
    property alias label: lbl.text
    property alias labelWidth: lbl_container.width
    property alias tooltip: tooltip.text
    // Use "content" as parent for instantiated children
    default property alias children: content.children

    height: childrenRect.height
    spacing: 10

    // Fixed size label
    Item {
        id: lbl_container
        implicitWidth: 100
        height: lbl.height
        Label {
            id: lbl
            width: parent.width
            horizontalAlignment: Text.AlignRight
        }
        TooltipArea {
            id: tooltip
            anchors.fill: parent
        }
    }
    // Content
    Row {
        id: content
        Layout.fillWidth: true
        spacing: 6
    }
}
