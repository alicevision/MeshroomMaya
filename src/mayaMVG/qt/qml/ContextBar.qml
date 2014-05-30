import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: contextBar

    property bool settingsVisibility : false

    RowLayout
    {
        id: topLayout
        width: parent.width
        height: parent.height
        spacing: 0

        Item {
            implicitWidth: 3*parent.height
            height: parent.height


            ButtonRow {
                id: buttonRow

                width: parent.width
                height: parent.height

                exclusive: true

                ToolButton {
                    id: selectContextButton

                    width:  parent.height
                    height:  parent.height
                    text: "S"
                    iconSource: "img/mouse_select.png"
                    tooltip: "Select mode"

                    onClicked: _project.onSelectContextButtonClicked();

                }

                ToolButton {
                    id: placeContextButton

                    width:  parent.height
                    height:  parent.height
                    text: "P"
                    iconSource: "img/mouse_place.png"
                    tooltip: "Place mode"

                    onClicked: _project.onPlaceContextButtonClicked();
                }

                ToolButton {
                    id: moveContextButton

                    width:  parent.height
                    height:  parent.height
                    text: "M"
                    iconSource: "img/mouse_move.png"
                    tooltip: "Move mode"

                    onClicked: _project.onMoveContextButtonClicked();
                }



            }

        }


        Rectangle {
            height: parent.height
            Layout.horizontalSizePolicy: Layout.Expanding

            color: "transparent"

        }

        // Load project button
        ToolButton {
            id: folderButton

            implicitWidth: 30
            height: 30
            iconSource: "img/Folder.png"
            tooltip: "Load project folder"

            MouseArea {
                id: folderButtonMouseArea
                anchors.fill: parent

                onClicked: _project.onBrowseDirectoryButtonClicked()
            }
        }

        // Settings button
        ToolButton {
            id: settingsButton

            implicitWidth: parent.height
            height: parent.height
            iconSource: (settingsVisibility ? "img/down_arrow.png" : "img/left_arrow.png")
            tooltip: "Show/Hide settings"

            onClicked: contextBar.settingsVisibility = (contextBar.settingsVisibility ? false : true)

        }

        Text {
            id: settingsLabel

            text: "Settings"
            color: main.textColor
            font.pointSize: 11
        }
    }
}

