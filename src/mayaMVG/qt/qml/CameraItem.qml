import QtQuick 1.1
import QtDesktop 0.1

Rectangle {

    id: cameraItem
    border.color: "black"
    height: m.baseHeight
    property alias camera: m.camera
    property alias project: m.project
    property alias baseHeight: m.baseHeight
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
                PropertyChanges { target: cameraItem; height: m.baseHeight + 30; }
                PropertyChanges { target: cameraItem; color: "lightsteelblue"; }
                PropertyChanges { target: loader; sourceComponent: extraInformation; }
            }
        ]
        transitions: Transition {
            PropertyAnimation { target: cameraItem; properties: "height"; duration: 300 }
        }
    }

    RowLayout {
        id: cameraItemRow
        width: parent.width
        height: parent.height
        spacing: 10
        visible: false
        Component.onCompleted: {
            visible = true
        }
        CameraThumbnail {
            implicitWidth: height * m.thumbRatio
            height: parent.height
            Layout.horizontalSizePolicy: Layout.Fixed
            project: m.project
            camera: m.camera
        }
        // right part: data
        Item {
            implicitWidth: 100
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            ColumnLayout {
                anchors.fill: parent
                // title
                Item {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding
                    Text {
                        text: m.camera.name
                        font.pointSize: 12
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                // extra infos (see component below)
                Item {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding
                    Loader {
                        id: loader
                        sourceComponent: undefined
                        width: parent.width
                        height: childrenRect.height
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    m.camera.select();
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
        ColumnLayout
        {
            // file path
            Item {
                width: parent.width
                Layout.minimumHeight: childrenRect.height
                Text {
                    text: m.camera.imagePath
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: "#888888"
                }
            }
            // file size
            Item {
                width: parent.width
                Layout.minimumHeight: childrenRect.height
                Text {
                    text: m.camera.sourceSize.width + "x" + m.camera.sourceSize.height
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: "#888888"
                }
            }
            // file weight
            Item {
                width: parent.width
                Layout.minimumHeight: childrenRect.height
                Text {
                    text: convertWeight(m.camera.sourceWeight)
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: "#888888"
                }
            }
        }
    }

}
