import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1


Item {
    property alias project: m.project
    property alias title: titleLabel.text
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
        Label {
            id: titleLabel
            visible: text.trim() != ""
        }

        MTextField {
            Layout.fillWidth: true
            text: m.directory
            readOnly: true
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

