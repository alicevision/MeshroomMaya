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

    CameraModel {
        id: listModel
    }


    ListView {
        id: cameraListView

        anchors.fill: parent
        model: _project.cameraModel
        //model: listModel
        delegate: cameraDelegate

    }
}
