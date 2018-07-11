import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4

Item {
    id: cameraListView
    signal keyPressed(variant value)
    property alias project: m.project
    property alias thumbSize: m.thumbSize
    property alias currentIndex: m.currentIndex

    QtObject {
        id: m
        property variant project
        property int minThumbSize: 70
        property int maxThumbSize: 190
        property int thumbSize
        property int currentIndex
    }

//    function altColor(i) {
//        var colors = ["#262626", "#2f2f2f2f"];
//        if(i > colors.length)
//            return "#262626";
//        return colors[i];
//    }

    function selectCameras(oldIndex, newIndex) {
        listView.forceActiveFocus()
        var qlist = [];
        var begin = Math.min(oldIndex, newIndex);
        var end = Math.max(oldIndex, newIndex) + 1;

        for(var i = begin; i < end; ++i)
            qlist[qlist.length] = m.project.currentCameraSet.cameras.get(i).dagPath;

        m.project.addCamerasToIHMSelection(qlist);
        if(m.project.activeSynchro)
            m.project.addCamerasToMayaSelection(qlist);
    }

    function center(index, itemHeight, listView) {
        var itemY = index * itemHeight
        if(itemY > listView.contentHeight - listView.height)
            return listView.contentHeight - listView.height;
        else
            return itemY;
    }

    Connections {
        target: m.project
        onCenterCameraListByIndex: listView.contentY = center(cameraIndex , m.thumbSize, listView)
    }

    MDialog {
        id: deleteSetConfirmDialog
        anchors.fill: parent
        z: 1
        message: "Delete Camera Set '<b>" + m.project.currentCameraSet.name + "</b>' ?"
        Component.onCompleted: okButton.text = "Delete"
        onAccepted: m.project.deleteCameraSet(m.project.currentCameraSet)
    }

    CamSetCreationDialog {
        id: camSetCreationDialog
        anchors.fill: parent
        project: m.project
        z: 1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            Layout.fillWidth: true

            implicitHeight: childrenRect.height
            radius: 2
            color: "#393939"
            visible: m.project.projectDirectory !== ""

            Column {
                width: parent.width - 6
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 2
                RowLayout {
                    width: parent.width
                    height: 23
                    ToolButton {
                        implicitWidth: 23
                        implicitHeight: 23
                        iconSource: "img/swapViews.png"
                        tooltip: "Swap Views"
                        onClicked: m.project.swapViews()
                    }
                    ToolButton {
                        id: particleModeBtn
                        implicitHeight: parent.height
                        implicitWidth: implicitHeight
                        tooltip: "Filter Cameras from Particle Selection"
                        iconSource: "img/particleSelection.png"
                        checked: m.project.useParticleSelection
                        onClicked: m.project.useParticleSelection = !m.project.useParticleSelection
                        ButtonCheckIndicator {}
                    }
                    Row {
                        height: parent.height
                        spacing: 4
                        visible: m.project.useParticleSelection

                        Label {
                            text: "Min. Accuracy"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Slider  {
                            id: accuracySlider
                            implicitWidth: 90
                            minimumValue: 0
                            maximumValue: 100
                            stepSize: 1
                            value: m.project.particleSelectionAccuracy
                            onValueChanged: m.project.particleSelectionAccuracy = value
                        }

                        Text {
                            property int minAccuracy: Math.max(m.project.particleMaxAccuracy * (m.project.particleSelectionAccuracy/100), 1)
                            anchors.verticalCenter: parent.verticalCenter
                            color: "white"
                            text: minAccuracy + " / " + m.project.particleSelectionCount + " points"
                            visible: m.project.particleSelectionCount > 0
                        }
                    }

                    Item { Layout.fillWidth: true }
                }
                RowLayout {
                    implicitHeight: 23
                    width: parent.width - 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 4

                    Label {
                        text: "Camera Set :"
                    }
                    ComboBox {
                        id: cameraSetsCB

                        Layout.fillWidth: true
                        implicitHeight: parent.height
                        enabled: !m.project.useParticleSelection

                        model: ListModel { id: proxy }

                        Component.onCompleted: rebuildMenu()

                        currentIndex: m.project.currentCameraSetIndex
                        onActivated: m.project.currentCameraSetIndex = index

                        function rebuildMenu() {
                            proxy.clear()
                            for(var i = 0; i < m.project.cameraSets.count; ++i)
                                proxy.append({"text" : m.project.cameraSets.get(i).name})
                        }
                        Connections {
                            target: m.project.cameraSets
                            onCountChanged: cameraSetsCB.rebuildMenu()
                        }
                    }

                    ToolButton {
                        implicitWidth: 23
                        implicitHeight: 23
                        iconSource: "img/add_box.png"
                        tooltip: "Create new Camera Set"

                        onClicked: {
                            camSetCreationDialog.useSelection = m.project.cameraSelectionCount > 1
                            camSetCreationDialog.show()
                        }
                    }

                    ToolButton {
                        implicitWidth: 23
                        implicitHeight: 23
                        enabled: m.project.currentCameraSet.editable
                        iconSource: "img/delete.png"
                        tooltip: "Delete current Camera Set"
                        onClicked: {
                            deleteSetConfirmDialog.show()
                        }
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

        ListView {
            id: listView
            anchors.fill: parent
            model: m.project.currentCameraSet.cameras
            spacing: 1

            delegate: CameraItem {
                implicitWidth: listView.width - 4
                baseHeight: cameraListView.thumbSize
                camera: object
                project: cameraListView.project

                onSelection: {
                    cameraListView.currentIndex = index
                    selectCameras(index, index)
                }
                onMultipleSelection: selectCameras(cameraListView.currentIndex, index)
            }

            MouseArea {
                id: ma
                property variant clickedItem : null
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onPressed: {
                    var idx = listView.indexAt(mouse.x + listView.contentX, mouse.y + listView.contentY)
                    if(idx >= 0)
                    {
                        clickedItem = listView.model.get(idx)
                        selectCameras(idx, idx)
                        menu.popup()
                    }
                }
                onWheel: {
                    var step = 5
                    if(wheel.modifiers & Qt.ControlModifier)
                    {
                        var thumbSize = cameraListView.thumbSize + (wheel.angleDelta.y > 0 ? step : -step);
                        cameraListView.thumbSize = Math.max(m.minThumbSize, Math.min(thumbSize, m.maxThumbSize));
                    }
                    else
                        wheel.accepted = false
                }
            }

            Loader {
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 30

                property bool active: m.project.useParticleSelection && listView.count == 0
                sourceComponent: active ? particleSelectionHelper_component : null

                Component {
                    id: particleSelectionHelper_component
                    Column {
                        property color textColor: "#DDD"
                        spacing: 8
                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "img/no_particle_selection.png"
                        }

                        Text {
                            text: "<b>No particle selected</b>"
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pointSize: 11
                            color: "#EEE"
                        }
                        Text {
                            text: "<p>To keep the most accurate cameras for a specific area:</p>"
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pointSize: 10
                            color: textColor
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 4

                            Repeater {
                                model: ["Select relevant particles in the 3D viewport",
                                        "Filter cameras using the accuracy slider",
                                        "Create a new Camera Set with <img src='img/add_box.png' align='top'/> when done"]
                                delegate: Text {
                                    text: index + 1 + ". " + modelData
                                    font.pointSize: 10
                                    color: textColor
                                }
                            }
                        }
                    }
                }
            }

            Menu {
                id: menu
                style: MenuStyle {}
                MenuSeparator {}
                MenuItem {
                    text: "View in \"persp\""
                    onTriggered: m.project.setPerspFromCamera(ma.clickedItem)
                }
                MenuSeparator { }
                MenuItem {
                    text: "Select Camera(s) visible Points"
                    onTriggered: m.project.selectCamerasPoints()
                }
                MenuItem {
                    text: "Create Set from Selection"
                    onTriggered: {
                        camSetCreationDialog.useSelection = true
                        camSetCreationDialog.show()
                    }
                }
                MenuSeparator { }
                MenuItem {
                    text: "Open File"
                    onTriggered: {Qt.openUrlExternally(ma.clickedItem.imagePath)}
                }
            }
        }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 22
            radius: 2
            color: "#393939"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: anchors.leftMargin
                Image {
                    source: "img/thumbnailSize.png"
                    TooltipArea {
                        anchors.fill: parent
                        text: "Thumbnails Size"
                    }
                }
                Slider {
                    implicitWidth: 70
                    minimumValue: m.minThumbSize
                    maximumValue: m.maxThumbSize
                    value: m.thumbSize
                    onValueChanged: m.thumbSize = value
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: (m.project.cameraSelectionCount > 0 ? (m.project.cameraSelectionCount + " / ") : "") + listView.model.count + " camera(s)"
                    color: "#ccc"
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
    onKeyPressed: listView.keyPressed(value)
}
