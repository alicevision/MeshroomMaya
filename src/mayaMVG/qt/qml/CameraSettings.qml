import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraSettings
    property alias project: m.project
    QtObject {
        id: m
        property variant project
        property color textColor: "white"
        property int textSize: 11
    }

    Item { // needed to set margins around GroupBox
        clip: true
        anchors.fill: parent
        GroupBox {
            title: "Cameras"
            anchors.fill: parent
            anchors.margins: 5
            ColumnLayout {
                anchors.fill: parent
                Item {
                    width: parent.width
                    implicitHeight: 25
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 1
                        Text {
                            width: 25
                            height: parent.height
                            text: "Near : "
                            verticalAlignment: Text.AlignVCenter
                            color: m.textColor
                            font.pointSize: m.textSize
                        }
                        Rectangle {
                            implicitWidth: 50
                            height: parent.height - 2
                            color: "grey"
                            radius: 2

                            TextInput {
                                id: nearValue
                                width: parent.width
                                horizontalAlignment: TextInput.AlignHCenter
                                anchors.verticalCenter: parent.verticalCenter
                                selectByMouse: true
                                font.pointSize: m.textSize
                                validator: DoubleValidator{bottom: 0.001;}
                            }
                        }
                        Button {
                            text: "Set"
                            height: parent.height
                            implicitWidth: 30
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    m.project.setCamerasNear(nearValue.text)
                                    nearValue.text = ""
                                }

                            }
                        }
                        Rectangle {
                            Layout.horizontalSizePolicy: Layout.Expanding
                            color: "blue"
                        }
                    }

                    TooltipArea {
                        anchors.fill: parent
                        text: "Camera near"
                    }
                }
                Item {
                    width: parent.width
                    implicitHeight: 25
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 1
                        Text {
                            width: 25
                            height: parent.height
                            text: "Far :   "
                            verticalAlignment: Text.AlignVCenter
                            color: m.textColor
                            font.pointSize: m.textSize
                        }
                        Rectangle {
                            implicitWidth: 50
                            height: parent.height - 2
                            color: "grey"
                            radius: 2

                            TextInput {
                                id: farValue
                                width: parent.width
                                horizontalAlignment: TextInput.AlignHCenter
                                anchors.verticalCenter: parent.verticalCenter
                                selectByMouse: true
                                font.pointSize: m.textSize
                                validator: DoubleValidator{bottom: 0.001;}
                            }
                        }
                        Button {
                            text: "Set"
                            height: parent.height
                            implicitWidth: 30
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    m.project.setCamerasFar(farValue.text)
                                    farValue.text = ""
                                }
                            }
                        }
                        Rectangle {
                            Layout.horizontalSizePolicy: Layout.Expanding
                            color: "blue"
                        }
                    }

                    TooltipArea {
                        anchors.fill: parent
                        text: "Camera far"
                    }
                }
                Item {
                    width: parent.width
                    implicitHeight: 25
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 1
                        Text {
                            width: 25
                            height: parent.height
                            text: "Locator scale :   "
                            verticalAlignment: Text.AlignVCenter
                            color: m.textColor
                            font.pointSize: m.textSize
                        }
                        Rectangle {
                            implicitWidth: 35
                            height: parent.height - 2
                            color: "grey"
                            radius: 2

                            TextInput {
                                id: locatorScaleValue
                                width: parent.width
                                horizontalAlignment: TextInput.AlignHCenter
                                anchors.verticalCenter: parent.verticalCenter
                                selectByMouse: true
                                font.pointSize: m.textSize
                                validator: DoubleValidator{bottom: 0.0;}
                            }
                        }
                        Button {
                            text: "Set"
                            height: parent.height
                            implicitWidth: 30
                            MouseArea {
                                anchors.fill: parent
                                onClicked:
                                {
                                    m.project.setCameraLocatorScale(locatorScaleValue.text)
                                    locatorScaleValue.text = ""
                                }
                            }
                        }
                        Rectangle {
                            Layout.horizontalSizePolicy: Layout.Expanding
                            color: "blue"
                        }
                    }

                    TooltipArea {
                        anchors.fill: parent
                        text: "Camera locator scale"
                    }
                }
            }
        }
    }
}
