import QtQuick 1.1
import QtDesktop 0.1


Item
{
    id: textBox

    property alias text: textEdit.text
    property alias textColor: textEdit.color
    property alias readOnly: textEdit.readOnly

    RowLayout {
        width: parent.width
        height: parent.height

        spacing: 0

        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height

            ScrollArea {
                id: scrollArea

                width: parent.width
                height: parent.height
                color: "#fec04c"
                opacity: 0.7

                contentHeight: textEdit.height

                Item {
                    width: parent.width
                    height: parent.height

                    TextEdit {
                        id: textEdit

                        width: parent.width
                        selectByMouse: true
                        readOnly: true

                        onTextChanged: textBox.visible = true;
                    }
                }
            }

            TooltipArea {
                id: logAreaTooltip
                anchors.fill: parent
                text: "Log area"
            }
        }

        ButtonColumn {
            id: logAreaControls

            width: 20
            height: parent.height
            exclusive: false


            ToolButton {
                id: closeButton

                width: parent.width
                height: parent.width
                iconSource: "img/cross.png"
                iconSize: 12
                tooltip: "Close log area"
                checked: true

                onClicked: textBox.visible = false;
            }

            ToolButton {
                id: clearButton

                width: parent.width
                height: parent.width
                iconSource: "img/clear.png"
                iconSize: 15
                checked: true
                tooltip: "Clear log area"

                onClicked: _project.setLogText(" ");
            }
        }
    }
}


