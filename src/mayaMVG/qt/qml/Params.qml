import QtQuick 1.1
import QtDesktop 0.1

GroupBox {
    id: settings

    flat: false
    title: "Settings"
    anchors.margins: 2

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Directory
        BrowseDirectory
        {
            id: browseDirectory

            width: parent.width
            implicitHeight: 30
        }

        Item {
            width: parent.width
            implicitHeight: 30

            RowLayout {
                id: sliderLayout

                anchors.fill: parent
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





