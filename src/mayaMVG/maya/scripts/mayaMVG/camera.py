
import pymel.core as pm


def mvgGetCameraFromView(viewName):
    import maya.cmds as cmds
    return cmds.modelPanel(viewName, q=True, cam=True)

def mvgScaleLocator(scale):
    cameraList = pm.ls(type='camera')
    for c in cameraList:
        pm.setAttr( c.attr('locatorScale'), scale)
