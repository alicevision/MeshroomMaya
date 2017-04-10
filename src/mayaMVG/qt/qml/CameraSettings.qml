import QtQuick 1.1
import QtDesktop 0.1



GroupBox {
    property alias project: m.project
    title: "Cameras"

    QtObject {
        id: m
        property variant project
        property color textColor: "white"
        property int textSize: 11
    }

    Column {
        width: parent.width
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

            MTextField {
                id: nearValue
                width: 60
                validator: DoubleValidator{bottom: 0.001;}
                onAccepted: commit()
                function commit() { m.project.setCamerasNear(nearValue.text) }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Set"
                implicitWidth: 25
                onClicked: nearValue.commit()
            }
        }
        MSettingsEntry {
            label: "Far"
            width: parent.width
            MTextField {
                id: farValue
                width: 60
                validator: DoubleValidator{bottom: 0.001;}
                onAccepted: commit()
                function commit() { m.project.setCamerasFar(farValue.text) }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Set"
                implicitWidth: 25
                onClicked: farValue.commit()
            }
        }

        MSettingsEntry {
            label: "Image Plane Depth"
            width: parent.width

            MTextField {
                id: depthValue
                width: 60
                validator: DoubleValidator{bottom: 0.001;}
                onAccepted: commit()
                function commit() { m.project.setCamerasDepth(depthValue.text) }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Set"
                implicitWidth: 25
                onClicked: depthValue.commit()
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
                activeFocusOnPress: true
                onValueChanged: if(activeFocus) m.project.setCameraLocatorScale(value)
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: locatorScale.value.toFixed(2)
                color: "white"
            }
        }

    }
}

