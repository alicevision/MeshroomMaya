
import pymel.core as pm


def mvgGetCameraFromView(viewName):
    import maya.cmds as cmds
    return cmds.modelPanel(viewName, q=True, cam=True)
