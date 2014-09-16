import QtQuick 1.1

Item {
    property bool topFocus

    anchors.fill: parent
    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.parent.forceActiveFocus()
        }
    }

    onTopFocusChanged: parent.forceActiveFocus()
}
