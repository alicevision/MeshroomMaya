#include "mayaMVG/maya/MVGMayaUtil.h"
#include <QWidget>
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MSelectionList.h>
#include <maya/M3dView.h>
#include <maya/MPlug.h>
#include <maya/MDataHandle.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MPlugArray.h>
#include <maya/MCommonSystemUtils.h>
#include <maya/MPointArray.h>

using namespace mayaMVG;

MStatus MVGMayaUtil::openFileDialog(MString& directory)
{
	MStatus status;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def openFileDialog():\n"
		"	directory = cmds.fileDialog2(caption=\"Choose projectdirectory\", fileMode=3, okCaption=\"Load\")\n"
		"	return directory[0].encode('ascii')\n");
	
	status = MGlobal::executePythonCommand("openFileDialog()", directory);
	
	return status;
}

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
		"        cmds.modelEditor('mvgLPanel', e=True, displayAppearance='smoothShaded')\n"
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
		"        cmds.modelEditor('mvgRPanel', e=True, displayAppearance='smoothShaded')\n"
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
		"if cmds.contextInfo('mayaMVGTool1', exists=True):\n"
		"    cmds.deleteUI('mayaMVGTool1', toolContext=True)\n"
		"cmds.mayaMVGTool('mayaMVGTool1')\n");
}

MStatus MVGMayaUtil::deleteMVGContext() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setToolTo('selectSuperContext')\n"
		"if cmds.contextInfo('mayaMVGTool1', exists=True):\n"
		"    cmds.deleteUI('mayaMVGTool1', toolContext=True)\n");
}

MStatus MVGMayaUtil::activeContext() 
{
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setToolTo('mayaMVGTool1')");
}

MStatus MVGMayaUtil::activeSelectionContext()
{
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.setToolTo('selectSuperContext')\n");
}

MStatus MVGMayaUtil::setCameraInView(const MVGCamera& camera, const MString& viewName) {
	return MGlobal::executePythonCommand(
		MString("import maya.cmds as cmds\n"
		"cmds.modelPanel('")+viewName+MString("', e=True, cam='")+camera.name().c_str()+"')");
}

MStatus MVGMayaUtil::getCameraInView(MDagPath& path, const MString& viewName) {
	MString camera;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"def getMVGPanel():\n"
		"    return cmds.modelPanel('"+viewName+"', q=True, cam=True)\n");
	MGlobal::executePythonCommand("getMVGPanel()", camera);
	MSelectionList sList;
	MGlobal::getSelectionListByName(camera, sList);
	return sList.isEmpty() ? MS::kFailure : sList.getDagPath(0, path);
}

MStatus MVGMayaUtil::addToMayaSelection(MString objectName) {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.select('"+objectName+"', add=True)");
}

MStatus MVGMayaUtil::clearMayaSelection() {
	return MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.select(cl=True)");
}

MStatus MVGMayaUtil::getIntArrayAttribute(const MObject& object, const MString& param, MIntArray& intArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	intArray.clear();
	if (plug.isArray()) {
		for(size_t i = 0; i < plug.numElements(); ++i) {
			MPlug plugElmt = plug[i];
			intArray.append(plugElmt.asInt());
		}
	} else {
		MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
		CHECK_RETURN_STATUS(status);
		MFnIntArrayData arrayData(dataHandle.data(), &status);
		CHECK_RETURN_STATUS(status);
		status = arrayData.copyTo(intArray);
	}
	return status;
}

MStatus MVGMayaUtil::setIntArrayAttribute(const MObject &object, const MString &param, const MIntArray &intArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	MFnIntArrayData fnData;
	MObject obj = fnData.create(intArray, &status);
	CHECK_RETURN_STATUS(status);
	status = plug.setValue(obj);
	return status;
}

MStatus MVGMayaUtil::getIntAttribute(const MObject& object, const MString& param, int& value, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	value = plug.asInt();
	return status;
}

MStatus MVGMayaUtil::setIntAttribute(const MObject& object, const MString& param, const int& value, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	plug.setInt(value);
	return status;
}

MStatus MVGMayaUtil::getDoubleArrayAttribute(const MObject& object, const MString& param, MDoubleArray& doubleArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	doubleArray.clear();
	if (plug.isArray()) {
		for(size_t i = 0; i < plug.numElements(); ++i) {
			MPlug plugElmt = plug[i];
			doubleArray.append(plugElmt.asDouble());
		}
	} else {
		MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
		CHECK_RETURN_STATUS(status);
		MFnDoubleArrayData arrayData(dataHandle.data(), &status);
		CHECK_RETURN_STATUS(status);
		status = arrayData.copyTo(doubleArray);
	}
	return status;
}

