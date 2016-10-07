import QtQuick 1.1

MouseArea {
    id: dialog

    property alias content: content
    default property alias data: content.data

    onClicked: hide()
    visible: false
    hoverEnabled: true


    function show() {
        visible = true
    }
    function hide() {
        visible = false
    }

    MouseArea {
        anchors.fill: content
    }

    Rectangle {
        id: content
        width: parent.width - 80
        height: childrenRect.height
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#333"
        border.color: "#666"
        clip: true
    }
}
