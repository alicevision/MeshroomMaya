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
        spacing: 0

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
                    implicitWidth: 15

                    checked: _project.computeLastPoint
                    onClicked: _project.onComputeLastPointCheckBoxClicked(checked)
                }

                Text {
                    id: computeLastPointLabel

                    text: "Compute last point"
                    color: "white"
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
                    implicitWidth: 15

                    checked: _project.connectFace
                    onClicked: _project.onConnectFaceCheckBoxClicked(checked)
                }

                Text {
                    id: connectFaceLabel

                    text: "Connect face"
                    color: "white"
                    font.pointSize: 11
                    horizontalAlignment: Text.AlignLeft
                    Layout.horizontalSizePolicy: Layout.Expanding
                }
            }
        }


//        // Cores
//        Item {
//            id: cores

//            width: parent.width
//            implicitHeight: 30

//            RowLayout {
//                id: coresLayout

//                width: parent.width
//                height: parent.height
//                spacing: 10

//                Text {
//                    id: coresLabel

//                    height: parent.height
//                    verticalAlignment: Text.AlignVCenter
//                    text: "Cores"
//                    font.pointSize: 12
//                    color: "white"
//                }

//                SpinBox {
//                    id: coresSpinBox

//                    minimumValue: 0
//                    maximumValue: 30
//                    value: 10
//                    Layout.minimumWidth: 40
//                    Layout.maximumWidth: 60
//                    Layout.horizontalSizePolicy: Layout.Expanding

//                    TooltipArea {
//                        id: coresSpinBoxTooltip
//                        anchors.fill: parent
//                        text: coresLabel.text
//                    }


//                }

//                Slider {
//                    id: coresSlider

//                    minimumValue: 0
//                    maximumValue: 30
//                    value: 10
//                    Layout.minimumWidth: 100
//                    Layout.maximumWidth: 200
//                    Layout.horizontalSizePolicy: Layout.Expanding


//                    TooltipArea {
//                        id: coresSliderTooltip
//                        anchors.fill: parent
//                        text: coresLabel.text
//                    }
//                }

//                Rectangle
//                {
//                    Layout.minimumWidth: 0
//                    Layout.horizontalSizePolicy: Layout.Expanding
//                    height: 30

//                    color: "transparent"
//                }
//            }
//        }

//        // Quality
//        Item {
//            id: quality

//            width: parent.width
//            implicitHeight: 30


//            RowLayout {
//                id: qualityLayout

//                width: parent.width
//                height: parent.height
//                spacing: 10

//                Text {
//                    id: qualityLabel

//                    height: parent.height
//                    verticalAlignment: Text.AlignVCenter
//                    text: "Quality"
//                    font.pointSize: 12
//                    color: "white"
//                }

//                ListModel {
//                    id: qualityChoices
//                    ListElement { text: "Low" }
//                    ListElement { text: "Medium" }
//                    ListElement { text: "High" }
//                }


//                ComboBox {
//                    id: qualityComboBox

//                    model:qualityChoices

//                }

//                TooltipArea {
//                    id: qualityComboBoxTooltip
//                    anchors.fill: qualityComboBox
//                    text: qualityLabel.text
//                }

//                Rectangle
//                {
//                    Layout.horizontalSizePolicy: Layout.Expanding
//                    implicitHeight: 30

//                    color: "transparent"
//                }
//            }

//        }

        Rectangle
        {
            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding

            color: "transparent"
        }




    }
}





