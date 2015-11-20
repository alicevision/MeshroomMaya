
import pymel.core as pm
import maya.cmds as cmds

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

def setUndistortImage(abcFilePath, imageAttribute):
  import os
  
  projectPath = os.path.dirname(abcFilePath)
  imageDir = os.path.join(projectPath, 'undistort/jpg/')
  cameraList = cmds.ls(ca=True)
  for c in cameraList:
    if cmds.attributeQuery(imageAttribute, node=c, exists=True):    
      imagePath = cmds.getAttr(c+'.'+imageAttribute)
      basename = os.path.basename(imagePath)
      fileName, fileExtension = os.path.splitext(basename)
      fileName += "-UO-full.jpg"

      cmds.setAttr(c+'.'+imageAttribute, os.path.join(imageDir, fileName), type="string")
