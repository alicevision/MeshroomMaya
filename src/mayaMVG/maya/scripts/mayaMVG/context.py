
import pymel.core as pm
import maya.cmds as cmds
import maya.mel as mel


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

# source_type = "mel" or "python"
def initMVGCommand(name, script, source_type, key_sequence, alt=False, ctl=False):
    # Runtime command
    exists = mel.eval("runTimeCommand -exists {name}".format(name=name))
    if exists:
        cmd = "runTimeCommand -e -c \"{script}\" -cl {sourceType} {cmdName}".format(script=script, sourceType=source_type, cmdName=name)
        mel.eval(cmd)
        return

    # Check hotkey validity
    commandName = cmds.hotkey(key_sequence, q=True, name=True, ctl=True)
    if commandName != None:
        print "WARNING : hotkey already assigned to \"{name}\". Please set your own sequence".format(name=commandName);
        return
    cmd = "runTimeCommand -c \"{script}\" -cl {sourceType} -cat User {cmdName}".format(script=script, sourceType=source_type, cmdName=name)
    mel.eval(cmd)
 
    #Create nameCommands
    annotation = name
    cmds.nameCommand(
        name,
        command=name,
        annotation=name,
        stp=source_type)
 
    # Bind nameCommands to hotkeys
    cmds.hotkey(
        keyShortcut=key_sequence,
        name=name,
        alt=alt,
        ctl=ctl)

# Remove command (and hotkey)
def removeMVGCommand(name):
    #TODO : remove hotkey ... and commandName !!! cf assignCommand ? 
    #cmd =  "runTimeCommand -e -delete {cmdName}".format(cmdName=name)
    cmd =  "runTimeCommand -e -c \"\" {cmdName}".format(cmdName=name)
    mel.eval(cmd)