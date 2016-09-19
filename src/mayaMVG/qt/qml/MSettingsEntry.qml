import QtQuick 1.1
import QtDesktop 0.1


/*
  A simple row layout displaying a label with a fixed size on the left
  and everything instantiated as children in an expanding row on the right.
  Mimics Maya AttributeEditor look and feel.
*/
RowLayout {
    property alias label: lbl.text
    property alias labelWidth: lbl_container.width
    // Use "content" as parent for instantiated children
    default property alias children: content.children

    height: childrenRect.height
    spacing: 10

    // Fixed size label
    Item {
        id: lbl_container
        implicitWidth: 100
        height: childrenRect.height
        Text {
            id: lbl
            color: "white"
            width: parent.width
            horizontalAlignment: Text.AlignRight
        }
    }
    // Content
    Row {
        id: content
        Layout.horizontalSizePolicy: Layout.Expanding
        height: childrenRect.height
        spacing: 6
    }
}
