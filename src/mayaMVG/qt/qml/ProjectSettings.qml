import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: settings
    property alias project: m.project
    property alias isOpen: m.isOpen
    property alias thumbSize: m.thumbSize
    property alias sliderMinValue: m.sliderMinValue
    property alias sliderMaxValue: m.sliderMaxValue
    signal settingProjectLoaded

    QtObject {
        id: m
        property variant project
        property bool isOpen
        property int thumbSize
        property int sliderMinValue
        property int sliderMaxValue
        property color textColor: "white"
        property int textSize: 11
        property variant leftPanel: m.project.panelList.get(0)
        property variant rightPanel: m.project.panelList.get(1)
    }

    opacity: 0  // needed for animation

    StateGroup {
        id: openState
        states: [
            State {
                name: "CLOSED"
                PropertyChanges { target: settings; height: 0; }
                PropertyChanges { target: settings; opacity: 0; }
            },
            State {
                name: "OPEN"
                when: m.isOpen
                PropertyChanges { target: settings; height: 340; }
                PropertyChanges { target: settings; opacity: 1; }
            }
        ]
        transitions: Transition {
            PropertyAnimation { target: settings; properties: "height"; duration: 200 }
            PropertyAnimation { target: settings; properties: "opacity"; duration: 100 }
        }
    }

    Item { // needed to set margins around GroupBox
        clip: true
        anchors.fill: parent

            Column {
                id: mainColumn
                anchors.fill: parent
                spacing: 4
                property int settingsEntryWidth: width - 40
                // browse button
                BrowseDirectory {
                    width: parent.width
                    title: "Scene"
                    project: m.project
                    onBrowserProjectLoaded: settingProjectLoaded()
                }

                // Toolbar
                Row {
                    height: childrenRect.height
                    spacing: 4
                    ToolButton {
                        implicitWidth: 30
                        implicitHeight: 30
                        iconSource: "img/syncSelection.png"
                        tooltip: "Synchronize with Maya Selection"
                        checked: m.project.activeSynchro
                        onClicked: m.project.activeSynchro = !m.project.activeSynchro
                        ButtonCheckIndicator {}
                    }

                    // Clear blind data
                    ToolButton {
                        implicitWidth: 30
                        implicitHeight: 30
                        iconSource: "img/clearBD.png"
                        tooltip: "Clear all 2D points (triangulation)"
                        onClicked: m.project.clearAllBlindData()
                    }
                    // Select closest cam from persp
                    ToolButton {
                        implicitHeight: 30
                        implicitWidth: 30
                        iconSource: "img/greenCamera.png"
                        tooltip: "Select closest cam"
                        onClicked: m.project.selectClosestCam()
                    }
                    // Reorient scene
                    ToolButton {
                        implicitHeight: 30
                        implicitWidth: 30
                        iconSource: "img/locatorMode.png"
                        tooltip: "Reorient scene"
                        onClicked: m.project.applySceneTransformation()
                    }
                }

                GroupBox {
                    title: "Point Cloud"
                    width: parent.width
                    Column {
                        width: parent.width
                        spacing: 2

                        MSettingsEntry {
                            label: "Display Point Cloud"
                            width: mainColumn.settingsEntryWidth

                            MCheckBox {
                                width: 40
                                text: m.leftPanel.label
                                checked: m.leftPanel.isPointCloudDisplayed
                                onClicked: m.leftPanel.isPointCloudDisplayed = !m.leftPanel.isPointCloudDisplayed
                            }
                            MCheckBox {
                                width: 40
                                text: m.rightPanel.label
                                checked: m.rightPanel.isPointCloudDisplayed
                                onClicked: m.rightPanel.isPointCloudDisplayed = !m.rightPanel.isPointCloudDisplayed
                            }
                        }

                        MSettingsEntry {
                            label: "Cam. Visible Points"
                            width: parent.width - 40
                            tooltip: "Display mode for points visible by active (left/right) cameras<br/>
                                      <b>None</b>: no display<br/>
                                      <b>Both</b>: points of both cams in both views<br/>
                                      <b>Each</b>: points of each cam only in its view<br/>
                                      <b>Common Points</b>: only points visible by both cams<br/>
                                     "
                            ComboBox {
                                implicitWidth: 120
                                model: ListModel {
                                    ListElement { text: "None" }
                                    ListElement { text: "Both" }
                                    ListElement { text: "Each" }
                                    ListElement { text: "Common Points Only" }
                                }
                                activeFocusOnPress: true
                                enabled: m.project.projectDirectory !== ""
                                currentIndex: m.project.cameraPointsDisplayMode
                                onActivated: {
                                    m.project.cameraPointsDisplayMode = index
                                    currentIndex = Qt.binding(function() { return m.project.cameraPointsDisplayMode })
                                }
                            }
                        }

                        MSettingsEntry {
                            width: mainColumn.settingsEntryWidth
                            label: "Filter Points"
                            tooltip: "Display only points seen by at least the given number of cameras of the current camera set."
                            MCheckBox {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 14
                                checked: m.project.filterPoints
                                onClicked: m.project.filterPoints = !m.project.filterPoints
                                clip: true
                            }
                            Slider  {
                                implicitWidth: 100
                                minimumValue: 1
                                maximumValue: 30
                                stepSize: 1
                                enabled: m.project && m.project.filterPoints
                                value: m.project.pointsFilteringThreshold
                                activeFocusOnPress: true
                                onValueChanged: {
                                    if(hovered || activeFocus) m.project.pointsFilteringThreshold = value
                                    value = Qt.binding(function() { return m.project.pointsFilteringThreshold } )
                                }
                            }
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: m.project.pointsFilteringThreshold + " +"
                                color: m.textColor
                            }
                        }
                    }
                }

                // Camera parameters
                CameraSettings {
                    implicitWidth: parent.width
                    project: m.project
                }

                // Version
                Text {
                    width: parent.width
                    text: "MayaMVG " + m.project.getPluginVersion()
                    color: m.textColor
                    font.italic: true
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
}
