
import pymel.core as pm


def mvgDeleteContext():
    import maya.cmds as cmds
    cmds.setToolTo('selectSuperContext')
    if cmds.contextInfo('mayaMVGTool1', exists=True):
        cmds.deleteUI('mayaMVGTool1', toolContext=True)

def mvgCreateContext():
    import maya.cmds as cmds
    if cmds.contextInfo('mayaMVGTool1', exists=True):
        cmds.deleteUI('mayaMVGTool1', toolContext=True)
    cmds.mayaMVGTool('mayaMVGTool1')

