import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: settings

    width: parent.width
    height: 100

    visible: true
    state: heightStateGroup.state

    StateGroup {
        id: heightStateGroup

        states: [
            State {
                name: "OPENED"
                when: (visible)
                PropertyChanges {target: settings; height:130}
            },
            State {
                name: "CLOSED"
                when: (!visible)
                PropertyChanges {target: settings; height:0}
            }

        ]
    }

    ColumnLayout {
        width: parent.width
        height: parent.height
        spacing: 0

        // Directory
        BrowseDirectory
        {
            id: browseDirectory

            width: parent.width
            implicitHeight: 30
        }

        // ComputeLastPoint
        Item {
            id: computeLastPoint

            width: parent.width
            implicitHeight: 30

            RowLayout {
                id: computeLastPointLayout

                width: parent.width
                height: parent.height

                CheckBox {
                    id: computeLastPointCheckBox
//                    checked: true
                    implicitWidth: 20

                    checked: _project.computeLastPoint
                    onClicked: _project.onComputeLastPointCheckBoxClicked(checked)
                }

                Text {
                    id: computeLastPointLabel

                    text: "Compute last point"
                    color: main.textColor
                    font.pointSize: 11
                    horizontalAlignment: Text.AlignLeft
                    Layout.horizontalSizePolicy: Layout.Expanding
                }
            }
        }

        // ConnectFace
        Item {
            id: connectFace

            width: parent.width
            implicitHeight: 30

            RowLayout {
                id: connectFaceLayout

                width: parent.width
                height: parent.height

                CheckBox {
                    id: connectFaceCheckBox
//                    checked: true
                    implicitWidth: 20
                    checked: _project.connectFace
                    onClicked: _project.onConnectFaceCheckBoxClicked(checked)
                }

                Text {
                    id: connectFaceLabel

                    text: "Connect face"
                    color: main.textColor
                    font.pointSize: 11
                    horizontalAlignment: Text.AlignLeft
                    Layout.horizontalSizePolicy: Layout.Expanding
                }
            }
        }

        Item {
            width: parent.width
            implicitHeight: 30

            RowLayout {
                id: sliderLayout

                width: parent.width
                height: parent.height
                spacing: 15

                // Thumbnail size
                Slider {
                    id: thumbSizeSlider

                    implicitWidth: parent.width/3
                    Layout.minimumWidth: 10
                    Layout.maximumWidth: parent.width/2

                    minimumValue: 90
                    maximumValue: 200
                    value: main.thumbSize

                    onValueChanged: {
                        main.thumbSize = value
                    }

                }

                Text {
                    id: thumbSizeSliderLabel

                    Layout.horizontalSizePolicy: Layout.Expanding
                    text: "Thumbnail size"
                    color: main.textColor
                    font.pointSize: 11
                }
            }
        }




        Rectangle
        {
            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding

            color: "transparent"
        }




    }
}





