import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: contextBar
    width: 30
    height: 120

    property bool settingsVisibility : true

    ColumnLayout
    {
        width: parent.width
        height: parent.height


        ToolButton {
            id: settingsButton

            width: 30
            height: 150
            iconSource: "img/down_arrow.png"

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
            implicitHeight: parent.height


            ButtonColumn
            {
                id: buttonCol
                checkedButton: selectContectButton

                width: parent.width
                height: parent.height

                ToolButton {
                    id: selectContectButton

                    width:  parent.width
                    height:  parent.width
                    text: "S"

                    onClicked: _project.onSelectContextButtonClicked();
                }

                ToolButton {
                    id: moveContextButton

                    width:  parent.width
                    height:  parent.width
                    text: "M"

                    onClicked: _project.onMoveContextButtonClicked();
                }

                ToolButton {
                    id: placeContextButton

                    width:  parent.width
                    height:  parent.width
                    text: "P"

                    onClicked: _project.onPlaceContextButtonClicked();
                }

            }
        }

        Rectangle {
            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding

            color: "red"

        }
    }
}

