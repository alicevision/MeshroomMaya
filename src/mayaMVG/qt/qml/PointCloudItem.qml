import QtQuick 1.1
import QtDesktop 0.1

GroupBox {
    id: pointCloudItem

    flat: false
    title: "PointCloud"

    ColumnLayout {
        width: parent.width
        height: parent.height

        Item {
            width: parent.width
            implicitHeight: 20

            RowLayout {
                width: parent.width
                height: parent.height

                Button {
                    text: "Load dense pointCloud"
                    tooltip: "Load dense pointCloud"
                }
                Rectangle {
                    Layout.horizontalSizePolicy: Layout.Expanding
                    height: parent.height
                    color: "transparent"
                }
            }
        }

        // Path
        Item {
            id: pathItem
            width: parent.width
            implicitHeight: 20

            Text {
                id: pointCloudPath

                text: _project.pointCloudFile
                elide: Text.ElideLeft
                width: parent.width
                anchors.left: parent.left
                font.pointSize: 10
                color: main.textColor
            }

            TooltipArea {
                id: pathItemTooltip
                anchors.fill: parent
                text: "PointCloud path"
            }
        }

        // Color checkBox
        Item {
            id: coloredPointCloud

            width: parent.width
            implicitHeight: 10

            RowLayout {
                id: coloredPointCloudLayout

                width: parent.width
                height: parent.height

                CheckBox {
                    id: coloredPointCloudCheckBox
                    implicitWidth: 20
                    checked: false

                }

                Text {
                    id: computeLastPointLabel

                    text: "Colored point cloud"
                    color: main.textColor
                    font.pointSize: 11
                    horizontalAlignment: Text.AlignLeft
                    Layout.horizontalSizePolicy: Layout.Expanding
                }

            }

            TooltipArea {
                id: computeLastPointTooltip
                anchors.fill: parent
                text: "Colored pointCloud checkbox"
            }
        }

        // Progress bar
        ProgressBar {
            id: progressbar

            width: parent.width
            implicitHeight: 10
            value: 30
            maximumValue: 100


            TooltipArea {
                id:progressbarTooltip
                anchors.fill: parent
                text: "Reconstruction progress"
            }
        }
    }


}
