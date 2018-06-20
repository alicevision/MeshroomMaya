#pragma once

// see polyModifierCmd from maya devkit

#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MPlug.h>
#include <maya/MPxToolCommand.h>

namespace meshroomMaya
{

class MVGPolyModifierCmd : public MPxToolCommand
{
public:
    MVGPolyModifierCmd();
    virtual ~MVGPolyModifierCmd();

protected:
    void setMeshNode(MDagPath mesh);
    MDagPath getMeshNode() const;
    void setModifierNodeType(MTypeId type);
    void setModifierNodeName(MString name);
    MTypeId getModifierNodeType() const;
    MString getModifierNodeName() const;
    virtual MStatus initModifierNode(MObject modifierNode);
    virtual MStatus directModifier(MObject mesh);
    MStatus doModifyPoly();
    MStatus redoModifyPoly();
    MStatus undoModifyPoly();

private:
    struct modifyPolyData
    {
        MObject meshNodeTransform;
        MObject meshNodeShape;
        MPlug meshNodeDestPlug;
        MObject meshNodeDestAttr;
        MObject upstreamNodeTransform;
        MObject upstreamNodeShape;
        MPlug upstreamNodeSrcPlug;
        MObject upstreamNodeSrcAttr;
        MObject modifierNodeSrcAttr;
        MObject modifierNodeDestAttr;
        MObject tweakNode;
        MObject tweakNodeSrcAttr;
        MObject tweakNodeDestAttr;
    };

private:
    bool isCommandDataValid();
    void collectNodeState();
    MStatus createModifierNode(MObject& modifierNode);
    MStatus processMeshNode(modifyPolyData& data);
    MStatus processUpstreamNode(modifyPolyData& data);
    MStatus processModifierNode(MObject modifierNode, modifyPolyData& data);
    MStatus processTweaks(modifyPolyData& data);
    MStatus connectNodes(MObject modifierNode);
    MStatus cacheMeshData();
    MStatus cacheMeshTweaks();
    MStatus undoCachedMesh();
    MStatus undoTweakProcessing();
    MStatus undoDirectModifier();
    MStatus getFloat3PlugValue(MPlug plug, MFloatVector& value);
    MStatus getFloat3asMObject(MFloatVector value, MObject& object);

private:
    bool fDagPathInitialized;
    MDagPath fDagPath;
    MDagPath fDuplicateDagPath;
    bool fModifierNodeTypeInitialized;
    bool fModifierNodeNameInitialized;
    MTypeId fModifierNodeType;
    MString fModifierNodeName;
    bool fHasHistory;
    bool fHasTweaks;
    bool fHasRecordHistory;
    MIntArray fTweakIndexArray;
    MFloatVectorArray fTweakVectorArray;
    MObject fMeshData;
    MDGModifier fDGModifier;
    MDagModifier fDagModifier;
};

inline void MVGPolyModifierCmd::setMeshNode(MDagPath mesh)
{
    fDagPath = mesh;
    fDagPathInitialized = true;
}

inline MDagPath MVGPolyModifierCmd::getMeshNode() const
{
    return fDagPath;
}

inline void MVGPolyModifierCmd::setModifierNodeType(MTypeId type)
{
    fModifierNodeType = type;
    fModifierNodeTypeInitialized = true;
}

inline void MVGPolyModifierCmd::setModifierNodeName(MString name)
{
    fModifierNodeName = name;
    fModifierNodeNameInitialized = true;
}

inline MTypeId MVGPolyModifierCmd::getModifierNodeType() const
{
    return fModifierNodeType;
}

inline MString MVGPolyModifierCmd::getModifierNodeName() const
{
    return fModifierNodeName;
}

} // namespace
