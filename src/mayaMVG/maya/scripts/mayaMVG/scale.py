
import pymel.core as pm


def lockNode(node, lockValue):
    import maya.cmds as cmds
    cmds.setAttr(node+".translateX", lock=lockValue)
    cmds.setAttr(node+".translateY", lock=lockValue)
    cmds.setAttr(node+".translateZ", lock=lockValue)
    cmds.setAttr(node+".rotateX", lock=lockValue)
    cmds.setAttr(node+".rotateY", lock=lockValue)
    cmds.setAttr(node+".rotateZ", lock=lockValue)

def scaleScene(transformMatrix, projectNodeName, meshName):
    import maya.cmds as cmds
    relatives = cmds.listRelatives(projectNodeName, ad=True, pa=True, type="transform")
    for r in relatives:
        lockNode(r, False)
    lockNode(meshName, False)
    matrix = [];
    tm = transformMatrix.split(" ")
    for i in tm:
        matrix.append(float(i))
    cmds.xform(projectNodeName, r=True, m=matrix)
    cmds.makeIdentity(projectNodeName, a=True)
    cmds.xform(meshName, r=True, m=matrix)
    cmds.makeIdentity(meshName, a=True)
    for r in relatives:
        lockNode(r, True)
    lockNode(meshName, True)