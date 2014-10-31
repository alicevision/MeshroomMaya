#include "mayaMVG/maya/mesh/MVGMeshEditNode.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MPointArray.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MIOStream.h>

namespace mayaMVG
{

MTypeId MVGMeshEditNode::_id(0x00085000); // FIXME

MObject MVGMeshEditNode::aComponentList;
MObject MVGMeshEditNode::aInMesh;
MObject MVGMeshEditNode::aInIndices;
MObject MVGMeshEditNode::aInWorldPositions;
MObject MVGMeshEditNode::aInCameraPositions;
MObject MVGMeshEditNode::aInCameraID;
MObject MVGMeshEditNode::aInClearBlindData;
MObject MVGMeshEditNode::aInEditType;
MObject MVGMeshEditNode::aOutMesh;

MVGMeshEditNode::MVGMeshEditNode()
{
}

MVGMeshEditNode::~MVGMeshEditNode()
{
}

void* MVGMeshEditNode::creator()
{
    return new MVGMeshEditNode();
}

MStatus MVGMeshEditNode::initialize()
{
    MStatus status;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnEnumAttribute eAttr;

    aComponentList =
        tAttr.create("inputComponents", "icl", MFnComponentListData::kComponentList, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aComponentList))

    aInMesh = tAttr.create("inMesh", "im", MFnMeshData::kMesh, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInMesh))

    aInIndices = tAttr.create("inIndices", "iin", MFnData::kIntArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInIndices))

    aInWorldPositions = tAttr.create("inWorldPositions", "iwp", MFnData::kPointArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInWorldPositions))

    aInCameraPositions = tAttr.create("inCameraPositions", "icp", MFnData::kPointArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInCameraPositions))

    aInCameraID = nAttr.create("inCameraID", "ici", MFnNumericData::kInt, -1, &status);
    CHECK_RETURN_STATUS(status)
    nAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInCameraID))

    aInClearBlindData =
        nAttr.create("inClearBlindData", "icb", MFnNumericData::kBoolean, false, &status);
    CHECK_RETURN_STATUS(status)
    nAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aInClearBlindData))

    aInEditType = eAttr.create("inEditType", "iet", 0, &status);
    CHECK_RETURN_STATUS(status)
    eAttr.setStorable(true);
    eAttr.addField("create", 0);
    eAttr.addField("move", 1);
    CHECK_RETURN_STATUS(addAttribute(aInEditType))

    aOutMesh = tAttr.create("outMesh", "om", MFnMeshData::kMesh, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    CHECK_RETURN_STATUS(addAttribute(aOutMesh))

    CHECK_RETURN_STATUS(attributeAffects(aComponentList, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInMesh, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInIndices, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInWorldPositions, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInCameraPositions, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInCameraID, aOutMesh))
    CHECK_RETURN_STATUS(attributeAffects(aInEditType, aOutMesh))

    return MS::kSuccess;
}

MStatus MVGMeshEditNode::compute(const MPlug& plug, MDataBlock& data)
{
    if(plug != aOutMesh)
        return MS::kUnknownParameter;

    MStatus status;
    MDataHandle inMeshHandle = data.inputValue(aInMesh, &status);
    CHECK_RETURN_STATUS(status)
    MDataHandle outMeshHandle = data.outputValue(aOutMesh, &status);
    CHECK_RETURN_STATUS(status)

    // copy the inMesh to the outMesh, so you can perform operations directly on outMesh
    outMeshHandle.set(inMeshHandle.asMesh());

    // state attribute
    MDataHandle stateHandle = data.outputValue(state, &status);
    CHECK_RETURN_STATUS(status)
    if(stateHandle.asShort() == 1) // HasNoEffect/PassThrough
        return status;

    // retrieve indices
    MDataHandle indexHandle = data.outputValue(aInIndices, &status);
    MObject indexArrayObj = indexHandle.data();
    MFnIntArrayData indexArrayFn(indexArrayObj);
    MIntArray indexArray;
    indexArrayFn.copyTo(indexArray);

    // retrieve world positions
    MDataHandle worldPositionHandle = data.outputValue(aInWorldPositions, &status);
    MObject worldPositionArrayObj = worldPositionHandle.data();
    MFnPointArrayData worldPositionArrayFn(worldPositionArrayObj);
    MPointArray worldPositionArray;
    worldPositionArrayFn.copyTo(worldPositionArray);

    // retrieve camera positions
    MDataHandle cameraPositionHandle = data.outputValue(aInCameraPositions, &status);
    MObject cameraPositionArrayObj = cameraPositionHandle.data();
    MFnPointArrayData cameraPositionArrayFn(cameraPositionArrayObj);
    MPointArray cameraPositionArray;
    cameraPositionArrayFn.copyTo(cameraPositionArray);

    // retrieve camera id
    MDataHandle cameraIDHandle = data.outputValue(aInCameraID, &status);

    // retrieve clearBD
    MDataHandle clearBlindDataHandle = data.outputValue(aInClearBlindData, &status);

    // retrieve edit type
    MDataHandle editTypeHandle = data.outputValue(aInEditType, &status);

    // configure factory
    MObject meshObj = outMeshHandle.asMesh();
    _editFactory.setMesh(meshObj);
    // _editFactory.setComponentList(compList);
    _editFactory.setComponentIDs(indexArray);
    _editFactory.setWorldPositions(worldPositionArray);
    _editFactory.setCameraPositions(cameraPositionArray);
    _editFactory.setCameraID(cameraIDHandle.asInt());
    _editFactory.setClearBlindData(clearBlindDataHandle.asBool());
    _editFactory.setEditType(static_cast<MVGMeshEditFactory::EditType>(editTypeHandle.asShort()));
    // perform mesh operation
    CHECK_RETURN_STATUS(_editFactory.doIt())

    outMeshHandle.setClean();
    return status;
}

} // namespace
