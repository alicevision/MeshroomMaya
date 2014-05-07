import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: settings

    width: parent.width
    height: 100

    visible: true

    ColumnLayout {
        width: parent.width
        height: parent.height

        // Cores
        Item {
            id: cores

            width: parent.width
            implicitHeight: 30

            RowLayout {
                id: coresLayout

                width: parent.width
                height: parent.height
                spacing: 10

                Text {
                    id: coresLabel

                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    text: "Cores"
                    font.pointSize: 12
                    color: "white"
                }

                SpinBox {
                    id: coresSpinBox

                    minimumValue: 0
                    maximumValue: 30
                    value: 10
                    Layout.minimumWidth: 40
                    Layout.maximumWidth: 60
                    Layout.horizontalSizePolicy: Layout.Expanding

                    TooltipArea {
                        id: coresSpinBoxTooltip
                        anchors.fill: parent
                        text: coresLabel.text
                    }


                }

                Slider {
                    id: coresSlider

                    minimumValue: 0
                    maximumValue: 30
                    value: 10
                    Layout.minimumWidth: 100
                    Layout.maximumWidth: 200
                    Layout.horizontalSizePolicy: Layout.Expanding


                    TooltipArea {
                        id: coresSliderTooltip
                        anchors.fill: parent
                        text: coresLabel.text
                    }
                }

                Rectangle
                {
                    Layout.minimumWidth: 0
                    Layout.horizontalSizePolicy: Layout.Expanding
                    height: 30

                    color: "transparent"
                }
            }
        }

        // Quality
        Item {
            id: quality

            width: parent.width
            implicitHeight: 30


            RowLayout {
                id: qualityLayout

                width: parent.width
                height: parent.height
                spacing: 10

                Text {
                    id: qualityLabel

                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    text: "Quality"
                    font.pointSize: 12
                    color: "white"
                }

                ListModel {
                    id: qualityChoices
                    ListElement { text: "Low" }
                    ListElement { text: "Medium" }
                    ListElement { text: "High" }
                }


                ComboBox {
                    id: qualityComboBox

                    model:qualityChoices

                }

                TooltipArea {
                    id: qualityComboBoxTooltip
                    anchors.fill: qualityComboBox
                    text: qualityLabel.text
                }

                Rectangle
                {
                    Layout.horizontalSizePolicy: Layout.Expanding
                    implicitHeight: 30

                    color: "transparent"
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





