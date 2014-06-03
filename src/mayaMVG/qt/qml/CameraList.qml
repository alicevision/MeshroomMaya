import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraList

    Component {
        id: cameraDelegate

        CameraItem {
            id: cameraItem
            thumb: model.modelData.imagePath
            name: model.modelData.name
            leftChecked: model.modelData.isLeftChecked
            rightChecked: model.modelData.isRightChecked
            selectionState: model.modelData.state

            project: _project
            width: cameraList.width    //scroll.viewportWidth
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

        horizontalScrollBar.visible: false

        ListView {
            id: cameraListView

            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            interactive: false

            contentHeight: 50  // Arbitrary but only to enable the computation of the real children's size

            width: scroll.viewportWidth // QML bug: don't use parent with ScrollArea
            height: childrenRect.height
            model:  _project.cameraModel
            delegate: cameraDelegate
            clip: false
        }
    }
}
