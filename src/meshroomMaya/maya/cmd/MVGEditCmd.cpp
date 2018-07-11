#include "meshroomMaya/maya/cmd/MVGEditCmd.hpp"
#include "meshroomMaya/maya/mesh/MVGMeshEditNode.hpp"
#include "meshroomMaya/core/MVGMesh.hpp"
#include "meshroomMaya/core/MVGProject.hpp"
#include "meshroomMaya/core/MVGLog.hpp"
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <cassert>

namespace meshroomMaya
{

MString MVGEditCmd::_name("MVGEditCmd");

MVGEditCmd::MVGEditCmd()
{
}

MVGEditCmd::~MVGEditCmd()
{
}

void* MVGEditCmd::creator()
{
    return new MVGEditCmd();
}

MSyntax MVGEditCmd::newSyntax()
{
    MSyntax s;
    s.enableEdit(false);
    s.enableQuery(false);
    return s;
}

MStatus MVGEditCmd::doIt(const MArgList& args)
{
    setMeshNode(_meshPath);
    setModifierNodeType(MVGMeshEditNode::_id);
    return doModifyPoly();
}

MStatus MVGEditCmd::redoIt()
{
    return redoModifyPoly();
}

MStatus MVGEditCmd::undoIt()
{
    return undoModifyPoly();
}

bool MVGEditCmd::isUndoable() const
{
    return true;
}

MStatus MVGEditCmd::finalize()
{
    MArgList command;
    command.addArg(MVGEditCmd::_name);
    return MPxToolCommand::doFinalize(command);
}

MStatus MVGEditCmd::initModifierNode(MObject node)
{
    MStatus status;
    MFnIntArrayData intArrayFn;
    MFnPointArrayData pointArrayFn;
    MObject attributeObject;
    // indices
    MPlug indicesPlug(node, MVGMeshEditNode::aInIndices);
    attributeObject = intArrayFn.create(_componentIDs, &status);
    indicesPlug.setValue(attributeObject);
    // world positions
    MPlug worldPositionsPlug(node, MVGMeshEditNode::aInWorldPositions);
    attributeObject = pointArrayFn.create(_worldSpacePositions, &status);
    worldPositionsPlug.setValue(attributeObject);
    // camera positions
    MPlug cameraPositionsPlug(node, MVGMeshEditNode::aInCameraPositions);
    attributeObject = pointArrayFn.create(_cameraSpacePositions, &status);
    cameraPositionsPlug.setValue(attributeObject);
    // camera id
    MPlug cameraIDPlug(node, MVGMeshEditNode::aInCameraID);
    cameraIDPlug.setValue(_cameraID);
    // clear blind data
    MPlug clearBDPlug(node, MVGMeshEditNode::aInClearBlindData);
    clearBDPlug.setValue(_clearBD);
    // edit type
    MPlug editTypePlug(node, MVGMeshEditNode::aInEditType);
    editTypePlug.setValue(_editType);
    return status;
}

void MVGEditCmd::addFace(const MDagPath& meshPath, const MPointArray& worldSpacePositions,
                         const MPointArray& cameraSpacePositions, int cameraID)
{
    _editType = MVGMeshEditFactory::kAddFace;
    _meshPath = meshPath;
    _worldSpacePositions = worldSpacePositions;
    _cameraSpacePositions = cameraSpacePositions;
    _cameraID = cameraID;

    // TODO remove from here, should always be valid
    if(!meshPath.isValid())
    {
        MVGMesh mesh = MVGMesh::create(MVGProject::_MESH);
        _meshPath = mesh.getDagPath();
    }
}

void MVGEditCmd::move(const MDagPath& meshPath, const MIntArray& componentIDs,
                      const MPointArray& worldSpacePositions,
                      const MPointArray& cameraSpacePositions, const int cameraID,
                      const bool clearBD)
{
    if(!meshPath.isValid())
    {
        LOG_ERROR("Mesh path is not valid : " << meshPath.fullPathName())
        return;
    }
    _editType = MVGMeshEditFactory::kMove;
    _meshPath = meshPath;
    _componentIDs = componentIDs;
    _worldSpacePositions = worldSpacePositions;
    _cameraSpacePositions = cameraSpacePositions;
    _cameraID = cameraID;
    _clearBD = clearBD;
}

void MVGEditCmd::clearBD(const MDagPath& meshPath, const MIntArray& componentIDs)
{
    if(!meshPath.isValid())
    {
        LOG_ERROR("Mesh path is not valid : " << meshPath.fullPathName())
        return;
    }
    _editType = MVGMeshEditFactory::kClearBD;
    _meshPath = meshPath;
    _componentIDs = componentIDs;
}

} // namespace
