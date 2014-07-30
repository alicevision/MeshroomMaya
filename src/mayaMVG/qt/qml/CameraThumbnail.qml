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
        property color selectedColor: "orangered"
        property color hoverColor: "white"
        property color normalColor: "transparent"
    }

    Item {
        anchors.fill: parent
        visible: cameraThumbnail.status == Image.Ready || cameraThumbnail.status == Image.Error
        anchors.verticalCenter: parent.verticalCenter
        // TODO: use max(width, height)
        width: parent.width
        height: parent.width * m.thumbRatio
        Image {
            id: cameraThumbnail
            anchors.fill: parent
            sourceSize.width: 400  // Use proxy buffer at smaller resolution
            source: m.camera ? m.camera.imagePath : m.source
            asynchronous: true
        }
        ListView {
            id: viewList
            model: m.project.panelModel
            anchors.fill: parent
            orientation: ListView.Horizontal
            property int itemWidth: parent.width / m.project.panelModel.length
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
                        m.project.rebuildCacheFromMaya();
                    }
                }
            }
        }
    }
    
}
