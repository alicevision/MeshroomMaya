import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Item {
    id: componentList
    signal keyPressed(variant value)
    property alias thumbSize: m.thumbSize
    property alias project: m.project
    property alias model: m.model
    property alias delegate: m.delegate

    QtObject {
        id: m
        property int thumbSize
        property variant project
        property Item currenItem
        property variant model
        property variant delegate
    }

    StateGroup {
        id: modeState
        states: [
            State {
                name: "CAMERA"
                when: camButton.checked
                PropertyChanges { target: m; currenItem: camListView }
            },
            State {
                name: "MESH"
                when: meshButton.checked
                PropertyChanges { target: m; currenItem: meshListView }
            }
        ]
    }

    Text {
        id: loadingText
        anchors.centerIn: parent
        text: "LOADING ... "
        font.pointSize: 15
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        visible: _project.isProjectLoading
    }

    ExclusiveGroup {
        id: modeSwitchGroup
    }

    ColumnLayout {
        anchors.fill: parent
        visible: _project.projectDirectory.trim() != ""
        spacing: 0
        Row {
            Layout.alignment: Qt.AlignLeft
            Button {
                id: camButton
                text: "Cameras"
                checkable: true
                checked: true
                exclusiveGroup: modeSwitchGroup
            }
            Button {
                id: meshButton
                text: "Meshes"
                checkable: true

                onCheckedChanged: {
                    // Should not call this function here
                    // Problem with callbacks and refresh of the list
                    if(checked)
                        _project.reloadMVGMeshesFromMaya();
                }
                exclusiveGroup: modeSwitchGroup
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: "#333"
            focus: true

            CameraListView {
                id: camListView
                anchors.fill: parent
                anchors.margins: 1
                project: m.project
                thumbSize: m.thumbSize
                visible: m.currenItem == camListView
            }

            MeshListView {
                id: meshListView
                anchors.fill: parent
                anchors.margins: 1
                project: m.project
                visible: m.currenItem == meshListView
            }
            Keys.onPressed: {
                m.currentItem.keyPressed(event.key)
            }
        }
    }

}
