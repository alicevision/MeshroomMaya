import QtQuick 1.1
import QtDesktop 0.1

/**
 * Contextual modal dialog with a text message and an optional set of default buttons.
 * Everything instantiated under this dialog is added between the message and those buttons.
 * Height is based on content.
 * This dialog should fill the area it's covering as a modal window; this area gets darkened as
 * the dialog is shown and a click in it automatically hides the dialog.
 */
MouseArea {
    id: dialog

    /// Main text
    property alias message: _message.text
    /// Background darkening color
    property alias darkenColor: darkener.color
    /// Whether to show the ok/cancel buttons
    property bool showButtons: true
    property alias okButton: okBtn
    property alias cancelButton: cancelBtn
    /// The Rectangle item holding the dialog's visual content
    property alias container: _container

    default property alias data: content.data

    signal accepted()
    signal rejected()

    visible: false
    hoverEnabled: true

    // Hide the dialog when this area is clicked
    onClicked: hide()

    function accept() {
        accepted();
        hide();
    }

    function reject() {
        rejected();
        hide();
    }

    function show() {
        visible = true;
    }
    function hide() {
        visible = false;
    }

    // Darken the background
    Rectangle {
        id: darkener
        anchors.fill: parent
        opacity: parent.visible ? 0.4 : 0
        color: "black"
        // Gentle fade in when dialog is shown
        Behavior on opacity {
            NumberAnimation { duration: 100 }
        }
    }

    // Clickable safe-zone filling the dialog's content
    MouseArea {
        anchors.fill: _container
    }

    Rectangle {
        id: _container
        width: parent.width - 80
        height: childrenRect.height
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top; topMargin: 10
        }
        color: "#333"
        border.color: "#666"
        clip: true

        Column {
            width: parent.width - spacing * 2
            spacing: 6
            anchors.horizontalCenter: parent.horizontalCenter

            Item { height: parent.spacing; width: 1 }

            Text {
                id: _message
                color: "white"
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: text !== ""
            }

            Item {
                id: content
                height: childrenRect.height
                width: parent.width
            }

            Row {
                height: childrenRect.height
                anchors.right: parent.right
                spacing: 8
                visible: showButtons
                Button {
                    id: cancelBtn
                    text: "Cancel"
                    onClicked: dialog.reject()
                }
                Button {
                    id: okBtn
                    text: "OK"
                    onClicked: dialog.accept()
                }
            }

            Item { height: parent.spacing; width: 1 }
        }
    }
}
