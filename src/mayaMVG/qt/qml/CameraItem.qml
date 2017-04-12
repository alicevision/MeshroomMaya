import QtQuick 1.1
import QtDesktop 0.1


Rectangle {
    id: cameraItem
    border.color: "black"
    color: "#262626"
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
        width: parent.width
        height: parent.height
        spacing: 10
        visible: false
        Component.onCompleted: visible = true

        CameraThumbnail {
            implicitWidth: height * m.thumbRatio
            height: parent.height
            Layout.horizontalSizePolicy: Layout.Fixed
            project: m.project
            camera: m.camera
        }

        // Right part : data
        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height

            MouseArea { // Place here to not block mouse event below
                anchors.fill: parent
                onClicked: {
                    if(mouse.modifiers & Qt.ShiftModifier)
                        multipleSelection(index)
                    else
                        selection(index)
                }
            }
            ColumnLayout {
                anchors.fill: parent
                // title
                Item {
                    width: parent.width
                    implicitHeight: cameraName.height * 2 // x2 to add vertical margins
                    Text {
                       id: cameraName
                       anchors.verticalCenter: parent.verticalCenter
                       width: parent.width
                       horizontalAlignment: Text.AlignLeft
                       elide: Text.ElideRight
                       text: m.camera.name
                       font.pointSize: 10
                       color: "white"
                    }
                }
                // extra infos (see component below)
                Item {
                    width: parent.width
                    implicitHeight: childrenRect.height
                    Layout.verticalSizePolicy: Layout.Expanding
                    Loader {
                        id: loader
                        sourceComponent: undefined
                        width: parent.width
                        height: childrenRect.height
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
        Item {
            ColumnLayout
            {
                width: parent.width
                height: childrenRect.height
                spacing: 5
                // file path
                Item {
                    width: parent.width
                    Layout.minimumHeight: childrenRect.height
                    Text {
                        width: parent.width
                        anchors.verticalCenter: parent.verticalCenter
                        text: m.camera.imagePath
                        font.pointSize: 10
                        elide: Text.ElideMiddle
                        color: "#888888"
                    }
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
                Item {
                    width: parent.width
                    Layout.minimumHeight: childrenRect.height
                    Text {
                        width: parent.width
                        text: convertWeight(m.camera.sourceWeight)
                        font.pointSize: 10
                        color: "#888888"
                    }
                }
                // Vertical spacer
                Item {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding
                }
            }
        }
    }
}
