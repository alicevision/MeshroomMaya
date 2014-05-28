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

            width: parent.width
            height: littleHeight
        }

//        CameraItem {
//            id: cameraItem
//            thumb: model.image    //image
//            name: model.cameraName  //cameraName
//            width: parent.width
//            //height: 40
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


    ScrollArea {
        id: scroll
        width: parent.width
        height: parent.height
//        color: "grey"
/*
        Item {
            id: toto
            width: scroll.viewportWidth
            height: cameraListView.height
*/
            ListView {
                id: cameraListView

                currentIndex: -1
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick
                interactive: false

                width: scroll.viewportWidth
                height: (3*main.thumbSize/4 + spacing) * count
                model:  _project.cameraModel
                delegate: cameraDelegate
                clip: false
            }
        }
    //}
}


//import QtQuick 1.1
//import QtDesktop 0.1


//Item {
//    id: cameraList

//    Component {
//        id: cameraDelegate

//        CameraItem {

//            property int littleHeight: 3*main.thumbSize/4
//            property int bigHeight: littleHeight + 50
//            property bool infoVisibility: false

//            width: cameraList.width
//            height: littleHeight

//            thumb: model.modelData.imagePath
//            name: model.modelData.name
//            leftChecked: model.modelData.isLeftChecked
//            rightChecked: model.modelData.isRightChecked
//            selectionState: model.modelData.state
//        }

////        CameraItem {
////            id: cameraItem
////            thumb: model.image    //image
////            name: model.cameraName  //cameraName
////            width: parent.width
////            //height: 40
////        }
//    }

////    CameraModel {
////        id: listModel
////    }

//    ScrollArea {
//        id: scroll
//        width: parent.width
//        height: parent.height

///*
//        Item {
//            id: toto
//            width: scroll.viewportWidth
//            height: cameraListView.height
//*/
//            ListView {
//                id: cameraListView

//                currentIndex: -1
//                boundsBehavior: Flickable.StopAtBounds
//                flickableDirection: Flickable.VerticalFlick
//                interactive: false

//                width: cameraList.width
//                height: (3*main.thumbSize/4 + spacing) * count //TO DO
//                model:  _project.cameraModel
//                delegate: cameraDelegate
//                clip: false

//                Component.onCompleted: {
//                    width = scroll.viewportWidth
//                    height = contentHeight
//                }
//            }
//        }
//    //}
//}
