import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: stack
    anchors.fill: parent

    property alias current: m.current
    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    QtObject {
        id: m
        property int current: 0
    }

    function setOpacities() {
        for (var i = 0; i < stack.children.length; ++i) {
            stack.children[i].opacity = (i == current ? 1 : 0)
            stack.children[i].visible = (i == current ? 1 : 0)
        }
    }

}

