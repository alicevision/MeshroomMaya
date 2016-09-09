import QtQuick 1.1
import QtDesktop 0.1


Item {
    property alias project: m.project
    signal browserProjectLoaded
    implicitHeight: childrenRect.height
    implicitWidth: 100

    QtObject {
        id: m
        property variant project
        property string directory: m.project.projectDirectory
    }

    RowLayout
    {
        width: parent.width
        height: childrenRect.height

        TextField {
            Layout.horizontalSizePolicy: Layout.Expanding
            text: m.directory
            readOnly: true
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
            onClicked: {
                var abcFile = m.project.openFileDialog()
                m.project.loadABC(abcFile)
            }
        }
    }   
}

