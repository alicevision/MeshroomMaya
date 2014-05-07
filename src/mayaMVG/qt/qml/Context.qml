import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: context

    width: 500
    height: 120


    RowLayout
    {
        id: row
        width: parent.width
        height: parent.height

        ContextBar
        {
            id: contextBar

            implicitWidth: 30
            height: parent.height
        }

        Params
        {
            id: params

            Layout.horizontalSizePolicy: Layout.Expanding
            height: parent.height
            visible: contextBar.settingsVisibility
        }
    }
}


