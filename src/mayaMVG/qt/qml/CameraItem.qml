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

    property variant project

    color: "transparent"
    border.color: "black"
    border.width: 1

    // States
    state: heightStateGroup.state && selectionStateGroupe.state

    StateGroup {
        id: selectionStateGroupe
        states: [
            State {
                name: "NORMAL"
                PropertyChanges { target: cameraItem; color: "transparent" }
            },
            State {
                name: "SELECTED"
                PropertyChanges { target: cameraItem; color: "#084F7E" }
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
                PropertyChanges { target: loaderItem; implicitHeight: 0 }
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

        Rectangle {
            implicitWidth: main.thumbSize
            height: parent.height
            color: "#111111"

            // Thumbnail
            CustomImage2 {
                id: thumbnail
                width: parent.width
                height: littleHeight

                anchors.verticalCenter: parent.verticalCenter
                //borderWidth: 3
                leftChecked:  cameraItem.leftChecked
                rightChecked: cameraItem.rightChecked
            }
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
                    Layout.maximumHeight: 30

                    RowLayout {
                        width: parent.width
                        height: parent.height

                        // Name
                        Text {
                            id: cameraName
                            Layout.horizontalSizePolicy: Layout.Expanding
                            font.pointSize: 11
                            color: main.textColor

                            TooltipArea {
                                id:cameraNameTooltip
                                anchors.fill: parent
                                text: "Camera name"
                            }
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

                            TooltipArea {
                                id:placePointsTooltip
                                anchors.fill: parent
                                text: "Number of placed points"
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    implicitHeight: 0
                    Layout.minimumHeight: 0
                    Layout.verticalSizePolicy: Layout.Expanding

                    color: "transparent"
                }

                // Extra datas
                Item {
                    id: loaderItem
                    width: parent.width
                    Layout.maximumHeight: loader.height
                    implicitHeight: loader.height

                    Loader {
                        id: loader
                        visible: cameraItem.infoVisibility
                        width: loaderItem.width
                        //sourceComponent = extraInformation
                    }
                }

                // Progress bar
                ProgressBar {
                    id: progressbar

                    width: parent.width
                    implicitHeight: 10
                    value: 30
                    maximumValue: 100

                    TooltipArea {
                        id:progressbarTooltip
                        anchors.fill: parent
                        text: "Reconstruction progress"
                    }
                }
            }

            // MouseArea
            MouseArea {
                id: cameraItemMouseArea

                anchors.fill: parent

                onClicked: {
                    project.getCameraAtIndex(index).select();
                }
                onDoubleClicked: {
                    loader.sourceComponent = extraInformation
                    cameraItem.infoVisibility = (cameraItem.infoVisibility ? false : true)
                }

            }
        }
    }


    function convertWeight(weight)
    {

        if(weight > Math.pow(1024, 3))
        {
            return ( Math.ceil(weight/Math.pow(1024, 3))) + " Go"
        }

        if(weight > Math.pow(1024, 2))
        {
            return ( Math.ceil(weight/Math.pow(1024, 2))) + " Mo"
        }
        else if(weight > 1024)
        {
            return ( Math.ceil(weight/1024)) + " ko"
        }
        else
        {
            return  Math.ceil((weight/1024).toString()) + " octets"
        }

    }

    // Component for loader
    Component {
        id: extraInformation

        ColumnLayout
        {
            width: parent.width
            height: childrenRect.height

            // Path
            Item {
                id: pathItem
                width: loaderItem.width
                Layout.minimumHeight: cameraPath.height

                Text {
                    id: cameraPath

                    text: thumb
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: main.textColor
                }

                TooltipArea {
                    id: pathItemTooltip
                    anchors.fill: parent
                    text: "Image path"
                }
            }

            // Size
            Item {
                id: sizeItem
                width: loaderItem.width
                Layout.minimumHeight: cameraPath.height

                Text {
                    id: size

                    text: project.getCameraAtIndex(index).sourceSize.width + "x" + project.getCameraAtIndex(index).sourceSize.height
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: main.textColor
                }

                TooltipArea {
                    id: sizeTooltip
                    anchors.fill: parent
                    text: "Image size"
                }
            }

            // Weight
            Item {
                id: weightItem
                width: loaderItem.width
                Layout.minimumHeight: cameraPath.height

                Text {
                    id: weight

                    text: convertWeight(project.getCameraAtIndex(index).sourceWeight)
                    elide: Text.ElideLeft
                    width: parent.width
                    font.pointSize: 10
                    color: main.textColor
                }

                TooltipArea {
                    id: weightTooltip
                    anchors.fill: parent
                    text: "Image weight"
                }
            }
        }

    }
}
