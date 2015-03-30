import QtQuick 1.1
import MyTools 1.0
import QtDesktop 0.1

Item {

    // Needed to detect top focus changed
    TopFocusHandler {
        anchors.fill: parent
    }

    ColumnLayout
    {
        anchors.fill: parent
        ContextBar {
            id: contextBar
            implicitHeight: 35
            implicitWidth: parent.width
            project: _project
            settingsVisibility: (_project.projectDirectory === "")

            onShowMeshSignal: {
                // To avoir errors when switching lists
                componentList.model = "None";
                componentList.delegate = meshComponent;
                componentList.model = _project.meshModel;

            }
            onShowCameraSignal: {
                // To avoir errors when switching lists
                componentList.model = "None";
                componentList.delegate = cameraComponent;
                componentList.model = _project.cameraModel;

            }
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
        Component
        {
            id: cameraComponent
            CameraItem {
                width: componentList.width
                baseHeight: m.thumbSize
                camera: model.modelData
                project: m.project
                onSelection: {
                    m.currentIndex = index
                    selectCameras(index, index)
                }
                onMultipleSelection: selectCameras(m.currentIndex, index)
                Component.onCompleted: color = altColor(index%2)
            }
        }

        Component
        {
            id: meshComponent
            MeshItem {
                width: componentList.width - 12 // ScrollBar height
                height: 75
                mesh: model.modelData
                project: m.project
                Component.onCompleted: color = altColor(index%2)
            }

        }

        MVGList {
            id: componentList
            implicitWidth: parent.width
            Layout.verticalSizePolicy: Layout.Expanding
            thumbSize: settings.thumbSize
            project: _project
            model: _project.cameraModel
            delegate: cameraComponent
        }
    }

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
    Keys.onPressed: componentList.keyPressed(event.key)
}
