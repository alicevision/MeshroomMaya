import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraList
    signal keyPressed(string value)
    property alias thumbSize: m.thumbSize
    property alias project: m.project
    QtObject {
        id: m
        property int thumbSize
        property variant project
        property int currentIndex
    }
    function altColor(i) {
        var colors = [ "#262626", "#2f2f2f2f" ];
        return colors[i];
    }
    function selectCameras(oldIndex, newIndex) {
        var qlist = [];
        var begin = Math.min(oldIndex, newIndex);
        var end = Math.max(oldIndex, newIndex) + 1;

        for(var i = begin; i < end; ++i)
        {
            qlist[qlist.length] = m.project.cameraModel.get(i).name;
        }
        m.project.selectItems(qlist)
        m.project.selectCameras(qlist);
    }
    function computeListPosition(direction, upLimit, downLimit, currentPosition, step)
    {
        if(direction == "up")
            return (currentPosition - step > upLimit) ? currentPosition - step : 0

        return (currentPosition + step < downLimit) ? currentPosition + step : downLimit
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        ListView {
            id: cameraListView
            height: parent.height
            Layout.horizontalSizePolicy:  Layout.Expanding
            currentIndex: -1  // don't use ListView selection mechanism
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            model: m.project.cameraModel
            delegate: CameraItem {
                width: cameraList.width
                baseHeight: m.thumbSize
                color: altColor(index%2)
                camera: model.modelData
                project: m.project
                onSelection: {
                    m.currentIndex = index
                    selectCameras(index, index) //m.currentIndex = index
                }
                onMultipleSelection: selectCameras(m.currentIndex, index)
            }
            clip: true
            transitions: Transition  {
                NumberAnimation { properties: "opacity"; duration: 200 }
            }
        }
        Item {
            height: parent.height
            implicitWidth: 12
            ScrollIndicator  {
                id: verticalScrollBar
                anchors.fill: parent
                orientation: Qt.Vertical
                position: cameraListView.visibleArea.yPosition
                pageSize: cameraListView.visibleArea.heightRatio
                onPositionUpdated: cameraListView.contentY = value * cameraListView.contentHeight
                onMoveFromStep: {
                    var newPosition = computeListPosition(direction, verticalScrollBar.upLimit, verticalScrollBar.downLimit, cameraListView.visibleArea.yPosition, 0.1)
                    cameraListView.contentY = newPosition * cameraListView.contentHeight
                }
            }
        }
    }
    onKeyPressed:
    {
        var newPosition = computeListPosition(value, verticalScrollBar.upLimit, verticalScrollBar.downLimit, cameraListView.visibleArea.yPosition, 0.1)
        cameraListView.contentY = newPosition * cameraListView.contentHeight
    }
}
