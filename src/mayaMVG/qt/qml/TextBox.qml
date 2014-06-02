import QtQuick 1.1
import QtDesktop 0.1

Item
{
    id: textBox

    property string text: ""
    property color textColor: "black"
    property bool readOnly: true

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height

            ScrollArea {
                id: scrollArea

                anchors.fill: parent
                color: "#fec04c"
                opacity: 0.7

                contentHeight: textEdit.height

                Item {
                    anchors.fill: parent

                    TextEdit {
                        id: textEdit

                        width: parent.width
                        selectByMouse: true
                        readOnly: textBox.readOnly
                        text: textBox.text
                        color: textBox.textColor

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

        Item
        {
            implicitWidth: 20
            height: parent.height

            ButtonColumn {
                id: logAreaControls

                width: parent.width
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
}


