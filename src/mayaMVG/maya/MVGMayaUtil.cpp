#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include <maya/MFnDependencyNode.h>
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
#include <maya/MVectorArray.h>
#include <maya/MFnVectorArrayData.h>

namespace mayaMVG
{

namespace
{
MStatus getFloat3PlugValue(MPlug plug, MVector& value)
{
    // Retrieve the value as an MObject
    MObject object;
    plug.getValue(object);
    // Convert the MObject to a float3
    MFnNumericData numDataFn(object);
    numDataFn.getData(value[0], value[1], value[2]);
    return MS::kSuccess;
}

MStatus getFloat3asMObject(MVector value, MObject& object)
{
    // Convert the float value into an MObject
    MFnNumericData numDataFn;
    numDataFn.create(MFnNumericData::k3Float);
    numDataFn.setData(value[0], value[1], value[2]);
    object = numDataFn.object();
    return MS::kSuccess;
}
}

MStatus MVGMayaUtil::createMVGWindow()
{
    return MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                         "window.mvgCreateWindow()");
}

MStatus MVGMayaUtil::deleteMVGWindow()
{
    return MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                         "window.mvgDeleteWindow()");
}

QWidget* MVGMayaUtil::getMVGWindow()
{
    return MQtUtil::findWindow("mayaMVG");
}

QWidget* MVGMayaUtil::getMVGMenuLayout()
{
    return MQtUtil::findLayout("mvgMenuPanel");
}

QWidget* MVGMayaUtil::getMVGViewportLayout(const MString& viewName)
{
    M3dView view;
    if(M3dView::getM3dViewFromModelPanel(viewName, view))
        return view.widget();
    return NULL;
}

MStatus MVGMayaUtil::setFocusOnView(const MString& viewName)
{
    return MGlobal::executePythonCommand("import maya.cmds as cmds\n"
                                         "cmds.setFocus('" +
                                         viewName + "')\n");
}

bool MVGMayaUtil::isMVGView(const M3dView& view)
{
    QWidget* leftViewport = MVGMayaUtil::getMVGViewportLayout("mvgLPanel");
    QWidget* rightViewport = MVGMayaUtil::getMVGViewportLayout("mvgRPanel");
    if(!leftViewport || !rightViewport)
        return false;
    return ((view.widget() == leftViewport) || (view.widget() == rightViewport));
}

bool MVGMayaUtil::isActiveView(const M3dView& view)
{
    M3dView activeView = M3dView::active3dView();
    return (activeView.widget() == view.widget());
}

bool MVGMayaUtil::getComplementaryView(const M3dView& view, M3dView& complementaryView)
{
    M3dView tmpView;
    for(size_t i = 0; i < M3dView::numberOf3dViews(); ++i)
    {
        M3dView::get3dView(i, tmpView);
        if(isMVGView(tmpView) && tmpView.widget() != view.widget())
        {
            complementaryView = tmpView;
            return true;
        }
    }
    return false;
}

MStatus MVGMayaUtil::createMVGContext()
{
    return MGlobal::executePythonCommand("from mayaMVG import context;\n"
                                         "context.mvgCreateContext()");
}

MStatus MVGMayaUtil::deleteMVGContext()
{
    return MGlobal::executePythonCommand("from mayaMVG import context;\n"
                                         "context.mvgDeleteContext()");
}

MStatus MVGMayaUtil::activeContext()
{
    MString cmd;
    cmd.format("import maya.cmds as cmds\n"
               "cmds.setToolTo('^1s')",
               MVGContextCmd::instanceName);
    return MGlobal::executePythonCommand(cmd);
}

MStatus MVGMayaUtil::activeMayaContext()
{
    MGlobal::executePythonCommand("import maya.cmds as cmds\n");
    MString cmd;
    cmd.format("cmds.setToolTo('^1s')", MVGContext::_lastMayaManipulator);
    return MGlobal::executePythonCommand(cmd);
}

MStatus MVGMayaUtil::getCurrentContext(MString& context)
{
    MGlobal::executePythonCommand("import maya.cmds as cmds\n");
    return MGlobal::executePythonCommand("cmds.currentCtx()\n", context);
}

MStatus MVGMayaUtil::setCurrentContext(MString& context)
{
    MGlobal::executePythonCommand("import maya.cmds as cmds\n");
    MString command;
    command.format("cmds.setToolTo('^1s')", context);
    return MGlobal::executePythonCommand(command);
}

