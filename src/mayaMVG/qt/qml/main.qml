import QtQuick 1.1
import MyTools 1.0
import QtDesktop 0.1

Item {
   
    ColumnLayout
    {
        anchors.fill: parent
        ContextBar {
            id: contextBar
            implicitHeight: 35
            implicitWidth: parent.width
            project: _project
            settingsVisibility: (_project.projectDirectory === "")
        }
        ProjectSettings {
            id: settings
            implicitWidth: parent.width
            Layout.minimumHeight: childrenRect.height
            Layout.maximumHeight: childrenRect.height
            Layout.verticalSizePolicy: Layout.Expanding
            isOpen: contextBar.settingsVisibility
            project: _project
            sliderMinValue: 90
            sliderMaxValue: 180
            onSettingProjectLoaded: contextBar.settingsVisibility = false
            thumbSize: sliderMinValue
        }
        // PointCloudItem {
        //     implicitWidth: parent.width
        //     Layout.minimumHeight: childrenRect.height
        //     Layout.maximumHeight: childrenRect.height
        //     Layout.verticalSizePolicy: Layout.Expanding
        //     thumbSize: settings.thumbSize
        // }
        CameraList {
            implicitWidth: parent.width
            Layout.verticalSizePolicy: Layout.Expanding
            thumbSize: settings.thumbSize
            project: _project
        }
    }

//    // Log
//    LogText {
//        id: logText
//        width: parent.width - 20
//        height: 120
//        visible: false

//        anchors {
//            bottom: parent.bottom
//            left: parent.left

//            bottomMargin: 10
//            leftMargin: 10
//        }
//        text: _project.logText
//    }

//    // Open/Close LogText
//    Rectangle {
//        width: 20
//        height: 20
//        color: "#fec04c"
//        opacity: 0.7

//        anchors {
//            bottom: parent.bottom
//            right: parent.right

//        }
//        ToolButton {
//            anchors.fill: parent
//            iconSource: "img/up_left_arrow.png"
//            tooltip: "Open error log"
//            onClicked: logText.visible = true
//        }
//    }

    CustomWheelArea {
        id: wheelArea
        anchors.fill: parent

        QtObject {
            id: m
            property int step: 5
        }

        onVerticalWheel: {
            if(modifier & Qt.ControlModifier)
            {
                if(delta > 0 && settings.thumbSize < settings.sliderMaxValue)
                {
                   settings.thumbSize += m.step;
                }
                else if (delta < 0 && settings.thumbSize > settings.sliderMinValue){
                    settings.thumbSize -= m.step;
                }
                wheelArea.eventAccept()
            }
            else
            {
                wheelArea.eventIgnore()
            }
        }
    }
}
