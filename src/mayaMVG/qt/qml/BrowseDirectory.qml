import QtQuick 1.1
import QtDesktop 0.1


Item {
    property alias project: m.project
    signal browserProjectLoaded
    QtObject {
        id: m
        property variant project
        property string directory: m.project.projectDirectory

    }
    RowLayout
    {
        width: parent.width
        height: parent.height
        Rectangle {
            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            color: "grey"
            radius: 2
            TextInput {
                anchors.fill: parent
                text: m.directory
                readOnly: true
                selectByMouse: true
                font.pointSize: 13
                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        var projectPath = m.project.openFileDialog()
                        m.project.loadNewProject(projectPath)
                        browserProjectLoaded()
                    }
                }
            }
            TooltipArea {
                anchors.fill: parent
                text: "Project file (.abc)"
            }
        }
        ToolButton {
            implicitWidth: 30
            height: 30
            iconSource: "img/Folder.png"
            tooltip: "Select project file (.abc)"
            MouseArea {
                id: folderButtonMouseArea
                anchors.fill: parent
                onClicked: {
                    var abcFile = m.project.openFileDialog()
                    m.project.loadABC(abcFile)
                }
            }
        }
    }   
}

