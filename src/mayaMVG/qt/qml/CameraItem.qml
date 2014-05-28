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
                    Layout.verticalSizePolicy: Layout.Expanding

                    color: "transparent"
                }

                // Extra datas
                Item {
                    id: loaderItem
                    width: parent.width
                    implicitHeight: 50

                    anchors.left: parent.left

                    Loader {
                        id: loader
                        anchors.fill: parent
                        visible: cameraItem.infoVisibility

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
                onDoubleClicked: {
                    loader.sourceComponent = extraInformation
                    cameraItem.infoVisibility = (cameraItem.infoVisibility ? false : true)
                }

            }
        }
    }


    function convertWeight(weight)
    {

        console.log(weight);

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
            width: 50
            height: 50

            // Path
           Item {
               id: pathItem
               width: loaderItem.width
               implicitHeight: 20

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
                implicitHeight: 20

                Text {
                    id: size

                    text: _project.getCameraAtIndex(index).sourceSize.width + "x" + _project.getCameraAtIndex(index).sourceSize.height
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
              implicitHeight: 20

              Text {
                  id: weight

                  text: convertWeight(_project.getCameraAtIndex(index).sourceWeight)
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













//import QtQuick 1.1
//import QtDesktop 0.1

//Rectangle {
//    id: cameraItem

//    property alias thumb: thumbnail.source
//    property alias name: cameraName.text
//    property alias selectionState: selectionStateGroupe.state

//    property bool leftChecked: false
//    property bool rightChecked: false

//    color: "transparent"
//    border.color: "black"
//    border.width: 1

//    // States
//    state: heightStateGroup.state && selectionStateGroupe.state

//    StateGroup {
//        id: selectionStateGroupe
//        states: [
//            State {
//                name: "NORMAL"
//                PropertyChanges { target: cameraItem; color: "transparent" }
//            },
//            State {
//                name: "SELECTED"
//                PropertyChanges { target: cameraItem; color: "#084F7E" }
//            }

//        ]
//    }

//    StateGroup {
//        id: heightStateGroup
//        states: [
//            State {
//                name: "CLOSED"
//                when: (!cameraItem.infoVisibility)
//                PropertyChanges { target: cameraItem; height: littleHeight }
//                PropertyChanges { target: cameraListView; height: cameraListView.height - (bigHeight-littleHeight)}
//            },
//            State {
//                name: "OPEN"
//                when: (cameraItem.infoVisibility)
//                PropertyChanges { target: cameraItem; height: bigHeight }
//                PropertyChanges { target: cameraListView; height: cameraListView.height + (bigHeight-littleHeight)}
//            }

//        ]
//    }


//    RowLayout {
//        id: cameraItemRow
//        width: parent.width
//        height: parent.height

//        spacing: 5

//        // Thumbnail
//        CustomImage2 {
//            id: thumbnail
//            implicitWidth: main.thumbSize
//            height: littleHeight

//            anchors.top: parent.top
//            //borderWidth: 3
//            leftChecked:  cameraItem.leftChecked
//            rightChecked: cameraItem.rightChecked
//        }

//        // Datas
//        Item {
//            Layout.horizontalSizePolicy: Layout.Expanding
//            height: parent.height

//            ColumnLayout {
//                width: parent.width
//                height: parent.height

//                Item {
//                    width: parent.width
//                    implicitHeight: 30

//                    RowLayout {
//                        width: parent.width
//                        height: parent.height

//                        // Name
//                        Text {
//                            id: cameraName
//                            Layout.horizontalSizePolicy: Layout.Expanding
//                            font.pointSize: 11
//                            color: main.textColor

//                            TooltipArea {
//                                id:cameraNameTooltip
//                                anchors.fill: parent
//                                text: "Camera name"
//                            }
//                        }

//                        // Placed points
//                        Rectangle {
//                            implicitWidth: 15
//                            height: 15

//                            color: "grey"

//                            Text {
//                                id: placeCount
//                                anchors.fill: parent

//                                horizontalAlignment: Text.Center
//                                verticalAlignment: Text.AlignVCenter

//                                text: "4"
//                                color: main.textColor
//                            }

//                            TooltipArea {
//                                id:placePointsTooltip
//                                anchors.fill: parent
//                                text: "Number of placed points"
//                            }
//                        }
//                    }
//                }

//                Rectangle {
//                    width: parent.width
//                    Layout.verticalSizePolicy: Layout.Expanding

//                    color: "transparent"
//                }

//                // Extra datas
//                Item {
//                    id: loaderItem
//                    width: parent.width
//                    implicitHeight: 50

//                    anchors.left: parent.left

//                    Loader {
//                        id: loader
//                        anchors.fill: parent
//                        visible: cameraItem.infoVisibility

//                    }
//                }

//                // Progress bar
//                ProgressBar {
//                    id: progressbar

//                    width: parent.width
//                    implicitHeight: 10
//                    value: 30
//                    maximumValue: 100


//                    TooltipArea {
//                        id:progressbarTooltip
//                        anchors.fill: parent
//                        text: "Reconstruction progress"
//                    }
//                }
//            }

//            // MouseArea
//            MouseArea {
//                id: cameraItemMouseArea

//                anchors.fill: parent
//                onDoubleClicked: {
//                    loader.sourceComponent = extraInformation
//                    cameraItem.infoVisibility = (cameraItem.infoVisibility ? false : true)
//                }

//            }
//        }
//    }


//    function convertWeight(weight)
//    {
//        if(weight > Math.pow(1024, 3))
//        {
//            return ( Math.ceil(weight/Math.pow(1024, 3))) + " Go"
//        }

//        if(weight > Math.pow(1024, 2))
//        {
//            return ( Math.ceil(weight/Math.pow(1024, 2))) + " Mo"
//        }
//        else if(weight > 1024)
//        {
//            return ( Math.ceil(weight/1024)) + " ko"
//        }
//        else
//        {
//            return  Math.ceil((weight/1024).toString()) + " octets"
//        }

//    }

//    // Component for loader
//    Component {
//        id: extraInformation

//        ColumnLayout
//        {
//            width: 50
//            height: 50

//            // Path
//           Item {
//               id: pathItem
//               width: loaderItem.width
//               implicitHeight: 20

//               Text {
//                   id: cameraPath

//                   text: thumb
//                   elide: Text.ElideLeft
//                   width: parent.width
//                   font.pointSize: 10
//                   color: main.textColor
//               }

//               TooltipArea {
//                   id: pathItemTooltip
//                   anchors.fill: parent
//                   text: "Image path"
//               }
//           }

//           // Size
//            Item {
//                id: sizeItem
//                width: loaderItem.width
//                implicitHeight: 20

//                Text {
//                    id: size

//                    text: _project.getCameraAtIndex(index).sourceSize.width + "x" + _project.getCameraAtIndex(index).sourceSize.height
//                    elide: Text.ElideLeft
//                    width: parent.width
//                    font.pointSize: 10
//                    color: main.textColor
//                }

//                TooltipArea {
//                    id: sizeTooltip
//                    anchors.fill: parent
//                    text: "Image size"
//                }
//            }

//           // Weight
//          Item {
//              id: weightItem
//              width: loaderItem.width
//              implicitHeight: 20

//              Text {
//                  id: weight

//                  text: convertWeight(_project.getCameraAtIndex(index).sourceWeight)
//                  elide: Text.ElideLeft
//                  width: parent.width
//                  font.pointSize: 10
//                  color: main.textColor
//              }

//              TooltipArea {
//                  id: weightTooltip
//                  anchors.fill: parent
//                  text: "Image weight"
//              }
//          }


//        }
//    }
//}

