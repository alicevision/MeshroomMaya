import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: settings
    property alias project: m.project
    property alias isOpen: m.isOpen
    property alias thumbSize: m.thumbSize
    property alias sliderMinValue: m.sliderMinValue
    property alias sliderMaxValue: m.sliderMaxValue
    signal settingProjectLoaded

    // UI params
    property int labelWidth: 160
    property int fieldWidth: 50
    property int fieldMargin: 4
    property int buttonWidth: 50

    QtObject {
        id: m
        property variant project
        property bool isOpen
        property int thumbSize
        property int sliderMinValue
        property int sliderMaxValue
        property color textColor: "white"
        property int textSize: 11
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
                PropertyChanges { target: settings; height: 380; }
                PropertyChanges { target: settings; opacity: 1; }
            }
        ]
        transitions: Transition {
            PropertyAnimation { target: settings; properties: "height"; duration: 300 }
            PropertyAnimation { target: settings; properties: "opacity"; duration: 200 }
        }
    }

    Item { // needed to set margins around GroupBox
        clip: true
        anchors.fill: parent
        GroupBox {
            title: "Settings"
            anchors.fill: parent
            anchors.margins: 5
            ColumnLayout {
                anchors.fill: parent
                // browse button
                BrowseDirectory {
                    project: m.project
                    Layout.minimumHeight: 25
                    implicitWidth: parent.width
                    onBrowserProjectLoaded: settingProjectLoaded()
                }

                // Toolbar
                Item {
                    implicitWidth: parent.width
                    Layout.minimumHeight: 25
                    RowLayout {
                        anchors.fill: parent
                        // Clear blind data
                        Item {
                            implicitWidth: 30
                            implicitHeight: 30
                            Layout.minimumHeight: 30
                            ToolButton {
                                iconSource: "img/clearBD.png"
                                height: parent.height
                                width: parent.height
                                tooltip: "Clear all 2D points (triangulation)"
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: m.project.clearAllBlindData()
                                }
                            }
                        }
                        // Select closest cam from persp
                        Item {
                            implicitWidth: 30
                            implicitHeight: 30
                            Layout.minimumHeight: 30

                            ToolButton {
                                iconSource: "img/greenCamera.png"
                                tooltip: "Select closest cam"
                                height: parent.height
                                width: parent.height
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: m.project.selectClosestCam()
                                }
                            }
                        }

                        // Reorient scene
                        Item {
                            implicitWidth: 30
                            implicitHeight: 30
                            Layout.minimumHeight: 30

                            ToolButton {
                                iconSource: "img/locatorMode.png"
                                tooltip: "Reorient scene"
                                height: parent.height
                                width: parent.height
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: m.project.applySceneTransformation()
                                }
                            }
                        }
                        Rectangle {
                            Layout.horizontalSizePolicy: Layout.Expanding
                        }
                    }
                }

                // thumbnail slider
                Item {
                    Layout.minimumHeight: 25
                    implicitWidth: parent.width
                    RowLayout {
                        anchors.fill: parent
                        Item {
                            implicitWidth: labelWidth
                            implicitHeight: parent.height
                            Text {
                                text: "Thumbnail size"
                                color: m.textColor
                                font.pointSize: m.textSize
                            }
                        }
                        Slider {
                            Layout.horizontalSizePolicy: Layout.Expanding
                            minimumValue: m.sliderMinValue
                            maximumValue: m.sliderMaxValue
                            value: m.thumbSize
                            onValueChanged: {
                                m.thumbSize = value
                            }
                        }
                    }

                    TooltipArea {
                        anchors.fill: parent
                        text: "Thumbnail size"
                    }
                }

                Item {
                    id: displayPointCloudItem
                    Layout.minimumHeight: 25
                    implicitWidth: parent.width
                    property variant leftPanel: m.project.panelList.get(0)
                    property variant rightPanel: m.project.panelList.get(1)

                    RowLayout {
                        anchors.fill: parent
                        Item {
                            implicitWidth: labelWidth
                            implicitHeight: parent.height

                            Text {
                                text: "Display point cloud"
                                color: m.textColor
                                font.pointSize: m.textSize
                            }
                        }
                        
                        CheckBox {
                            id: pointCloudLCheckBox
                            implicitHeight: parent.height
                            implicitWidth: parent.height
                            checked: displayPointCloudItem.leftPanel.isPointCloudDisplayed
                            MouseArea {
                                anchors.fill: parent
                                onClicked: displayPointCloudItem.leftPanel.isPointCloudDisplayed = !pointCloudLCheckBox.checked
                            }
                        }
                        Text {
                            width: text.length
                            text: displayPointCloudItem.leftPanel.label
                            color: m.textColor
                            font.pointSize: m.textSize
                            MouseArea {
                                anchors.fill: parent
                                onClicked: displayPointCloudItem.leftPanel.isPointCloudDisplayed = !pointCloudLCheckBox.checked
                            }
                        }
                        CheckBox {
                            id: pointCloudRCheckBox
                            implicitHeight: parent.height
                            implicitWidth: parent.height
                            checked: displayPointCloudItem.rightPanel.isPointCloudDisplayed
                            MouseArea {
                                anchors.fill: parent
                                onClicked: displayPointCloudItem.rightPanel.isPointCloudDisplayed = !pointCloudRCheckBox.checked
                            }
                        }
                        Text {
                            Layout.horizontalSizePolicy: Layout.Expanding
                            width: text.length
                            text: displayPointCloudItem.rightPanel.label
                            color: m.textColor
                            font.pointSize: m.textSize
                            MouseArea {
                                anchors.fill: parent
                                onClicked: displayPointCloudItem.rightPanel.isPointCloudDisplayed = !pointCloudRCheckBox.checked
                            }
                        }
                    }
                }

                // Active synchro
                Item {
                    implicitWidth: parent.width
                    Layout.minimumHeight: 25
                    RowLayout {
                        anchors.fill: parent
                        Item {
                            implicitWidth: labelWidth
                            implicitHeight: parent.height
                            Text {
                                text: "Active synchronization"
                                verticalAlignment: Text.AlignVCenter
                                color: m.textColor
                                font.pointSize: m.textSize
                            }
                        }
                        CheckBox {
                            id: cameraSynchroCheckBox
                            implicitHeight: parent.height
                            implicitWidth: parent.height
                            checked: m.project.activeSynchro
                            
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: m.project.activeSynchro = !cameraSynchroCheckBox.checked
                        }

                        Rectangle {
                            implicitWidth: 80
                            Layout.horizontalSizePolicy: Layout.Expanding
                            color: "blue"
                        }
                    }
                    TooltipArea {
                        anchors.fill: parent
                        text: "Camera synchronization"
                    }
                }

                // Camera parameters
                CameraSettings {
                    implicitWidth: parent.width
                    Layout.minimumHeight: 180
                    project: m.project
                }

                // Version
                Item
                {
                    implicitWidth: parent.width
                    Layout.minimumHeight: 25
                    Text {
                        anchors.fill: parent
                        text: "MayaMVG " + m.project.getPluginVersion()
                        color: m.textColor
                        font.italic: true
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }
    }
}