// static
MStatus MVGMayaUtil::setCreationMode()
{
    MString cmd;
    cmd.format("^1s -e -em 0 ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    return MGlobal::executeCommandOnIdle(cmd);
}
// static
MStatus MVGMayaUtil::setTriangulationMode()
{
    MString cmd;
    cmd.format("^1s -e -em 1 -mv 0 ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    return MGlobal::executeCommandOnIdle(cmd);
}
// static
MStatus MVGMayaUtil::setPointCloudMode()
{
    MString cmd;
    cmd.format("^1s -e -em 1 -mv 1 ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    return MGlobal::executeCommandOnIdle(cmd);
}
// static
MStatus MVGMayaUtil::setAdjacentPlaneMode()
{
    MString cmd;
    cmd.format("^1s -e -em 1 -mv 2 ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    return MGlobal::executeCommandOnIdle(cmd);
}
// static
MStatus MVGMayaUtil::setLocatorMode()
{
    MString cmd;
    cmd.format("^1s -e -em 2 ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    return MGlobal::executeCommandOnIdle(cmd);
}

MStatus MVGMayaUtil::setCameraInView(const MVGCamera& camera, const MString& viewName)
{
    MGlobal::executePythonCommand("import maya.cmds as cmds");
    std::stringstream ss; // one line cmd, to get result
    ss << "cmds.modelPanel('" << viewName << "', e=True, cam='" << camera.getName().c_str() << "')";
    return MGlobal::executePythonCommand(ss.str().c_str());
}

MStatus MVGMayaUtil::getCameraInView(MDagPath& path, const MString& viewName)
{
    MString camera;
    MGlobal::executePythonCommand("from mayaMVG import camera");
    MGlobal::executePythonCommand("camera.mvgGetCameraFromView('" + viewName + "')", camera);
    MSelectionList sList;
    MGlobal::getSelectionListByName(camera, sList);
    return sList.isEmpty() ? MS::kFailure : sList.getDagPath(0, path);
}

MStatus MVGMayaUtil::clearCameraInView(const MString& viewName)
{
    MGlobal::executePythonCommand("import maya.cmds as cmds");
    std::stringstream ss; // one line cmd, to get result
    ss << "cmds.modelPanel('" << viewName << "', e=True, cam='persp')";
    return MGlobal::executePythonCommand(ss.str().c_str());
}

MStatus MVGMayaUtil::addToMayaSelection(const MString& objectName)
{
    return MGlobal::executePythonCommand("import maya.cmds as cmds\n"
                                         "cmds.select('" +
                                         objectName + "', add=True)");
}

MStatus MVGMayaUtil::clearMayaSelection()
{
    return MGlobal::executePythonCommand("import maya.cmds as cmds\n"
                                         "cmds.select(cl=True)");
}

MStatus MVGMayaUtil::getIntArrayAttribute(const MObject& object, const MString& param,
                                          MIntArray& intArray, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    intArray.clear();
    if(plug.isArray())
    {
        for(size_t i = 0; i < plug.numElements(); ++i)
        {
            MPlug plugElmt = plug[i];
            intArray.append(plugElmt.asInt());
        }
    }
    else
    {
        MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
        CHECK_RETURN_STATUS(status);
        MFnIntArrayData arrayData(dataHandle.data(), &status);
        CHECK_RETURN_STATUS(status);
        status = arrayData.copyTo(intArray);
    }
    return status;
}

MStatus MVGMayaUtil::setIntArrayAttribute(const MObject& object, const MString& param,
                                          const MIntArray& intArray, bool networked)
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

MStatus MVGMayaUtil::getIntAttribute(const MObject& object, const MString& param, int& value,
                                     bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    value = plug.asInt();
    return status;
}

MStatus MVGMayaUtil::setIntAttribute(const MObject& object, const MString& param, const int& value,
                                     bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    plug.setInt(value);
    return status;
}

MStatus MVGMayaUtil::getDoubleAttribute(const MObject& object, const MString& param, double& value,
                                        bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    value = plug.asDouble();
    return status;
}

MStatus MVGMayaUtil::setDoubleAttribute(const MObject& object, const MString& param,
                                        const double& value, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    plug.setDouble(value);
    return status;
}

MStatus MVGMayaUtil::getDoubleArrayAttribute(const MObject& object, const MString& param,
                                             MDoubleArray& doubleArray, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    doubleArray.clear();
    if(plug.isArray())
    {
        for(size_t i = 0; i < plug.numElements(); ++i)
        {
            MPlug plugElmt = plug[i];
            doubleArray.append(plugElmt.asDouble());
        }
    }
    else
    {
        MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
        CHECK_RETURN_STATUS(status);
        if(dataHandle.data() == MObject::kNullObj)
            return MS::kFailure;
        MFnDoubleArrayData arrayData(dataHandle.data(), &status);
        CHECK_RETURN_STATUS(status);
        status = arrayData.copyTo(doubleArray);
    }
    return status;
}

MStatus MVGMayaUtil::setDoubleArrayAttribute(const MObject& object, const MString& param,
                                             const MDoubleArray& doubleArray, bool networked)
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

MStatus MVGMayaUtil::getVectorArrayAttribute(const MObject& object, const MString& param,
                                             MVectorArray& vectorArray, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    vectorArray.clear();
    if(plug.isArray())
    {
        for(size_t i = 0; i < plug.numElements(); ++i)
        {
            MPlug plugElmt = plug[i];
            MVector vector;
            getFloat3PlugValue(plugElmt, vector);
            vectorArray.append(vector);
        }
    }
    else
    {
        MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
        CHECK_RETURN_STATUS(status);
        if(dataHandle.data() == MObject::kNullObj)
            return MS::kFailure;
        MFnVectorArrayData arrayData(dataHandle.data(), &status);
        CHECK_RETURN_STATUS(status);
        status = arrayData.copyTo(vectorArray);
    }
    return status;
}

MStatus MVGMayaUtil::setVectorArrayAttribute(const MObject& object, const MString& param,
                                             const MVectorArray& vectorArray, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    MFnVectorArrayData fnData;
    MObject obj = fnData.create(vectorArray, &status);
    CHECK_RETURN_STATUS(status);
    status = plug.setValue(obj);
    return status;
}

MStatus MVGMayaUtil::getPointArrayAttribute(const MObject& object, const MString& param,
                                            MPointArray& pointArray, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    pointArray.clear();
    if(plug.isArray())
    {
        for(size_t i = 0; i < plug.numElements(); ++i)
        {
            MPlug plugElmt = plug[i];
            pointArray.append(MPoint(plugElmt.child(0).asFloat(), plugElmt.child(1).asFloat(),
                                     plugElmt.child(2).asFloat(), plugElmt.child(3).asFloat()));
        }
    }
    else
    {
        MDataHandle dataHandle = plug.asMDataHandle(MDGContext::fsNormal, &status);
        CHECK_RETURN_STATUS(status);
        if(dataHandle.data() == MObject::kNullObj)
            return MS::kFailure;
        MFnPointArrayData arrayData(dataHandle.data(), &status);
        CHECK_RETURN_STATUS(status);
        status = arrayData.copyTo(pointArray);
        CHECK_RETURN_STATUS(status);
    }
    return status;
}

MStatus MVGMayaUtil::setPointArrayAttribute(const MObject& object, const MString& param,
                                            const MPointArray& pointArray, bool networked)
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

MStatus MVGMayaUtil::getStringAttribute(const MObject& object, const MString& param,
                                        MString& string, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    string = plug.asString();
    return status;
}

MStatus MVGMayaUtil::setStringAttribute(const MObject& object, const MString& param,
                                        const MString& string, bool networked)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug(fn.findPlug(param, networked, &status));
    CHECK_RETURN_STATUS(status);
    plug.setString(string);
    return status;
}

MStatus MVGMayaUtil::findConnectedNodes(const MObject& object, const MString& param,
                                        std::vector<MObject>& nodes)
{
    MStatus status;
    MFnDependencyNode fn(object, &status);
    CHECK_RETURN_STATUS(status);
    MPlug plug = fn.findPlug(param, &status);
    CHECK_RETURN_STATUS(status);
    if(plug.isArray())
    {
        plug.evaluateNumElements(&status);
        CHECK_RETURN_STATUS(status);
        for(unsigned int i = 0; i < plug.numElements(); ++i)
        {
            MPlug plugElmt = plug[i];
            MPlugArray plugArray;
            plugElmt.connectedTo(plugArray, true, true, &status);
            if(plugArray.length() > 0)
            {
                for(unsigned int j = 0; j < plugArray.length(); ++j)
                    nodes.push_back(plugArray[j].node(&status));
            }
        }
    }
    else
    {
        MPlugArray plugArray;
        plug.connectedTo(plugArray, true, true, &status);
        if(plugArray.length() > 0)
        {
            for(unsigned int i = 0; i < plugArray.length(); ++i)
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
    if(status == MS::kSuccess)
        status = list.getDependNode(0, obj);
    return status;
}

MStatus MVGMayaUtil::getDagPathByName(const MString& name, MDagPath& path)
{
    MSelectionList list;
    MStatus status = list.add(name);
    if(status == MS::kSuccess)
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

MStatus MVGMayaUtil::openFileDialog(MString& directory)
{
    MStatus status = MGlobal::executePythonCommand("from mayaMVG import window");
    status = MGlobal::executePythonCommand( // one line cmd, to get result
        "window.mvgOpenProjectFileDialog()", directory);
    return status;
}

MStatus MVGMayaUtil::getUndoName(MString& undoName)
{
    MStatus status = MGlobal::executePythonCommand("import maya.cmds as cmds");
    status = MGlobal::executePythonCommand( // one line cmd, to get result
        "cmds.undoInfo( q=True, undoName=True )", undoName);
    return status;
}

MStatus MVGMayaUtil::getRedoName(MString& redoName)
{
    MStatus status = MGlobal::executePythonCommand("import maya.cmds as cmds");
    status = MGlobal::executePythonCommand( // one line cmd, to get result
        "cmds.undoInfo( q=True, redoName=True )", redoName);
    return status;
}

} // namespace
