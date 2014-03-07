#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/qt/MVGMenu.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>
#include <maya/M3dView.h>

using namespace mayaMVG;

MStatus MVGMayaUtil::createMVGWindow() {
	MStatus status;
	MString windowName;
	status = MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def createMVGWindow():\n"
		"    win = cmds.window('mayaMVG')\n"
		"    cmds.paneLayout('mainPane', configuration='vertical3')\n"
		"    # first modelPanel\n"
		"    cmds.paneLayout('leftPane')\n"
		"    if cmds.modelPanel('mvgLPanel', ex=True):\n"
		"        cmds.modelPanel('mvgLPanel', e=True, p='leftPane')\n"
		"    else:\n"
		"        cmds.modelPanel('mvgLPanel', mbv=False)\n"
		"        cmds.modelEditor('mvgLPanel', e=True, grid=False)\n"
		"        cmds.modelEditor('mvgLPanel', e=True, cameras=False)\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    # second modelPanel\n"
		"    cmds.paneLayout('rightPane')\n"
		"    if cmds.modelPanel('mvgRPanel', ex=True):\n"
		"        cmds.modelPanel('mvgRPanel', e=True, p='rightPane')\n"
		"    else:\n"
		"        cmds.modelPanel('mvgRPanel', mbv=False)\n"
		"        cmds.modelEditor('mvgRPanel', e=True, grid=False)\n"
		"        cmds.modelEditor('mvgRPanel', e=True, cameras=False)\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    # custom Qt content\n"
		"    cmds.paneLayout('mvgMenuPanel')\n"
		"    cmds.setParent('..')\n"
		"    cmds.setParent('..')\n"
		"    cmds.showWindow(win)\n"
		"    cmds.window(win, e=True, widthHeight=[920,700])\n"
		"    return win\n");
	status = MGlobal::executePythonCommand("createMVGWindow()", windowName);
	return status;
}

MStatus MVGMayaUtil::deleteMVGWindow() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"if cmds.window('mayaMVG', exists=True):\n"
		"    cmds.deleteUI('mayaMVG', window=True)\n");
}

QWidget* MVGMayaUtil::getMVGWindow() {
	return MQtUtil::findWindow("mayaMVG");
}

QWidget* MVGMayaUtil::getMVGMenuLayout() {
	return MQtUtil::findLayout("mvgMenuPanel");
}

QWidget* MVGMayaUtil::getMVGLeftViewportLayout() {
	M3dView leftView;
	M3dView::getM3dViewFromModelPanel("mvgLPanel", leftView);
	return leftView.widget();
}

QWidget* MVGMayaUtil::getMVGRightViewportLayout() {
	M3dView rightView;
	M3dView::getM3dViewFromModelPanel("mvgRPanel", rightView);
	return rightView.widget();
}

MStatus MVGMayaUtil::setFocusOnLeftView() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setFocus('mvgLPanel')\n");
}

MStatus MVGMayaUtil::setFocusOnRightView() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setFocus('mvgRPanel')\n");
}

bool MVGMayaUtil::isMVGView(const M3dView & view) {
	QWidget* leftViewport = MVGMayaUtil::getMVGLeftViewportLayout();
	QWidget* rightViewport = MVGMayaUtil::getMVGRightViewportLayout();
	return ((view.widget() == leftViewport) || (view.widget() == rightViewport));
}

bool MVGMayaUtil::isActiveView(const M3dView & view) {
	M3dView activeView = M3dView::active3dView();
	return (activeView.widget() == view.widget());
}

bool MVGMayaUtil::mouseUnderView(const M3dView & view) {
	QWidget * viewWidget = view.widget();
	if (viewWidget->rect().contains(viewWidget->mapFromGlobal(QCursor::pos()))) {
		return true;
	}
	return false;
}

MStatus MVGMayaUtil::createMVGContext() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"if cmds.contextInfo('MVGTool1', exists=True):\n"
		"    cmds.deleteUI('MVGTool1', toolContext=True)\n"
		"cmds.MVGTool('MVGTool1')\n");
}

MStatus MVGMayaUtil::deleteMVGContext() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setToolTo('selectSuperContext')\n"
		"if cmds.contextInfo('MVGTool1', exists=True):\n"
		"    cmds.deleteUI('MVGTool1', toolContext=True)\n");
}

MStatus MVGMayaUtil::getMVGLeftCamera(MDagPath& path) {
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

MStatus MVGMayaUtil::setMVGLeftCamera(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.modelPanel('mvgLPanel', e=True, cam='"+camera+"')");
}

MStatus MVGMayaUtil::getMVGRightCamera(MDagPath& path) {
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

MStatus MVGMayaUtil::setMVGRightCamera(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.modelPanel('mvgRPanel', e=True, cam='"+camera+"')");
}

MStatus MVGMayaUtil::addToMayaSelection(MString camera) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.select('"+camera+"', add=True)");
}

MStatus MVGMayaUtil::clearMayaSelection() {
  return MGlobal::executePythonCommand(
    "import maya.cmds as cmds\n"
    "cmds.select(cl=True)");
}

MStatus MVGMayaUtil::activeContext() 
{
  return MGlobal::executePythonCommand(
    "import maya.cmds as cmds\n"
    "cmds.setToolTo('MVGTool1')\n");
}
