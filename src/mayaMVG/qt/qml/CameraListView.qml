import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: cameraListView
    signal keyPressed(variant value)
    property alias project: m.project
    property alias thumbSize: m.thumbSize
    property alias currentIndex: m.currentIndex

    QtObject {
        id: m
        property variant project
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
        width: parent.width
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
                    id: particleModeBtn
                    implicitHeight: parent.height
                    implicitWidth: implicitHeight
                    tooltip: "Select Particles"
                    iconSource: "img/particleSelection.png"
                    iconSize: 18
                    checked: m.project.useParticleSelection
                    onClicked: {
                        m.project.useParticleSelection = !m.project.useParticleSelection
                    }
                }
                Slider  {
                    id: toleranceSlider
                    implicitWidth: 90
                    enabled: m.project.useParticleSelection
                    minimumValue: 0
                    maximumValue: 100
                    stepSize: 1
                    value: m.project.particleSelectionTolerance
                    onValueChanged: m.project.particleSelectionTolerance = value
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: m.project.useParticleSelection ? "white" : "#BBB"
                    text: toleranceSlider.value + "%"
                }

                Item { Layout.horizontalSizePolicy: Layout.Expanding }
            }
            RowLayout {
                height: 23
                width: parent.width
                spacing: 4
                ToolButton {
                    implicitWidth: 23
                    height: 23
                    iconSource: "img/add_box.png"
                    tooltip: "Create new Camera Set"

                    onClicked: {
                        camSetCreationDialog.useSelection = m.project.cameraSelectionCount > 1
                        camSetCreationDialog.show()
                    }
                }

                Item {
                    height: parent.height
                    Layout.horizontalSizePolicy: Layout.Expanding

                    ComboBox {
                        id: cameraSetsCB
                        width: parent.width
                        anchors.verticalCenter: parent.verticalCenter
                        activeFocusOnPress: true
                        enabled: !m.project.useParticleSelection

                        model:  ListModel { id: proxy }

                        Component.onCompleted: rebuildMenu()

                        selectedIndex: m.project.currentCameraSetIndex
                        selectedText: m.project.currentCameraSet.name
                        onSelectedIndexChanged: {
                            if(m.project.currentCameraSetIndex !== selectedIndex)
                                m.project.currentCameraSetIndex = selectedIndex
                        }

                        function rebuildMenu() {
                            proxy.clear()
                            for(var i = 0; i < m.project.cameraSets.count; ++i)
                                proxy.append({"text" : m.project.cameraSets.get(i).name})
                            // Only way to force combobox's menu update...
                            data[5].rebuildMenu()
                        }
                    }

                    Connections {
                        target: m.project.cameraSets
                        onCountChanged: cameraSetsCB.rebuildMenu()
                    }
                }

                ToolButton {
                    implicitWidth: 23
                    height: 23
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

    CustomListView {
        id: listView
        width: parent.width
        Layout.verticalSizePolicy: Layout.Expanding
        model: m.project.currentCameraSet.cameras

        delegate: Component {

            CameraItem {
                width: listView.width
                baseHeight: cameraListView.thumbSize
                camera: object
                project: cameraListView.project
                onSelection: {
                    cameraListView.currentIndex = index
                    selectCameras(index, index)
                }
                onMultipleSelection: selectCameras(cameraListView.currentIndex, index)
            }
        }

        MouseArea {
            id: ma
            property variant clickedItem : null
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onPressed: {
                var pos = mapToItem(null, mouse.x, mouse.y)
                var idx = listView.listView.indexAt(mouse.x + listView.listView.contentX, mouse.y + listView.listView.contentY)
                if(idx >= 0)
                {
                    clickedItem = listView.model.get(idx)
                    if(!clickedItem.isSelected)
                        selectCameras(idx, idx)
                    menu.showPopup(pos.x, pos.y)
                }
            }
        }
        ContextMenu {
            id: menu
            Separator {}
            MenuItem {
                text: "Open File"
                onTriggered: {Qt.openUrlExternally(ma.clickedItem.imagePath)}
            }
            MenuItem {
                text: "Create Set from Selection"
                onTriggered: {
                    camSetCreationDialog.useSelection = true
                    camSetCreationDialog.show()
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        implicitHeight: 22
        radius: 2
        color: "#393939"
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: anchors.leftMargin
            Image {
                source: "img/thumbnailSize.png"
            }
            Slider {
                implicitWidth: 70
                minimumValue: 70
                maximumValue: 190
                value: 70
                onValueChanged: m.thumbSize = value
            }

            Item { Layout.horizontalSizePolicy: Layout.Expanding }

            Text {
                id: txt
                text: (m.project.cameraSelectionCount > 0 ? (m.project.cameraSelectionCount + " / ") : "") + listView.model.count + " camera(s)"
                color: "#ccc"
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    }
    onKeyPressed: listView.keyPressed(value)
}
