import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraSettings
    property alias project: m.project
    QtObject {
        id: m
        property variant project
        property color textColor: "white"
        property int textSize: 11
    }

    Item { // needed to set margins around GroupBox
        clip: true
        anchors.fill: parent
        GroupBox {
            title: "Cameras"
            anchors.fill: parent
            anchors.margins: 5
            Column {
                anchors.fill: parent
                spacing: 2

                ToolButton {
                    height: 25
                    width: height
                    iconSource: "img/mapPathFromABC.png"
                    tooltip: "Remap paths from .abc file"
                    onClicked: {
                        var abcFile = m.project.openFileDialog()
                        m.project.remapPaths(abcFile)
                    }
                }
                MSettingsEntry {
                    label: "Near"
                    width: parent.width

                    TextField {
                        id: nearValue
                        width: 60
                        validator: DoubleValidator{bottom: 0.001;}
                    }
                    Button {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Set"
                        implicitWidth: 25
                        onClicked: m.project.setCamerasNear(nearValue.text)
                    }
                }
                MSettingsEntry {
                    label: "Far"
                    width: parent.width

                    TextField {
                        id: farValue
                        width: 60
                        validator: DoubleValidator{bottom: 0.001;}
                    }
                    Button {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Set"
                        implicitWidth: 25
                        onClicked: m.project.setCamerasFar(farValue.text)
                    }
                }

                MSettingsEntry {
                    label: "Image Plane Depth"
                    width: parent.width

                    TextField {
                        id: depthValue
                        width: 60
                        validator: DoubleValidator{bottom: 0.001;}
                    }
                    Button {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Set"
                        implicitWidth: 25
                        onClicked: m.project.setCamerasDepth(depthValue.text)
                    }
                }
                MSettingsEntry {
                    label: "Locator Scale"
                    width: parent.width

                    Slider {
                        id: locatorScale
                        width: 90
                        minimumValue: 0.01
                        maximumValue: 1
                        value: 1
                        onValueChanged: m.project.setCameraLocatorScale(value)
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: locatorScale.value.toFixed(2)
                        color: "white"
                    }
                }

            }         
        }
    }
}
