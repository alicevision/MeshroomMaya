
import pymel.core as pm


def mvgOpenProjectFileDialog():
    import maya.cmds as cmds
    path = cmds.fileDialog2(caption='Select project file', fileMode=1, fileFilter="*.abc", okCaption='Load')
    if path: return path[0]
    else: return ''

def mvgDeleteWindow():
    import maya.cmds as cmds
    if cmds.window('mayaMVG', exists=True):
        cmds.deleteUI('mayaMVG', window=True)
    if cmds.modelPanel('mvgLPanel', q=True, ex=True):
        cmds.deleteUI('mvgLPanel', pnl=True)
    if cmds.modelPanel('mvgRPanel', q=True, ex=True):
        cmds.deleteUI('mvgRPanel', pnl=True)

def mvgReloadPanels():
    import maya.cmds as cmds
    # first modelPanel
    i = 0
    if not cmds.paneLayout('leftPane', q=True, ex=True):
        cmds.paneLayout('leftPane')
        i += 1
    if not cmds.modelPanel('mvgLPanel', q=True, ex=True):
        cmds.modelPanel('mvgLPanel', mbv=False, l='MVG leftView', p='leftPane')
        cmds.modelEditor('mvgLPanel', e=True, allObjects=False, grid=False, hud=False, polymeshes=True, imagePlane=True, locators=True)
        cmds.modelEditor('mvgLPanel', e=True, xray=1, displayAppearance='smoothShaded', wireframeOnShaded=True)
        cmds.modelEditor('mvgLPanel', e=True, rendererName="base_OpenGL_Renderer")
        i += 1
    for j in range(i):
        cmds.setParent('..')
    # second modelPanel
    i = 0
    if not cmds.paneLayout('rightPane', q=True, ex=True):
        cmds.paneLayout('rightPane')
        i += 1
    if not cmds.modelPanel('mvgRPanel', q=True, ex=True):
        cmds.modelPanel('mvgRPanel', mbv=False, l='MVG rightView', p='rightPane')
        cmds.modelEditor('mvgRPanel', e=True, allObjects=False, grid=False, hud=False, polymeshes=True, imagePlane=True, locators=True)
        cmds.modelEditor('mvgRPanel', e=True, xray=1, displayAppearance='smoothShaded', wireframeOnShaded=True)
        cmds.modelEditor('mvgRPanel', e=True, rendererName="base_OpenGL_Renderer")
        i += 1
    for j in range(i):
        cmds.setParent('..')


def mvgCreateWindow():
    
    import maya.cmds as cmds
    mvgDeleteWindow()
 
    win = cmds.window('mayaMVG')
    cmds.paneLayout('mainPane', configuration='vertical3')
    mvgReloadPanels()
    # custom Qt content
    cmds.paneLayout('mvgMenuPanel')
    cmds.setParent('..')
    cmds.setParent('..')
    cmds.showWindow(win)
    cmds.window(win, e=True, widthHeight=[1000,800])
    return win
