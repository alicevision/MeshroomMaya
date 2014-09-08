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
    QtObject {
        id: m
        property variant project
        property bool isOpen
        property int thumbSize
        property int sliderMinValue
        property int sliderMaxValue
    }
    opacity: 0  // needed for animation

    StateGroup {
        id: openState
        state: "OPEN"
        states: [
            State {
                name: "CLOSED"
                PropertyChanges { target: settings; height: 0; }
                PropertyChanges { target: settings; opacity: 0; }
            },
            State {
                name: "OPEN"
                when: m.isOpen
                PropertyChanges { target: settings; height: 120; }
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
                    implicitHeight: 35
                    implicitWidth: parent.width

                    onBrowserProjectLoaded: settingProjectLoaded()
                }
                // thumbnail slider
                Item {
                    implicitHeight: 35
                    implicitWidth: parent.width
                    RowLayout {
                        anchors.fill: parent
                        Text {
                            text: "Thumbnail size"
                            color: "white"
                            font.pointSize: 11
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
                    implicitHeight: 35
                    implicitWidth: parent.width
                    property variant leftPanel: m.project.panelList.get(0)
                    property variant rightPanel: m.project.panelList.get(1)

                    RowLayout {
                        anchors.fill: parent

                        Text {
                            width: text.length
                            text: "Display point cloud : "
                            color: "white"
                            font.pointSize: 11
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
                            color: "white"
                            font.pointSize: 11
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
                            color: "white"
                            font.pointSize: 11
                            MouseArea {
                                anchors.fill: parent
                                onClicked: displayPointCloudItem.rightPanel.isPointCloudDisplayed = !pointCloudRCheckBox.checked
                            }
                        }
                    }
                }
            }
        }
    }
}
