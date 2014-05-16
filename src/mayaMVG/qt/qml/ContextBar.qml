import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: contextBar
    width: 150
    height: 30

    property bool settingsVisibility : true

    RowLayout
    {
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

