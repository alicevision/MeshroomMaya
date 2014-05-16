import QtQuick 1.1
import QtDesktop 0.1

Rectangle {
    id: main
    color: "transparent"
    width: 458
    height: 397

    property string textColor: "white"
    property int thumbSize: 90


    // MouseArea
    MouseArea {
        id: mainMouseArea

        hoverEnabled: true
        anchors.bottom: parent.bottom
        height: 50
        width: parent.width

        onEntered: logArea.visible = true
        onExited: logArea.visible = false
    }

    ColumnLayout
    {
        id: mainLayout

        width: parent.width
        height: parent.height

        // ContextBar
        ContextBar
        {
            id: contextBar

            width: parent.width
            //height: 30
            implicitHeight: 30
        }


        // ContextSettings
        Params
        {
            id: params

            anchors.top: contextBar.bottom
            width: parent.width
            //height:130
            implicitHeight: 130
            visible: contextBar.settingsVisibility
        }


        // Camera list
        CameraList {
            id: cameraList

            implicitHeight: parent.height
            width: parent.width

            anchors.top: params.bottom
           //height: parent.height - contextBar.height - params.height
            Layout.verticalSizePolicy: Layout.Expanding

        }

    } // ColumnLayout

    // Timer for log area
    Timer {
        id: timer
        interval: 5000
        running: false
        repeat: false

        onTriggered: logArea.visible = true

        triggeredOnStart: true

        onRunningChanged: {
            if(!running)
                logArea.visible = false
        }

    }

    // Log
    TextArea {
        id: logArea
        readOnly: true
        width: parent.width - 20

        opacity: 0.7

        anchors {
            bottom: parent.bottom
            left: parent.left

            bottomMargin: 10
            leftMargin: 10

        }

        text: _project.logText
        onTextChanged: timer.restart();
        color: "#fec04c"


    }


}
