
import pymel.core as pm


def openMVGWindow_CB():
    pm.MVGCmd()

def mvgCreateMenu():
    gMainWindow = pm.mel.eval('$tmpVar=$gMainWindow')
    menuName = "mvgMenu"
    menuLabel = "MayaMVG"
    if(pm.menu(menuName, exists=True)):
        pm.deleteUI(menuName)
    if(gMainWindow != ""):
        pm.setParent(gMainWindow)
        menu = pm.menu(menuName, label=menuLabel, tearOff=True)
        pm.menuItem(parent=menu, label='Open...', command=pm.Callback(openMVGWindow_CB))
        pm.menuItem(parent=menu, divider=True)


def mvgDeleteMenu():
    menuName = "mvgMenu"
    if(pm.menu(menuName, exists=True)):
        pm.deleteUI(menuName)
