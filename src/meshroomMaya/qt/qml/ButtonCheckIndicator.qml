import QtQuick 2.5

/**
 * Mimick checked button background style for Buttons with 'checkable' set to false.
 *
 * Binding 'checked' property with 'checkable' set to false allows to programmatically
 * modify the checkedState of a button without having the binding broken by user interaction.
 */
Rectangle {
    anchors.fill: parent
    visible: parent.checked
    z: -1
    color: "#222"
    border.color: "#181818"
    radius: 2
}
