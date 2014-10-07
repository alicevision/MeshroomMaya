#pragma once

#include "mayaMVG/maya/mesh/MVGMeshEditFactory.hpp"
#include <maya/MPxNode.h>
#include <maya/MTypeId.h>

namespace mayaMVG
{

class MVGMeshEditNode : public MPxNode
{
public:
    MVGMeshEditNode();
    virtual ~MVGMeshEditNode();

public:
    virtual MStatus compute(const MPlug& plug, MDataBlock& data);
    static void* creator();
    static MStatus initialize();

public:
    static MTypeId _id;
    static MObject aComponentList;
    static MObject aInMesh;
    static MObject aInIndices;
    static MObject aInWorldPositions;
    static MObject aInCameraPositions;
    static MObject aInCameraID;
    static MObject aInEditType;
    static MObject aOutMesh;

private:
    MVGMeshEditFactory _editFactory;
};

} // namespace
