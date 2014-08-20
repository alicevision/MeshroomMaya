
import pymel.core as pm


def mvgOpenProjectFileDialog():
    import maya.cmds as cmds
    path = cmds.fileDialog2(caption='Select project path', fileMode=3, okCaption='Load')
    if path: return path[0]
    else: return ''

def mvgDeleteWindow():
    import maya.cmds as cmds
    if cmds.window('mayaMVG', exists=True):
        cmds.deleteUI('mayaMVG', window=True)

def mvgCreateWindow():
    import maya.cmds as cmds
    win = cmds.window('mayaMVG')
    cmds.paneLayout('mainPane', configuration='vertical3')
    # first modelPanel
    cmds.paneLayout('leftPane')
    if cmds.modelPanel('mvgLPanel', ex=True):
        cmds.modelPanel('mvgLPanel', e=True, p='leftPane')
    else:
        cmds.modelPanel('mvgLPanel', mbv=False, l='MVG leftView')
        cmds.modelEditor('mvgLPanel', e=True, grid=False, cameras=False, dynamics=False)
        # cmds.modelEditor('mvgLPanel', e=True, displayAppearance='smoothShaded')
    cmds.setParent('..')
    cmds.setParent('..')
    # second modelPanel
    cmds.paneLayout('rightPane')
    if cmds.modelPanel('mvgRPanel', ex=True):
        cmds.modelPanel('mvgRPanel', e=True, p='rightPane')
    else:
        cmds.modelPanel('mvgRPanel', mbv=False, l='MVG rightView')
        cmds.modelEditor('mvgRPanel', e=True, grid=False, cameras=False, dynamics=False)
        # cmds.modelEditor('mvgRPanel', e=True, displayAppearance='smoothShaded')
    cmds.setParent('..')
    cmds.setParent('..')
    # custom Qt content
    cmds.paneLayout('mvgMenuPanel')
    cmds.setParent('..')
    cmds.setParent('..')
    cmds.showWindow(win)
    cmds.window(win, e=True, widthHeight=[920,700])
    return win

def mvgReloadPanels():
    import maya.cmds as cmds
    if(cmds.paneLayout('leftPane', ex=True)):
        if not cmds.modelPanel('mvgLPanel', ex=True):
            cmds.modelPanel('mvgLPanel', mbv=False, l='MVG leftView', p='leftPane')
            cmds.modelEditor('mvgLPanel', e=True, grid=False, cameras=False, dynamics=False)
    if(cmds.paneLayout('rightPane', ex=True)):
        if not cmds.modelPanel('mvgRPanel', ex=True):
            cmds.modelPanel('mvgRPanel', mbv=False, l='MVG rightView', p='rightPane')
            cmds.modelEditor('mvgRPanel', e=True, grid=False, cameras=False, dynamics=False)
