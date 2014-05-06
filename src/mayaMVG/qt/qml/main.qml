import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: main
    color: "transparent"
    width: 458
    height: 397


    ColumnLayout
    {
        id: mainLayout

        width: parent.width
        height: parent.height

        // Directory
        BrowseDirectory
        {
            id: browseDirectory

            width: parent.width
            implicitHeight: 30
        }

        // Context
        Context
        {
            id: context

            width: parent.width
            implicitHeight: 120
        }

        // CameraList
        Item {
            id: cameraListItem

            width: parent.width
            Layout.verticalSizePolicy: Layout.Expanding

            RowLayout
            {

                width: parent.width
                height: parent.height

                // Camera list
                CameraList {
                    id: cameraList

                    height: parent.height
                    implicitWidth: parent.width

                }
            }
        }
    }
}
