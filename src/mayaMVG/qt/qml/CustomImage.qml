import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: customImage

    width: main.thumbSize
    height: main.thumbSize

    property int borderWidth: 2
    property color leftBorderColor: "blue"
    property color rightBorderColor: "yellow"
    property alias source: image.source
    property alias image: image
    property bool leftChecked: false
    property bool rightChecked: false

    Rectangle {
        id: leftRectangle

        anchors.left: parent.left
        width: parent.width/2
        height: parent.height
        color: leftBorderColor
        state: leftStateGroup.state

        StateGroup {
            id: leftStateGroup

            states:[
                State {
                    name: "LEFT_HOVER"
                    when: (leftMouseArea.containsMouse)
                    PropertyChanges {target: thumbnail; leftBorderColor: "white"}
                },
                State {
                    name: "LEFT_NORMAL"
                    when: (!leftChecked)
                    PropertyChanges{target: thumbnail; leftBorderColor: "transparent"}
                },
                State {
                    name: "LEFT_SELECTED"
                    when: (leftChecked == true)
                    PropertyChanges{target: thumbnail; leftBorderColor: "gold"}
                }

           ]
        }

        MouseArea {
            id: leftMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: _project.getCameraAtIndex(index).onLeftButtonClicked()
        }

    }

    Rectangle {
        id: rightRectangle

        anchors.right: parent.right
        width: parent.width/2
        height: parent.height
        color: rightBorderColor
        state: rightStateGroup.state

        StateGroup {
            id: rightStateGroup

            states:[
                State {
                    name: "RIGHT_HOVER"
                    when: (rightMouseArea.containsMouse)
                    PropertyChanges {target: thumbnail; rightBorderColor: "white"}
                },
                State {
                    name: "RIGHT_NORMAL"
                    when: (!rightChecked)
                    PropertyChanges{target: thumbnail; rightBorderColor: "transparent"}
                },
                State {
                    name: "RIGHT_SELECTED"
                    when: (rightChecked == true)
                    PropertyChanges{target: thumbnail; rightBorderColor: "gold"}
                }
           ]
        }

        MouseArea {
            id: rightMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: _project.getCameraAtIndex(index).onRightButtonClicked()
        }


    }

    Image {
        id: image

        x: leftRectangle.x + borderWidth
        y: leftRectangle.y + borderWidth

        sourceSize.width: parent.width - borderWidth*2;
        sourceSize.height: parent.height - borderWidth*2;

        source: "img/Folder.png"

    }
}
