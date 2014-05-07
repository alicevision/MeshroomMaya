import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: contextBar
    width: 30
    height: 150

    property bool settingsVisibility : true

    function getSettingsSource()
    {
        if(settingsVisibility)
            return "img/right_arrow.png"
        else
            return "img/down_arrow.png"

    }

    ColumnLayout
    {
        width: parent.width
        height: parent.height
        spacing: 0


        ToolButton {
            id: settingsButton

            width: parent.width
            implicitHeight: parent.width
            iconSource: getSettingsSource()
            tooltip: "Show/Hide settings"

            onClicked: {

                if(contextBar.settingsVisibility) {
                    contextBar.settingsVisibility = false;
                }
                else {
                    contextBar.settingsVisibility = true;
                }
            }
        }

        Item {
            width: parent.width
            implicitHeight: 3*parent.width


            ButtonColumn
            {
                id: buttonCol
                checkedButton: selectContectButton

                width: parent.width
                height: parent.height

                ToolButton {
                    id: selectContextButton

                    width:  parent.width
                    height:  parent.width
                    text: "S"
                    tooltip: "Select mode"

                    onClicked: _project.onSelectContextButtonClicked();
                }

                ToolButton {
                    id: moveContextButton

                    width:  parent.width
                    height:  parent.width
                    text: "M"
                    tooltip: "Move mode"

                    onClicked: _project.onMoveContextButtonClicked();
                }

                ToolButton {
                    id: placeContextButton

                    width:  parent.width
                    height:  parent.width
                    text: "P"
                    tooltip: "Place mode"

                    onClicked: _project.onPlaceContextButtonClicked();
                }

            }
        }

        Rectangle {
            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding

            color: "transparent"

        }
    }
}

