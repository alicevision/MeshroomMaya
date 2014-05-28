import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: customImage

    property int borderWidth: 2
    property color leftBorderColor: "blue"
    property color rightBorderColor: "yellow"
    property alias source: image.source
    property bool leftChecked: false
    property bool rightChecked: false

    property color selectedColor: "#fec04c"
    property color hoverColor: "white"
    property color normalColor: "transparent"

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
                    PropertyChanges {target: thumbnail; leftBorderColor: hoverColor}
                },
                State {
                    name: "LEFT_NORMAL"
                    when: (!leftChecked)
                    PropertyChanges{target: thumbnail; leftBorderColor: normalColor}
                },
                State {
                    name: "LEFT_SELECTED"
                    when: (leftChecked == true)
                    PropertyChanges{target: thumbnail; leftBorderColor: selectedColor}
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
                    PropertyChanges {target: thumbnail; rightBorderColor: hoverColor}
                },
                State {
                    name: "RIGHT_NORMAL"
                    when: (!rightChecked)
                    PropertyChanges{target: thumbnail; rightBorderColor: normalColor}
                },
                State {
                    name: "RIGHT_SELECTED"
                    when: (rightChecked == true)
                    PropertyChanges{target: thumbnail; rightBorderColor: selectedColor}
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
