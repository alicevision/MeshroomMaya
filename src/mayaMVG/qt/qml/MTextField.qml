import QtQuick 1.1
import QtDesktop 0.1


/**
 * QtDesktop TextField exposing "accepted" signal when Return/Enter key is pressed
 */
TextField {
    signal accepted()

    Keys.onReturnPressed: accepted()
    Keys.onEnterPressed: accepted()
}
