import QtQuick 2.5
import QtQuick.Controls 1.4

MDialog {
    id: root
    property alias project: m.project
    property bool useSelection: true
    message: "Camera Set name :"

    okButton.enabled: nameTF.text.trim() !== ""

    // Properties not accessible on construction
    Component.onCompleted: {
        okButton.text = "Create"
        okButton.tooltip = "Create Set"
    }

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

    onAccepted: {
        createSet(false)
    }

    function createSet(makeCurrent) {
        if(nameTF.text.trim() === "")
            return;
        if(useSelection)
            m.project.createCameraSetFromSelection(nameTF.text.trim(), makeCurrent)
        else
            m.project.duplicateCameraSet(nameTF.text.trim(), m.project.currentCameraSet, makeCurrent)
    }

    Column {
        width: parent.width
        spacing: 3

        MTextField {
            id: nameTF
            width: parent.width
            onAccepted: root.accept()
            Keys.onEscapePressed: root.reject()
        }

        GroupBox {
            title: " Content "
            width: parent.width
            Column {
                width: parent.width
                MRadioButton {
                    text: "Selected Cameras (" + m.project.cameraSelectionCount + ")"
                    enabled: m.project.cameraSelectionCount > 0
                    checked: root.useSelection
                    onClicked: root.useSelection = true
                }
                MRadioButton {
                    text: "All Cameras (" + m.project.currentCameraSet.cameras.count + ")"
                    checked: !root.useSelection
                    onClicked: root.useSelection = false
                }

            }
        }
        Button {
            text: "Create and Show"
            anchors.right: parent.right
            enabled: okButton.enabled
            onClicked: { createSet(true); root.hide(); }
        }
    }
}
