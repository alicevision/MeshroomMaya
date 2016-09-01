
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

def initHotkeySet():
    """ For Maya >=2016, ensure the current hotkey set is writable. """
    if cmds.about(version=True) < "2016":
        return
    currentSet = cmds.hotkeySet(current=True, q=True)
    if currentSet == "Maya_Default":  # Read-only set
        writableSetName = "Maya_Default_MVG"
        # Select or create writable hotkey set
        if cmds.hotkeySet(writableSetName, exists=True):
            cmds.hotkeySet(writableSetName, current=True, edit=True)
        else:
            cmds.hotkeySet(writableSetName, current=True)

# source_type = "mel" or "python"
def initMVGCommand(name, script, source_type, key_sequence, alt=False, ctl=False, command=False, release=False):
    # Runtime command
    exists = mel.eval("runTimeCommand -exists {name}".format(name=name))
    nameCommand = name + "NameCommand"
    if exists:
        cmd = "runTimeCommand -e -c \"{script}\" -cl {sourceType} {cmdName}".format(script=script, sourceType=source_type, cmdName=name)
        mel.eval(cmd)
    else:
        cmd = "runTimeCommand -c \"{script}\" -cl {sourceType} -cat \"MayaMVG\" {cmdName}".format(script=script, sourceType=source_type, cmdName=name)
        mel.eval(cmd)

    #Create nameCommand
    annotation = name
    cmds.nameCommand(
        nameCommand,
        command=name,
        annotation=name,
        stp=source_type)

    #Check for hotkey in optionVar
    varString = "{0}_hotkey".format(name)
    cmd = "optionVar -exists {varString}".format(varString=varString);
    if mel.eval(cmd):
        cmd = "optionVar -q {varString}".format(varString=varString)
        keyArray = mel.eval(cmd)
        key_sequence = keyArray[0]
        alt = True if keyArray[1] == "1" else False
        ctl = True if keyArray[2] == "1" else False
        release = True if keyArray[3] == "1" else False
        command = True if keyArray[4] == "1" else False
        
    # Check hotkey validity
    commandName = cmds.hotkey(key_sequence, q=True, name=True, ctl=ctl, alt=alt, cmd=command)
    if commandName != None and commandName != "":
        #TODO : proper warning
        print "WARNING : hotkey {hotkey} already assigned to \"{name}\". Please set your own sequence".format(hotkey=key_sequence, name=commandName);
        return
    
    # Bind nameCommands to hotkeys
    # TODO : use constructHotkeyCommand ? 
    cmds.hotkey(
      keyShortcut=key_sequence,
      name=nameCommand,
      alt=alt,
      ctl=ctl,
      cmd=command)

# Remove command (and hotkey)
def removeMVGCommand(name):
    cmd = "source \"mayaMVG/hotkey.mel\""
    mel.eval(cmd)
    cmd = "deleteRunTimeCommand \"{0}\"".format(name)
    mel.eval(cmd)