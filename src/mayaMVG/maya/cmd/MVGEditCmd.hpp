#pragma once

#include "mayaMVG/maya/cmd/MVGPolyModifierCmd.hpp"
#include "mayaMVG/maya/mesh/MVGMeshEditFactory.hpp"
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>

class MDagPath;

namespace mayaMVG
{

class MVGEditCmd : public MVGPolyModifierCmd
{

public:
    MVGEditCmd();
    virtual ~MVGEditCmd();

public:
    static void* creator();
    virtual MStatus doIt(const MArgList& args);
    static MSyntax newSyntax();

public:
    MStatus redoIt();
    MStatus undoIt();
    bool isUndoable() const;
    MStatus finalize();

public:
    MStatus initModifierNode(MObject modifierNode);
    void create(const MDagPath& meshPath, const MPointArray& points);
    void move(const MDagPath& meshPath, const MIntArray& componentIDs,
              const MPointArray& worldSpacePositions, const MPointArray& cameraSpacePositions,
              int cameraID);

public:
    static MString _name;

private:
    MVGMeshEditFactory _editFactory;
    MVGMeshEditFactory::EditType _editType;
    MDagPath _meshPath;
    MObject _componentList;
    MIntArray _componentIDs;
    MPointArray _worldSpacePositions;
    MPointArray _cameraSpacePositions;
    int _cameraID;
};

} // namespace
