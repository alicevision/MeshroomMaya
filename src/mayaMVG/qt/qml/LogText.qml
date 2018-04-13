import QtQuick 2.5
import QtQuick.Controls 1.4

Item
{
    id: textBox
    property alias text: m.text

    QtObject {
        id: m
        property string text: ""
        property color textColor: "black"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Item {
            Layout.fillWidth: true
            height: parent.height
            ScrollArea {
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
                        readOnly: true
                        text: m.text
                        color: m.textColor
                        onTextChanged: textBox.visible = true;
                    }
                }
            }
        }
        Item
        {
            implicitWidth: 20
            height: parent.height
            ButtonColumn {
                width: parent.width
                height: parent.height
                exclusive: false
                ToolButton {
                    id: closeButton
                    width: parent.width
                    height: parent.width
                    iconSource: "img/cross.png"
                    tooltip: "Close"
                    checked: true
                    onClicked: textBox.visible = false;
                }
                ToolButton {
                    width: parent.width
                    height: parent.width
                    iconSource: "img/clear.png"
                    checked: true
                    tooltip: "Clear"
                    onClicked: _project.logText = ""
                }
            }
        }
    }    
}


