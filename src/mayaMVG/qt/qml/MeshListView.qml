import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: meshListView
    signal keyPressed(variant value)
    property alias project: m.project
    property alias currentIndex: m.currentIndex
    QtObject {
        id: m
        property variant project
        property int currentIndex
    }

    function altColor(i) {
        var colors = [ "#262626", "#2f2f2f2f" ];
        return colors[i];
    }


    Component
    {
        id: meshComponent
        MeshItem {
            width: listView.width - 12 // ScrollBar height
            height: 75
            mesh: model.modelData
            project: meshListView.project
            Component.onCompleted: color = altColor(index%2)
        }

    }

    CustomListView {
        id: listView
        anchors.fill: parent
        model: m.project.meshModel
        delegate: meshComponent
    }

    onKeyPressed: listView.keyPressed(value)
}
