import QtQuick 1.1
import QtDesktop 0.1

GroupBox {
    id: settings

    flat: false
    title: "Settings"
    anchors.margins: 2

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

            TooltipArea {
                id: computeLastPointTooltip
                anchors.fill: parent
                text: "Compute last point checkbox"
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

            TooltipArea {
                id: connectFaceTooltip
                anchors.fill: parent
                text: "Connect face checkbox"
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

            TooltipArea {
                id: thumbSizeTooltip
                anchors.fill: parent
                text: "Thumbnail size slider"
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