MStatus MVGMayaUtil::setDoubleArrayAttribute(const MObject& object, const MString& param, const MDoubleArray& doubleArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	MFnDoubleArrayData fnData;
	MObject obj = fnData.create(doubleArray, &status);
	CHECK_RETURN_STATUS(status);
	status = plug.setValue(obj);
	return status;
}

MStatus MVGMayaUtil::getPointArrayAttribute(const MObject& object, const MString& param, MPointArray& pointArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	pointArray.clear();
		
	if (plug.isArray()) {
		for(size_t i = 0; i < plug.numElements(); ++i) {
			MPlug plugElmt = plug[i];
			pointArray.append(MPoint(plugElmt.child(0).asDouble(), plugElmt.child(1).asDouble()));
		}
	} else {
		MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
		CHECK_RETURN_STATUS(status);
		MFnPointArrayData arrayData(dataHandle.data(), &status);
		CHECK_RETURN_STATUS(status);
		status = arrayData.copyTo(pointArray);
	}
	return status;
}

MStatus MVGMayaUtil::getPointArrayAttributeSize(const MObject& object, const MString& param, int& size, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	
	if (plug.isArray()) {
		size = plug.numElements();
	}
	else
	{		
		MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
		CHECK_RETURN_STATUS(status);
		MFnPointArrayData arrayData(dataHandle.data(), &status);
		CHECK_RETURN_STATUS(status);
		
		size = arrayData.length();
	}

	
	return status;
}

MStatus MVGMayaUtil::getPointInArrayAttribute(const MObject& object, const MString& param, MPoint& point, int index, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);

	if (plug.isArray()) {
		
		point = MPoint(plug[index].child(0).asDouble(), plug[index].child(1).asDouble());

	} else {
		MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
		CHECK_RETURN_STATUS(status);
		MFnPointArrayData arrayData(dataHandle.data(), &status);
		CHECK_RETURN_STATUS(status);
		point = arrayData.array()[index];
	}
	return status;
}

MStatus MVGMayaUtil::setPointArrayAttribute(const MObject& object, const MString& param, const MPointArray& pointArray, bool networked)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug(fn.findPlug(param, networked, &status));
	CHECK_RETURN_STATUS(status);
	MFnPointArrayData fnData;
	MObject obj = fnData.create(pointArray, &status);
	CHECK_RETURN_STATUS(status);
	status = plug.setValue(obj);
	return status;
}

MStatus MVGMayaUtil::findConnectedNodes(const MObject& object, const MString& param, std::vector<MObject>& nodes)
{
	MStatus status;
	MFnDependencyNode fn(object, &status);
	CHECK_RETURN_STATUS(status);
	MPlug plug = fn.findPlug(param, &status);
	CHECK_RETURN_STATUS(status);
	if (plug.isArray()) {
		plug.evaluateNumElements(&status);
		CHECK_RETURN_STATUS(status);
		for (unsigned int i = 0; i < plug.numElements(); ++i) {
			MPlug plugElmt = plug[i];
			MPlugArray plugArray;
			plugElmt.connectedTo(plugArray, true, true, &status);
			if(plugArray.length() > 0) {
				for (unsigned int j = 0; j < plugArray.length(); ++j)
					nodes.push_back(plugArray[j].node(&status));
			}
		}
	} else {
		MPlugArray plugArray;
		plug.connectedTo(plugArray, true, true, &status);
		if(plugArray.length() > 0) {
			for (unsigned int i=0; i<plugArray.length(); ++i)
				nodes.push_back(plugArray[i].node(&status));
		}
	}
	CHECK_RETURN_STATUS(status);
	return status;
}

MStatus MVGMayaUtil::getObjectByName(const MString& name, MObject& obj)
{
	obj = MObject::kNullObj;
	MSelectionList list;
	MStatus status = list.add(name);
	if (status == MS::kSuccess)
		status = list.getDependNode(0, obj);
	return status;
}

MStatus MVGMayaUtil::getDagPathByName(const MString& name, MDagPath& path)
{
	MSelectionList list;
	MStatus status = list.add(name);
	if (status == MS::kSuccess)
		status = list.getDagPath(0, path);
	return status;
}

MString MVGMayaUtil::getEnv(const MString& var)
{
	MString result;
	MCommonSystemUtils::getEnv(var, result);
	return result;
}

MString MVGMayaUtil::getModulePath()
{
	MString result;
	MGlobal::executeCommand("getModulePath -moduleName \"mayaMVG\";", result);
	return result;
}

MStatus MVGMayaUtil::deletePreviewShape()
{
	MStatus status;
	MGlobal::executePythonCommand(
		"import maya.cmds as cmds\n"
		"cmds.delete('previewMesh')\n");
	
	
	return status;
}

