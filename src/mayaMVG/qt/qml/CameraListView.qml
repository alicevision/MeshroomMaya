import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraListView
    signal keyPressed(variant value)
    property alias project: m.project
    property alias thumbSize: m.thumbSize
    property alias currentIndex: m.currentIndex
    QtObject {
        id: m
        property variant project
        property int thumbSize
        property int currentIndex
    }

//    function altColor(i) {
//        var colors = ["#262626", "#2f2f2f2f"];
//        if(i > colors.length)
//            return "#262626";
//        return colors[i];
//    }

    function selectCameras(oldIndex, newIndex) {
        var qlist = [];
        var begin = Math.min(oldIndex, newIndex);
        var end = Math.max(oldIndex, newIndex) + 1;

        for(var i = begin; i < end; ++i)
        {
            qlist[qlist.length] = m.project.cameraModel.get(i).dagPath;
        }
        m.project.addCamerasToIHMSelection(qlist);
        m.project.addCamerasToMayaSelection(qlist);
    }

    function center(index, itemHeight, listView) {
        var itemY = index * itemHeight
        if(itemY > listView.contentHeight - listView.height)
            return listView.contentHeight - listView.height;
        else
            return itemY;
    }

    Connections {
         target: m.project
         onCenterCameraListByIndex: listView.contentY = center(cameraIndex , m.thumbSize, listView)
     }

    Component {
        id: cameraItem
        CameraItem {
            width: listView.width
            baseHeight: cameraListView.thumbSize
            camera: object
            project: cameraListView.project
            onSelection: {
                cameraListView.currentIndex = index
                selectCameras(index, index)
            }
            onMultipleSelection: selectCameras(cameraListView.currentIndex, index)
        }
    }

    CustomListView {
        id: listView
        anchors.fill: parent
        model: m.project.cameraModel
        delegate: cameraItem
    }

    onKeyPressed: listView.keyPressed(value)
}
