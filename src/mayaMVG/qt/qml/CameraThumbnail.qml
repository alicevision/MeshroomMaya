import QtQuick 1.1
import QtDesktop 0.1


Rectangle {

    color: "#111111"
    property alias project: m.project
    property alias camera: m.camera
    QtObject {
        id: m
        property variant project
        property variant camera
        property string source: "img/Folder.png"
        property color selectedColor: "#fec04c"
        property color hoverColor: "white"
        property color normalColor: "transparent"
    }

    Item {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        // TODO: use max(width, height)

        Image {
            id: cameraThumbnail
            anchors.fill: parent
            sourceSize.width: settings.sliderMaxValue  // Use proxy buffer at smaller resolution
            source: m.camera ? m.camera.imagePath : m.source
            asynchronous: true
            cache: true
            // fillMode: Image.PreserveAspectFit
        }

        Image {
            id: thumbnailProgress
            anchors.centerIn: parent
            width: parent.width/3
            height: parent.width/3
            sourceSize.width: 50
            source: "img/progress.png"
            visible: cameraThumbnail.status != Image.Ready
            RotationAnimation on rotation{
                from: 0; to: 360; loops: Animation.Infinite; direction: RotationAnimation.Clockwise; duration: 1500
            }
        }

        ListView {
            id: viewList
            model: m.project.visiblePanelNames
            anchors.fill: parent
            orientation: ListView.Horizontal
            interactive: false;
            property int itemWidth: (parent.width - spacing) / m.project.visiblePanelNames.length
            spacing: 2
            delegate: Rectangle {
                id: cameraSelection
                height: parent.height
                width: viewList.itemWidth
                color: m.normalColor
                opacity: 0.6
                property variant views: m.camera.views
                property bool isInView: false
                onViewsChanged: {
                    isInView = m.camera.isInView(model.modelData)
                }
                states: [
                    State {
                        name: "HOVER"
                        when: camMouseArea.containsMouse
                        PropertyChanges { target: cameraSelection; color: m.hoverColor}
                    },
                    State {
                        name: "SELECTED"
                        when: cameraSelection.isInView
                        PropertyChanges{ target: cameraSelection; color: m.selectedColor}
                    }
                ]
                MouseArea {
                    id: camMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        m.project.setCameraToView(m.camera, model.modelData);
                    }
                }
            }
        }
    }
}
