
import pymel.core as pm

def getParent(node):
    import maya.cmds as cmds
    parent = cmds.listRelatives(node, parent=True, fullPath=True)
    return parent[0]

def lockNode(node, lockValue):
    import maya.cmds as cmds
    print "OLAAAAAAAAAAAAAAAAAA"
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

def scaleScene(transformMatrix, projectNodeName):
    import maya.cmds as cmds
    relatives = cmds.listRelatives(projectNodeName, ad=True, pa=True, type="transform")
    for r in relatives:
        lockNode(r, False)
    mvgMeshList = listMVGMeshesTransform()
    for mesh in mvgMeshList:
        lockNode(mesh, False)
    matrix = [];
    tm = transformMatrix.split(" ")
    for i in tm:
        matrix.append(float(i))
    cmds.xform(projectNodeName, r=True, m=matrix)
    cmds.makeIdentity(projectNodeName, a=True)
    for mesh in mvgMeshList:
        cmds.xform(mesh, r=True, m=matrix)
        cmds.makeIdentity(mesh, a=True)
        lockNode(mesh, True)
    for r in relatives:
        lockNode(r, True)  
