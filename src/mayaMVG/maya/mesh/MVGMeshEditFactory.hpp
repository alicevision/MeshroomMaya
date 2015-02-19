#pragma once

#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>
#include <maya/MPointArray.h>

class MFnMesh;

namespace mayaMVG
{

class MVGMeshEditFactory
{

public:
    enum EditType
    {
        kAddFace = 0,
        kMove = 1,
    };

public:
    MVGMeshEditFactory();
    virtual ~MVGMeshEditFactory() {}

public:
    void setMesh(const MObject& mesh);
    void setComponentList(const MObject& componentList);
    void setComponentIDs(const MIntArray& componentIDs);
    void setWorldPositions(const MPointArray& worldPositions);
    void setCameraPositions(const MPointArray& cameraPositions);
    void setCameraID(const int cameraID);
    void setClearBlindData(const bool clear);
    void setEditType(const EditType type);

public:
    MStatus doIt();

private:
    MObject _meshObj;
    MIntArray _componentIDs;
    MObject _componentList;
    MPointArray _worldPositions;
    MPointArray _cameraPositions;
    int _cameraID;
    bool _clearBD;
    EditType _editType;
};

} // namespace
