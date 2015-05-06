import QtQuick 1.1
import QtDesktop 0.1


Item {
    id: meshListView
    signal keyPressed(variant value)
    property alias project: m.project
    property alias currentIndex: m.currentIndex
    property alias itemHeight: m.itemHeight
    QtObject {
        id: m
        property variant project
        property int currentIndex
        property int itemHeight : 75
    }

//    function altColor(i) {
//        var colors = [ "#262626", "#2f2f2f2f" ];
//        if(i > colors.length)
//            return "#262626";
//        return colors[i];
//    }

    function selectMeshes(oldIndex, newIndex) {
        var qlist = [];
        var begin = Math.min(oldIndex, newIndex);
        var end = Math.max(oldIndex, newIndex) + 1;

        for(var i = begin; i < end; ++i)
        {
            qlist[qlist.length] = m.project.meshModel.get(i).dagPath;
        }
        m.project.addMeshesToIHMSelection(qlist);
        m.project.addMeshesToMayaSelection(qlist);
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
         onCenterMeshListByIndex: listView.contentY = center(meshIndex, m.itemHeight, listView)
     }

    Component
    {
        id: meshComponent
        MeshItem {
            width: listView.width - 12 // ScrollBar height
            height: meshListView.itemHeight
            mesh: model.modelData
            project: meshListView.project
            onSelection: {
                meshListView.currentIndex = index
                selectMeshes(index, index)
            }
            onMultipleSelection: selectMeshes(meshListView.currentIndex, index)
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
