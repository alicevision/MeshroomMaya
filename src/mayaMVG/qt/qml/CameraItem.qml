import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: cameraItem

    // Alias
    property alias thumb: thumbnail.source
    property alias name: cameraName.text
    property alias selectionState: selectionStateGroupe.state
    property alias leftChecked: leftButton.checked
    property alias rightChecked: rightButton.checked

    // Params
    width: parent.width
    height: 50
    color: "transparent"
    border.color: "black"
    border.width: 1

    // States
    state: selectionStateGroupe.state

    StateGroup {
        id: selectionStateGroupe
        states: [
            State {
                name: "NORMAL"
                PropertyChanges { target: cameraItem; color: "transparent" }
            },
            State {
                name: "SELECTED"
                PropertyChanges { target: cameraItem; color: "blue" }
            }

        ]
    }

    RowLayout {
        id: cameraItemRow
        width: parent.width
        height: parent.height

        spacing: 5

        // Left Button
        Button {
            id: leftButton

            text: "L"
            height: parent.height
            implicitWidth: height/2

            onClicked: _project.getCameraAtIndex(index).onLeftButtonClicked()
        }

        // Right Button
        Button {
            id: rightButton

            implicitWidth: height/2
            height: parent.height
            text: "R"

            onClicked: _project.getCameraAtIndex(index).onRightButtonClicked()


        }

        // Thumbnail
        Image {
            id: thumbnail

            sourceSize.width: parent.height
            sourceSize.height: parent.height
            height: parent.height
        }

        // Name
        Text {
            id: cameraName

            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
            color: "white"
        }
    }
}
