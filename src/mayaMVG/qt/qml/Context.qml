import QtQuick 1.1
import QtDesktop 0.1

Item {
    id: context

    width: 500
    height: 120


    ColumnLayout
    {
        id: col
        width: parent.width
        height: parent.height

        Params
        {
            id: params

            Layout.verticalSizePolicy: Layout.Expanding
            width: parent.width
            visible: contextBar.settingsVisibility
        }
    }
}


