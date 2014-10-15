#include "mayaMVG/maya/cmd/MVGEditCmd.hpp"
#include "mayaMVG/qt/MVGUserLog.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <cassert>

namespace
{ // empty namespace

static const char* createFlag = "-cr";
static const char* createFlagLong = "-create";
static const char* moveFlag = "-mv";
static const char* moveFlagLong = "-move";
static const char* locateFlag = "-loc";
static const char* locateFlagLong = "-locate";

} // empty namespace

namespace mayaMVG
{

MString MVGEditCmd::name("MVGEditCmd");

MVGEditCmd::MVGEditCmd()
    : _flags(0)
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
    s.addFlag(createFlag, createFlagLong);
    s.addFlag(moveFlag, moveFlagLong);
    s.addFlag(locateFlag, locateFlagLong);
    s.enableEdit(false);
    s.enableQuery(false);
    return s;
}

MStatus MVGEditCmd::doIt(const MArgList& args)
{
    MSyntax syntax = MVGEditCmd::newSyntax();
    MArgDatabase argData(syntax, args);
    // -create
    if(argData.isFlagSet(createFlag))
    {
        _flags |= CMD_CREATE;
    }
    // -move
    if(argData.isFlagSet(moveFlag))
    {
        _flags |= CMD_MOVE;
    }
    // -locate
    if(argData.isFlagSet(locateFlag))
    {
        _flags |= CMD_LOCATE;
    }
    return redoIt();
}

MStatus MVGEditCmd::redoIt()
{
    MStatus status;
    MVGMesh mesh(_meshName);
    // -create
    if(_flags & CMD_CREATE)
    {
        if(!mesh.isValid())
        { // Retrieve mesh or create it
            mesh = MVGMesh::create(MVGProject::_MESH);
            // USER_WARNING("Action is no stacked in undo/redo")
            // status = MS::kFailure;
            // if(!mesh.isValid())
            //     return MS::kFailure;
            _meshName = mesh.getDagPath().fullPathName();
        }
        int index;
        if(!mesh.addPolygon(_points, index))
            return MS::kFailure;
        _indexes.clear();
        _indexes.append(index);
    }
    // -move
    if(_flags & CMD_MOVE)
    {
        if(!mesh.isValid())
            return MS::kFailure;
        MPoint oldPoint;
        for(int i = 0; i < _points.length(); ++i)
        {
            mesh.getPoint(_indexes[i], oldPoint);
            mesh.setPoint(_indexes[i], _points[i]);
            _points[i] = oldPoint;
        }
    }
    // -locate
    if(_flags & CMD_LOCATE)
    {
        for(int i = 0; i < _points.length(); ++i)
            mesh.setBlindDataPerCamera(_indexes[i], _cameraID, _points[i]);
    }

    return status;
}

MStatus MVGEditCmd::undoIt()
{
    MStatus status;
    MVGMesh mesh(_meshName);
    // -create
    if(_flags & CMD_CREATE)
    {
        if(!mesh.isValid())
        {
            return MStatus::kFailure;
        }
        if(mesh.getPolygonsCount() > 1)
            mesh.deletePolygon(_indexes[0]);
        else
        {
            USER_ERROR("Can't delete last face")
            return MS::kFailure;
            // MObject transform = mesh.dagPath().transform();
            // MGlobal::deleteNode(transform);
            // MGlobal::executeCommand("delete mvgMesh", false, false);
        }
    }
    // -move
    if(_flags & CMD_MOVE)
    {
        if(!mesh.isValid())
            return MS::kFailure;
        MPoint oldPoint;
        for(int i = 0; i < _points.length(); ++i)
        {
            mesh.getPoint(_indexes[i], oldPoint);
            mesh.setPoint(_indexes[i], _points[i]);
            _points[i] = oldPoint;
        }
    }
    // -locate
    if(_flags & CMD_LOCATE)
    {
        for(int i = 0; i < _points.length(); ++i)
            mesh.unsetBlindDataPerCamera(_indexes[i], _cameraID);
    }

    return status;
}

bool MVGEditCmd::isUndoable() const
{
    return true;
}

MStatus MVGEditCmd::finalize()
{
    MArgList command;
    command.addArg(MVGEditCmd::name);
    // -create
    if(_flags & CMD_CREATE)
    {
        command.addArg(MString(createFlagLong));
    }
    // -move
    if(_flags & CMD_MOVE)
    {
        command.addArg(MString(moveFlagLong));
    }
    // -locate
    if(_flags & CMD_LOCATE)
    {
        command.addArg(MString(locateFlagLong));
    }
    return MPxToolCommand::doFinalize(command);
}

void MVGEditCmd::doAddPolygon(const MDagPath& meshPath, const MPointArray& points)
{
    _flags |= CMD_CREATE;
    _meshName = meshPath.fullPathName();
    _points = points;
}

void MVGEditCmd::doMove(const MDagPath& meshPath, const MPointArray& points,
                        const MIntArray& verticesIndexes)
{
    assert(points.length() == verticesIndexes.length());
    _flags |= CMD_MOVE;
    _meshName = meshPath.fullPathName();
    _points = points;
    _indexes = verticesIndexes;
}

void MVGEditCmd::doLocate(const MDagPath& meshPath, const MPointArray& cameraSpacePoints,
                          const MIntArray& verticesIndexes, int cameraID)
{
    assert(cameraSpacePoints.length() == verticesIndexes.length());
    _flags |= CMD_LOCATE;
    _meshName = meshPath.fullPathName();
    _points = cameraSpacePoints;
    _indexes = verticesIndexes;
    _cameraID = cameraID;
}

} // namespace
