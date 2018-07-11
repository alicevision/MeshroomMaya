import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Rectangle {
    id: cameraItem
    color: "#303030"
    height: m.baseHeight
    property alias camera: m.camera
    property alias project: m.project
    property alias baseHeight: m.baseHeight
    signal multipleSelection(int index)
    signal selection(int index)
    QtObject {
        id: m
        property variant camera
        property variant project
        property int baseHeight
        property int thumbRatio: 4/3
    }

    StateGroup {
        id: selectionState
        states: [
            State {
                name: "SELECTED"
                when: m.camera.isSelected
                PropertyChanges { target: cameraItem; color: "#004161"; }
                PropertyChanges { target: loader; sourceComponent: extraInformation; }
            }
        ]
        transitions: Transition {
            PropertyAnimation { target: cameraItem; properties: "height"; duration: 300 }
        }
    }

    RowLayout {
        id: cameraItemRow
        anchors.fill: parent
        anchors.margins: 3
        spacing: 10
        visible: false
        Component.onCompleted: visible = true

        CameraThumbnail {
            implicitWidth: height * m.thumbRatio
            implicitHeight: parent.height
            Layout.alignment: Qt.AlignCenter
            project: m.project
            camera: m.camera
        }

        // Right part : data
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MouseArea { // Place here to not block mouse event below
                anchors.fill: parent

                onPressed: {
                    if(mouse.modifiers & Qt.ShiftModifier)
                        multipleSelection(index)
                    else
                        selection(index)
                }
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1

                // title
                Label {
                   id: cameraName
                   Layout.fillWidth: true
                   horizontalAlignment: Text.AlignLeft
                   elide: Text.ElideRight
                   text: m.camera.name
                }

                // extra infos (see component below)
                Item {
                    Layout.fillWidth: true
                    implicitHeight: childrenRect.height
                    Layout.fillHeight: true
                    Loader {
                        id: loader
                        sourceComponent: undefined
                        width: parent.width
                    }
                }
            }
        }
    }

    function convertWeight(weight)
    {
        // TODO: constants
        if(weight > Math.pow(1024, 3))
            return (Math.ceil(weight/Math.pow(1024, 3))) + " Go"
        if(weight > Math.pow(1024, 2))
            return (Math.ceil(weight/Math.pow(1024, 2))) + " Mo"
        if(weight > 1024)
            return (Math.ceil(weight/1024)) + " ko"
        return Math.ceil((weight/1024).toString()) + " octets"
    }
    // COMPONENT extraInformation
    Component {
        id: extraInformation

        Column
        {
            //width: parent.width
            spacing: 5
            // file path
            Text {
                width: parent.width
                text: m.camera.imagePath
                elide: Text.ElideMiddle
                color: "#888888"
            }
            // Image size
            /*
            Item {
                width: parent.width
                Layout.minimumHeight: childrenRect.height
                Text {
                    width: parent.width
                    text: m.camera.sourceSize.width + "x" + m.camera.sourceSize.height
                    font.pointSize: 10
                    color: "#888888"
                }
            }*/
            // file weight
            Text {
                text: convertWeight(m.camera.sourceWeight)
                color: "#888888"
            }
        }
    }
}
