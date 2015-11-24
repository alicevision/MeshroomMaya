
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

def setImagesPaths(abcFilePath, imageAttribute, thumbnailAttribute):
  import os
  
  projectPath = os.path.dirname(abcFilePath)
  imageDir = os.path.join(projectPath, 'undistort/jpg/')
  thumbnailDir = os.path.join(projectPath, 'undistort/thumbnail/')
  cameraList = cmds.ls(ca=True)
  for c in cameraList:    
    if not cmds.attributeQuery(imageAttribute, node=c, exists=True):
      continue
    originalImagePath = cmds.getAttr(c+'.'+imageAttribute)
    basename = os.path.basename(originalImagePath)
    fileName, fileExtension = os.path.splitext(basename)
    imageName = fileName + "-UO-full.jpg"
    cmds.setAttr(c+'.'+imageAttribute, os.path.join(imageDir, imageName), type="string")
    
    if not cmds.attributeQuery(thumbnailAttribute, node=c, exists=True):
      continue
    imageName = fileName + "-UO-thumbnail.jpg"
    cmds.setAttr(c+'.'+thumbnailAttribute, os.path.join(thumbnailDir, imageName), type="string")

def mapImagesPaths(imageAttribute, thumbnailAttribute, abcFilePath):
  import os

  projectPath = os.path.dirname(abcFilePath)
  imageDir = os.path.join(projectPath, 'undistort/jpg/')
  thumbnailDir = os.path.join(projectPath, 'undistort/thumbnail/')

  cameraList = cmds.ls(ca=True)
  for c in cameraList:      
    if not cmds.attributeQuery(imageAttribute, node=c, exists=True) or not imageDir:
      continue
    originalImagePath = cmds.getAttr(c+'.'+imageAttribute)
    basename = os.path.basename(originalImagePath)
    cmds.setAttr(c+'.'+imageAttribute, os.path.join(imageDir, basename), type="string")

    if not cmds.attributeQuery(thumbnailAttribute, node=c, exists=True) or not thumbnailDir:
      continue
    originalImagePath = cmds.getAttr(c+'.'+thumbnailAttribute)
    basename = os.path.basename(originalImagePath)
    cmds.setAttr(c+'.'+thumbnailAttribute, os.path.join(thumbnailDir, basename), type="string")