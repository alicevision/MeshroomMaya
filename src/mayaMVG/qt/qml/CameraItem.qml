import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: cameraItem

    property alias thumb: thumbnail.source
    property alias name: cameraName.text
    property alias selectionState: selectionStateGroupe.state


    property bool leftChecked: false
    property bool rightChecked: false
    property int littleHeight: 3*main.thumbSize/4
    property int bigHeight: littleHeight + 50
    property bool infoVisibility: false


    width: parent.width
    height: littleHeight
    color: "transparent"
    border.color: "black"
    border.width: 1

    // States
    state: heightStateGroup.state

    StateGroup {
        id: selectionStateGroupe
        states: [
            State {
                name: "NORMAL"
                PropertyChanges { target: cameraItem; color: "transparent" }
            },
            State {
                name: "SELECTED"
                PropertyChanges { target: cameraItem; color: "blue" }
            }

        ]
    }

    StateGroup {
        id: heightStateGroup
        states: [
            State {
                name: "CLOSED"
                when: (!cameraItem.infoVisibility)
                PropertyChanges { target: cameraItem; height: littleHeight }
                PropertyChanges { target: pathItem; implicitHeight: 0 }
                //PropertyChanges { target: infoItem; implicitHeight: 0 }
            },
            State {
                name: "OPEN"
                when: (cameraItem.infoVisibility)
                PropertyChanges { target: cameraItem; height: bigHeight }
            }

        ]
    }

    RowLayout {
        id: cameraItemRow
        width: parent.width
        height: parent.height

        spacing: 5

        // Thumbnail
        CustomImage2 {
            id: thumbnail
            implicitWidth: main.thumbSize
            height: littleHeight

            anchors.top: parent.top
            //borderWidth: 3
            leftChecked:  cameraItem.leftChecked
            rightChecked: cameraItem.rightChecked
        }

        // Datas
        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height

            ColumnLayout {
                width: parent.width
                height: parent.height

                Item {
                    width: parent.width
                    implicitHeight: 30

                    RowLayout {
                        width: parent.width
                        height: parent.height

                        // Name
                        Text {
                            id: cameraName
                            Layout.horizontalSizePolicy: Layout.Expanding
                            font.pointSize: 11
                            color: main.textColor
                        }

                        // Placed points
                        Rectangle {
                            implicitWidth: 15
                            height: 15

                            color: "grey"

                            Text {
                                id: placeCount
                                anchors.fill: parent

                                horizontalAlignment: Text.Center
                                verticalAlignment: Text.AlignVCenter

                                text: "4"
                                color: main.textColor
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding

                    color: "transparent"
                }



                 // Path
                Item {
                    id: pathItem
                    width: parent.width
                    implicitHeight: 20

                    Text {
                        id: cameraPath

                        text: thumb
                        elide: Text.ElideLeft
                        width: parent.width
                        anchors.left: parent.left
                        font.pointSize: 10
                        color: main.textColor
                        visible: infoVisibility
                    }
                }

//                // Infos
//                Item {
//                    id: infoItem
//                    width: parent.width
//                    implicitHeight: 20


//                    Text {
//                        id: cameraInfo

//                        height: 20
//                        text: _project.getCameraAtIndex(index).sourceSize.width + "x" + _project.getCameraAtIndex(index).sourceSize.height
//                        width: parent.width
//                        anchors.left: parent.left
//                        font.pointSize: 10
//                        color: main.textColor
//                        visible: infoVisibility
//                    }
//                }

                // Progress bar
                ProgressBar {
                    id: progressbar

                    width: parent.width
                    implicitHeight: 10
                    value: 30
                    maximumValue: 100
                }



            }

            // MouseArea
            MouseArea {
                id: cameraItemMouseArea

                anchors.fill: parent

                onClicked: cameraItem.infoVisibility = (cameraItem.infoVisibility ? false : true)

            }
        }
    }
}
