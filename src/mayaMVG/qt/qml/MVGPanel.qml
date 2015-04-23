import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: componentList
    signal keyPressed(variant value)
    property alias thumbSize: m.thumbSize
    property alias project: m.project
    property alias model: m.model
    property alias delegate: m.delegate
    property string mode
    QtObject {
        id: m
        property int thumbSize
        property variant project
        property int currentIndex
        property variant model
        property variant delegate
    }

    StateGroup {
        id: modeState
        states: [
            State {
                name: "CAMERA"
                when: (mode === "camera")
                PropertyChanges { target: stack; current: 0}
            },
            State {
                name: "MESH"
                when: (mode === "mesh")
                PropertyChanges { target: stack; current: 1}
            }
        ]
    }
    Item {
        anchors.fill: parent
        Text {
            id: loadingText
            anchors.fill: parent
            text: "LOADING ... "
            font.pointSize: 15
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            visible: _project.isProjectLoading
        }
    }

    Stack {
        id: stack
        anchors.fill: parent
        focus: true
        CameraListView {
            height: componentList.height
            width: componentList.width
            project: m.project
            thumbSize: m.thumbSize
        }

        MeshListView {
            height: componentList.height
            width: componentList.width
            project: m.project
        }

        Keys.onPressed: children[current].keyPressed(event.key)
    }
}
