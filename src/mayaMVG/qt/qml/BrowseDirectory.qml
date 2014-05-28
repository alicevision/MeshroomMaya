import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: browseDirectory

    RowLayout
    {
        id: browseDirectoryRowLayout

        width: parent.width
        height: parent.height

        // Directory TextEdit
        Rectangle {
            id: directoryRectangle

            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            color: "grey"
            radius: 2

            TextInput{
                id: directoryTextEdit

                anchors.fill: parent
                focus: false
                color: "black"
                text: _project.projectDirectory
                //verticalAlignment: Text.AlignVCenter
                selectionColor: "grey"
                selectedTextColor: "white"
                selectByMouse: true
                font.pointSize: 13


                onAccepted: _project.loadProject(text)
            }

            TooltipArea {
                id: qualityComboBoxTooltip
                anchors.fill: parent
                text: "Project directory path"
            }
        }

        // Folder button
        ToolButton {
            id: folderButton

            implicitWidth: 30
            height: 30
            iconSource: "img/Folder.png"
            tooltip: "Browse project folder"

            MouseArea {
                id: folderButtonMouseArea
                anchors.fill: parent

                onClicked: _project.onBrowseDirectoryButtonClicked()
            }
        }
    }
}

