
import pymel.core as pm

def getParent(node):
    import maya.cmds as cmds
    parent = cmds.listRelatives(node, parent=True, fullPath=True)
    return parent[0]

def lockNode(node, lockValue):
    import maya.cmds as cmds
    cmds.setAttr(node+".translateX", lock=lockValue)
    cmds.setAttr(node+".translateY", lock=lockValue)
    cmds.setAttr(node+".translateZ", lock=lockValue)
    cmds.setAttr(node+".rotateX", lock=lockValue)
    cmds.setAttr(node+".rotateY", lock=lockValue)
    cmds.setAttr(node+".rotateZ", lock=lockValue)

def listMVGMeshesTransform():
    import maya.cmds as cmds
    mvgMeshes = []
    meshList = pm.ls(type="mesh")
    for mesh in meshList:
        if not cmds.attributeQuery("mvg", node=mesh.name(), exists=True):
            continue
        mvgAttr = mesh.name() + ".mvg"
        if not cmds.getAttr(mvgAttr):
          continue
        relatives = cmds.listRelatives(mesh.name(), ad=True, ap=True, type="transform")
        for r in relatives:
            mvgMeshes.append(r)
    return mvgMeshes


