
import pymel.core as pm


def mvgGetCameraFromView(viewName):
    import maya.cmds as cmds
    return cmds.modelPanel(viewName, q=True, cam=True)

def mvgScaleLocator(scale):
    cameraList = pm.ls(type='camera')
    for c in cameraList:
        pm.setAttr( c.attr('locatorScale'), scale)

def mvgSetImagePlane(cameraShape, imageFile):
    import maya.cmds as cmds
    imagePlaneName = cmds.imagePlane(camera=cameraShape)
    cmds.setAttr( "%s.imageName" % imagePlaneName[0], imageFile, type="string")
    cmds.connectAttr( "%s.message" % imagePlaneName[0], "%s.imagePlane" %cameraShape, na=1)
