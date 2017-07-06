import QtQuick 2.5
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    color: "#444"
    // Needed to detect top focus changed
    TopFocusHandler {
        anchors.fill: parent
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 1
        ContextBar {
            id: contextBar
            implicitHeight: 35
            implicitWidth: parent.width
            project: _project
            settingsVisibility: (_project.projectDirectory === "")
        }

        CollapsibleItem {
            title: "Settings"
            Layout.fillWidth: true
            collapsed: !contextBar.settingsVisibility
            ProjectSettings {
                id: settings
                implicitWidth: parent.width
                Layout.minimumHeight: childrenRect.height
                Layout.maximumHeight: childrenRect.height
                Layout.fillHeight: true
                isOpen: true
                project: _project
                sliderMinValue: 90
                sliderMaxValue: 180
                onSettingProjectLoaded: contextBar.settingsVisibility = false
                thumbSize: sliderMinValue
            }
        }

        MVGPanel {
            id: componentList
            Layout.fillWidth: true
            Layout.fillHeight: true
            thumbSize: settings.thumbSize
            project: _project
        }
    }

    Keys.onPressed: componentList.keyPressed(event.key)
}
