import QtQuick 1.1
import QtDesktop 0.1


Item {

    id: cameraList
    property alias thumbSize: m.thumbSize
    property alias project: m.project
    QtObject {
        id: m
        property int thumbSize
        property variant project
    }
    function altColor(i) {
        var colors = [ "#262626", "#2f2f2f2f" ];
        return colors[i];
    }
    ListView {
        id: cameraListView
        anchors.fill: parent
        currentIndex: -1  // don't use ListView selection mechanism
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        model: m.project.cameraModel
        property variant aa: m.project.plop
        delegate: CameraItem {
            width: cameraList.width
            baseHeight: m.thumbSize
            color: altColor(index%2)
            camera: model.modelData
            project: m.project
        }
        clip: true
        states: State  {
            when: cameraListView.movingVertically
            PropertyChanges { target: verticalScrollBar; opacity: 0.8 }
        }
        transitions: Transition  {
            NumberAnimation { properties: "opacity"; duration: 200 }
        }
    }
    ScrollIndicator  {
        id: verticalScrollBar
        width: 12;
        height: cameraListView.height
        anchors.right: cameraListView.right
        opacity: 0
        orientation: Qt.Vertical
        position: cameraListView.visibleArea.yPosition
        pageSize: cameraListView.visibleArea.heightRatio
    }
    
}
