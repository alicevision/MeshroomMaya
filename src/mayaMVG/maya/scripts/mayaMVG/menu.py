
import pymel.core as pm
import os

'''
Add a new 'imagePath' attribute to each Maya camera transform node
  in the scene (only when the camera is connected to a valid ImagePlane
  node)
'''
def addImagePathAttribute():
    cameras = pm.ls(type='camera')
    for c in cameras:
        # retrieve connected imgplane
        imgplanes = c.imagePlane.listConnections(shapes=True, type='imagePlane')
        if not imgplanes:
            continue
        # retrieve the imgplane file path
        path = imgplanes[0].imageName.get()
        # if no imageName, check for 'imageNameDeferred' attribute
        if not path:
            if not pm.attributeQuery('imageNameDeferred', node=imgplanes[0], exists=True):
               continue
            path = imgplanes[0].imageNameDeferred.get()
            if not path:
                continue
        # add a new 'imagePath' attribute on camera transform
        transform = c.getParent()
        if not transform.hasAttr('imagePath'):
            transform.addAttr('imagePath', dt='string')
        # fill this new attribute w/ the imgplane file path
        transform.imagePath.set(path)

'''
Export the current selection to an ABC archive, w/ the following options:
  - the worldSpace option
  - the uvWrite option
  - an entry in the export attribute list : 'imagePath'
'''
def exportSelectionAsABC():
    # check for AbcExport command
    if not 'AbcExport' in dir(pm):
        pm.error('AbcExport: command not found.')
        return
    # retrieve all selected node paths
    selection = pm.ls(sl=True, recursive=True, dagObjects=True)
    if not selection:
        return
    nodePathtoExport = []
    for n in selection:
        if n.type() != 'transform':
            nodePathtoExport.append(n.getParent().fullPath())
    # get out file path from user
    outfile = pm.fileDialog2(fileMode=0)
    if not outfile:
        return
    # ensure we use a '*.abc' file extension
    outfile = os.path.splitext(outfile[0])[0]+'.abc'
    # build the AbcExport command
    exportCmd = '-worldSpace -attr imagePath -file %s'%outfile
    for p in nodePathtoExport:
        exportCmd += ' -root %s'%p
    exportCmd = '''
import pymel.core as pm
pm.AbcExport(j="%s")'''%exportCmd
    pm.evalDeferred(exportCmd)

def exportSelectionAsABC_CB():
    addImagePathAttribute()
    exportSelectionAsABC()
      
def openMVGWindow_CB():
    pm.MVGCmd()

def mvgCreateMenu():
    gMainWindow = pm.mel.eval('$tmpVar=$gMainWindow')
    menuName = "mvgMenu"
    menuLabel = "MayaMVG"
    if(pm.menu(menuName, exists=True)):
        pm.deleteUI(menuName)
    if(gMainWindow != ""):
        pm.setParent(gMainWindow)
        menu = pm.menu(menuName, label=menuLabel, tearOff=True)
        pm.menuItem(parent=menu, label='Open...', command=pm.Callback(openMVGWindow_CB))
        pm.menuItem(parent=menu, label='Export selection as ABC', command=pm.Callback(exportSelectionAsABC_CB))
        pm.menuItem(parent=menu, divider=True)

def mvgDeleteMenu():
    menuName = "mvgMenu"
    if(pm.menu(menuName, exists=True)):
        pm.deleteUI(menuName)
