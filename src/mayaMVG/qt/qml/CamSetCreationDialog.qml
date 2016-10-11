import QtQuick 1.1
import QtDesktop 0.1

MDialog {
    id: root
    property alias project: m.project
    property bool useSelection: true
    message: "Enter Camera Set name :"

    okButton.enabled: nameTF.text.trim() !== ""

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
        if(nameTF.text.trim() === "")
            return;
        if(useSelection)
            m.project.createCameraSetFromSelection(nameTF.text.trim())
        else
            m.project.duplicateCameraSet(nameTF.text.trim(), m.project.currentCameraSet)
    }

    Column {
        width: parent.width
        spacing: 2

        MTextField {
            id: nameTF
            width: parent.width
            onAccepted: root.accept()
            Keys.onEscapePressed: root.reject()
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
    }
}
