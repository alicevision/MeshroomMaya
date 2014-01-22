#include "MVGUtil.h"
#include "MVGLog.h"
#include "qt/MVGMenu.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>


using namespace mayaMVG;

QWidget* MVGUtil::createMVGWindow() {
	MString windowName;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def createMVGWindow():\n"
		"    win = cmds.window('openMVG')\n"
		"    cmds.paneLayout('mainPane', configuration='vertical3')\n"
		"    # first modelPanel\n"
		"    cmds.paneLayout('leftPane')\n"
		"    if cmds.modelPanel('mvgLPanel', ex=True):\n"
		"        cmds.modelPanel('mvgLPanel', e=True, p='leftPane')\n"
		"    else: cmds.modelPanel('mvgLPanel', mbv=False)\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    # second modelPanel\n"
		"    cmds.paneLayout('rightPane')\n"
		"    if cmds.modelPanel('mvgRPanel', ex=True):\n"
		"        cmds.modelPanel('mvgRPanel', e=True, p='rightPane')\n"
		"    else: cmds.modelPanel('mvgRPanel', mbv=False)\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    # custom Qt content\n"
		"    cmds.paneLayout('mvgMenuPanel')\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    cmds.showWindow(win)\n"
		"    cmds.window(win, e=True, widthHeight=[920,700])\n"
		"    return win\n");
	MGlobal::executePythonCommand("createMVGWindow()", windowName);
	// check for window creation
	QWidget* mayaWindow = MQtUtil::findWindow(windowName);
	return mayaWindow;
}

MStatus MVGUtil::deleteMVGWindow() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"if cmds.window('openMVG', exists=True):\n"
		"    cmds.deleteUI('openMVG', window=True)\n");
}

void MVGUtil::populateMVGMenu(MVGMenu* menu) {
	menu->clear();
	for(MItDependencyNodes it(MFn::kDependencyNode); !it.isDone(); it.next()) {
		MDagPath p;
		MDagPath::getAPathTo(it.item(), p);
		MFnDependencyNode fn(p.node());
		if(fn.typeName() == "camera") {
			MFnDependencyNode fn(p.transform());
			menu->addCamera(fn.name().asChar());
		}
	}
}

MStatus MVGUtil::createMVGContext() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"if cmds.contextInfo('MVGTool1', exists=True):\n"
		"    cmds.deleteUI('MVGTool1', toolContext=True)\n"
		"cmds.MVGTool('MVGTool1')\n");
}

MStatus MVGUtil::deleteMVGContext() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setToolTo('selectSuperContext')\n"
		"if cmds.contextInfo('MVGTool1', exists=True):\n"
		"    cmds.deleteUI('MVGTool1', toolContext=True)\n");
}

MStatus MVGUtil::getMVGLeftCamera(MDagPath& path) {
	MString camera;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def getMVGPanel():\n"
		"    return cmds.modelPanel('mvgLPanel', q=True, cam=True)\n");
	MGlobal::executePythonCommand("getMVGPanel()", camera);
	MSelectionList sList;
	MGlobal::getSelectionListByName(camera, sList);
	return sList.isEmpty() ? MS::kFailure : sList.getDagPath(0, path);
}

MStatus MVGUtil::setMVGLeftCamera(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.modelPanel('mvgLPanel', e=True, cam='"+camera+"')");
}

MStatus MVGUtil::getMVGRightCamera(MDagPath& path) {
	MString camera;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def getMVGPanel():\n"
		"    return cmds.modelPanel('mvgRPanel', q=True, cam=True)\n");
	MGlobal::executePythonCommand("getMVGPanel()", camera);
	MSelectionList sList;
	MGlobal::getSelectionListByName(camera, sList);
	return sList.isEmpty() ? MS::kFailure : sList.getDagPath(0, path);
}

MStatus MVGUtil::setMVGRightCamera(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.modelPanel('mvgRPanel', e=True, cam='"+camera+"')");
}

MStatus MVGUtil::addToMayaSelection(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.select('"+camera+"', add=True)");
}

MStatus MVGUtil::clearMayaSelection() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.select(cl=True)");
}
