import QtQuick 1.0
 
Item  {
    id: scrollBar
    signal positionUpdated(double value)
    signal moveFromStep(variant direction)
    property alias position: m.position
    property alias pageSize: m.pageSize
    property alias orientation: m.orientation
    property alias upLimit: m.upLimit       // ReadOnly
    property alias downLimit: m.downLimit   // ReadOnly
    // TODO : Handle horizontal scrollBar
    QtObject {
        id: m
        property real position
        property real pageSize
        property variant orientation : Qt.Vertical
        property real step: 0.1
        property real downLimit : (scrollBar.height - scrollElement.height)/scrollBar.height
        property real upLimit: scrollElement.height/scrollBar.height
    }
    opacity: 0.8
    visible: !(pageSize == 1)

    // Background
    Rectangle  {
        anchors.fill: parent
        radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
        color: "white"
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            onClicked:
            {
                if(mouseY/scrollBar.height > position)
                    moveFromStep(Qt.Key_Down)
                else
                    moveFromStep(Qt.Key_Up)
            }
        }
    }
    Rectangle  {
        id: scrollElement
        x: orientation == Qt.Vertical ? 1 : (m.position * (scrollBar.width-2) + 1)
        y: orientation == Qt.Vertical ? (m.position * (scrollBar.height-2) + 1) : 1
        width: orientation == Qt.Vertical ? (parent.width-2) : (m.pageSize * (scrollBar.width-2))
        height: orientation == Qt.Vertical ? (m.pageSize * (scrollBar.height-2)) : (parent.height-2)
        radius: orientation == Qt.Vertical ? (width/2 - 1) : (height/2 - 1)
        color: "black"
        opacity: 0.7

        MouseArea {
            anchors.fill: parent
            drag.axis: Drag.YAxis
            drag.target: scrollElement
            drag.maximumY: scrollBar.height - scrollElement.height - 1
            drag.minimumY: 1
            onPositionChanged: positionUpdated(scrollElement.y/scrollBar.height)
        }
    }

}
