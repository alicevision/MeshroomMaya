import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: componentList
    signal keyPressed(variant value)
    property alias model: m.model
    property alias delegate: m.delegate
    property alias contentY: componentListView.contentY
    property alias contentHeight: componentListView.contentHeight
    property alias count: componentListView.count
    QtObject {
        id: m
        property variant model
        property variant delegate
    }

    function computeListPosition(keyValue, upLimit, downLimit, currentPosition, step)
    {
        switch(keyValue)
        {
            case Qt.Key_Up:
                return (currentPosition - step > upLimit) ? currentPosition - step : 0
            case Qt.Key_Down:
                return (currentPosition + step < downLimit) ? currentPosition + step : downLimit
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        ListView {
            id: componentListView
            height: parent.height
            Layout.horizontalSizePolicy:  Layout.Expanding
            currentIndex: -1  // don't use ListView selection mechanism
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            model: m.model
            delegate: m.delegate
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
                position: componentListView.visibleArea.yPosition
                pageSize: componentListView.visibleArea.heightRatio
                onPositionUpdated: componentListView.contentY = value * componentListView.contentHeight
                onMoveFromStep: {
                    var newPosition = computeListPosition(direction, verticalScrollBar.upLimit, verticalScrollBar.downLimit, componentListView.visibleArea.yPosition, 0.1)
                    componentListView.contentY = newPosition * componentListView.contentHeight
                }
            }
        }
    }
    onKeyPressed:
    {
        switch(value)
        {
            case Qt.Key_Up:
            case Qt.Key_Down:
                var newPosition = computeListPosition(value, verticalScrollBar.upLimit, verticalScrollBar.downLimit, componentListView.visibleArea.yPosition, 0.1);
                componentListView.contentY = newPosition * componentListView.contentHeight;
                break;
            case Qt.Key_Home:
                componentListView.positionViewAtBeginning();
                break;
            case Qt.Key_End:
                componentListView.positionViewAtEnd();
                break;
        }
    }
}
