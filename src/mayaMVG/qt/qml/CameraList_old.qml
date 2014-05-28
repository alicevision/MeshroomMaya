import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: cameraList

    Component {
        id: cameraDelegate

        CameraItem {
            thumb: model.modelData.imagePath
            name: model.modelData.name
            leftChecked: model.modelData.isLeftChecked
            rightChecked: model.modelData.isRightChecked
            selectionState: model.modelData.state
        }

        //        CameraItem {
        //            thumb: model.image    //image
        //            name: model.cameraName  //cameraName
        //        }
    }

    //    CameraModel {
    //        id: listModel
    //    }

    Item {
        id: loaderItem

        Loader {
            id: loader

        }
    }

    RowLayout {

        width: parent.width
        height: parent.height

        spacing: 5


        Item {
            height: parent.height
            Layout.horizontalSizePolicy: Layout.Expanding

            ListView {
                id: cameraListView

                width: parent.width
                height: parent.height

                anchors.fill: parent
                model: _project.cameraModel
                //model: listModel
                delegate: cameraDelegate
                clip: true


//                MouseArea {
//                    anchors.fill: parent

//                    onClicked: {
//                        scrollBar.minimumValue = 0;
//                    }
//                }

            }
        }

//        ScrollBar {
//            id: scrollBar
//            height: parent.height
//            implicitWidth: 15

//            property variant flickable: cameraListView

//            orientation: 0
//            maximumValue: cameraListView.contentHeight - cameraListView.height
//            minimumValue:0

//            value: cameraListView.contentY

////            MouseArea {
////                anchors.fill: parent
////                hoverEnabled: true

////                onEntered: {
////                    console.log(scrollBar.movingHorizontally);
////                }
////            }

////            onValueChanged: cameraListView.contentY = value


//        }
    }


}
