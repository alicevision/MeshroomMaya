import QtQuick 1.1
import QtDesktop 0.1

MDialog {
    id: root
    property alias project: m.project
    property bool useSelection: true

    QtObject {
        id: m
        property variant project
    }

    onVisibleChanged: {
        if(visible)
            nameTF.forceActiveFocus()
        else
            nameTF.text = ""
    }

    Column {

        width: parent.width - spacing * 2
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 6

        Item { height: parent.spacing; width: 1 }

        Text {
            text: "Enter Set Name :"
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MTextField {
            id: nameTF
            width: parent.width
            onAccepted: root.submit()
            Keys.onEscapePressed: root.hide()
        }

        Column {
            MRadioButton {
                text: "Selection - " + m.project.cameraSelectionCount + " camera(s)"
                checked: root.useSelection
                onClicked: root.useSelection = true
            }
            MRadioButton {
                text: "All - " + m.project.currentCameraSet.cameras.count + " camera(s)"
                checked: !root.useSelection
                onClicked: root.useSelection = false
            }
        }

        Row {
            anchors.right: parent.right
            height: 20
            spacing: 10
            Button {
                text: "Cancel"
                onClicked: root.hide()
            }
            Button {
                text: "OK"
                enabled: nameTF.text.trim() !== ""
                onClicked: root.submit()
            }
        }

        Item { height: parent.spacing; width: 1 }
    }

    function submit() {
        if(nameTF.text.trim() === "")
            return;
        if(useSelection)
            m.project.createCameraSetFromSelection(nameTF.text.trim())
        else
            m.project.duplicateCameraSet(nameTF.text.trim(), m.project.currentCameraSet)
        root.hide()
    }
}
