import QtQuick 1.1
import QtDesktop 0.1

Item {

    id: pointCloudItem
    clip: true
    property int thumbSize
    property string title: "mvgPointCloud"
    property string filepath: "/"
    QtObject {
        id: m
        property int thumbRatio: 4/3
    }

    state: "NORMAL"
    states: [
        State {
            name: "NORMAL"
            PropertyChanges { target: pointCloudItem; height: m.thumbHeight }
            PropertyChanges { target: loader; opacity: 0; }
        },
        State {
            name: "SELECTED"
            PropertyChanges { target: pointCloudItem; height: m.thumbHeight + 50; }
            PropertyChanges { target: loader; opacity: 100; }
        }
    ]

    transitions: Transition {
        PropertyAnimation { properties: "height"; duration: 200 }
        PropertyAnimation { properties: "opacity"; duration: 400 }
    }

    RowLayout {
        width: parent.width
        height: parent.height
        spacing: 10
        // thumbnail
        Rectangle {
            implicitWidth: thumbSize
            height: parent.height
            color: "#111111"
            Rectangle {
                implicitWidth: thumbSize
                height: m.thumbHeight
                color: "#666666"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        // item data
        Item {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            ColumnLayout {
                anchors.fill: parent
                // title
                Item {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding
                    Text {
                        text: title
                        font.pointSize: 12
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                // extra infos (see component below)
                Item {
                    width: parent.width
                    Layout.verticalSizePolicy: Layout.Expanding
                    Loader {
                        id: loader
                        opacity: 0
                        width: parent.width
                    }
                }
            }
        }
        // mouse area
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: {
                loader.sourceComponent = extraInformation
                pointCloudItem.state = (pointCloudItem.state == 'NORMAL') ? 'SELECTED' : 'NORMAL' 
            }
        }
    }
    // component
    Component {
        id: extraInformation
        Text {
            text: filepath
            elide: Text.ElideLeft
            width: parent.width -5
            font.pointSize: 9
            color: "#888888"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

}
