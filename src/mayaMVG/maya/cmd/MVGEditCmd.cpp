#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>

namespace mayaMVG {

MString MVGEditCmd::name("MVGEditCmd");

namespace { // empty namespace

	static const char* createFlag = "-cr";
	static const char* createFlagLong = "-create";
	static const char* moveFlag = "-mv";
	static const char* moveFlagLong = "-move";

} // empty namespace

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
	s.enableEdit(false);
	s.enableQuery(false);
	return s;
}

MStatus MVGEditCmd::doIt(const MArgList& args)
{
	MStatus status = MS::kSuccess;
	MSyntax syntax = MVGEditCmd::newSyntax();
	MArgDatabase argData(syntax, args);
	// -create
	if (argData.isFlagSet(createFlag)) {
		_flags |= CMD_CREATE;
	}
	// -move
	if (argData.isFlagSet(moveFlag)) {
		_flags |= CMD_MOVE;
	}
	return redoIt();
}

MStatus MVGEditCmd::redoIt()
{
	MStatus status;
	// -create
	if(_flags & CMD_CREATE) {		
		// Retrieve mesh or create it
        MStatus status;
        status = MVGMayaUtil::getDagPathByName(_meshName, _meshPath);
		MVGMesh mesh(_meshPath);
		if(!mesh.isValid()) {
			mesh = MVGMesh::create(MVGProject::_MESH);
            _meshPath = mesh.dagPath();
            _meshName = mesh.dagPath().fullPathName();		
			if(!mesh.isValid())
				return MS::kFailure;
		}
		int index;
		if(!mesh.addPolygon(_points, index))
			return MS::kFailure;
		
		_indexes.clear();
		_indexes.append(index);		
	}
	// -move
	if(_flags & CMD_MOVE) {
        MVGMayaUtil::getDagPathByName(_meshName, _meshPath);
		MVGMesh mesh(_meshPath);
		if(!mesh.isValid())
			return MS::kFailure;
		
		MPoint oldPoint;
		_oldPoints.clear();
		for(int i = 0; i < _points.length(); ++i)
		{
			mesh.getPoint(_indexes[i], oldPoint);
			_oldPoints.append(oldPoint);
			mesh.setPoint(_indexes[i], _points[i]);
		}
	}
	
	MVGProjectWrapper::instance().rebuildMeshCacheFromMaya(_meshPath);
	MVGProjectWrapper::instance().rebuildCacheFromMaya();
	return status;
}

MStatus MVGEditCmd::undoIt()
{ 
	MStatus status;
	// -create
	if(_flags & CMD_CREATE) {
		MVGMesh mesh(_meshPath);
		if(!mesh.isValid())
			return MStatus::kFailure;
        if(mesh.getPolygonsCount() > 1)
        {
            mesh.deletePolygon(_indexes[0]);
        }
        // Can't delete last face with deletePolygon())
        // Delete directly the transform
        else
        {
            // MeshPath is invalid after destruction of the shape, we store it as a MString to retrieve it in redo
            _meshName = _meshPath.fullPathName();
            MObject toto = _meshPath.transform();
            MGlobal::deleteNode(toto);
        }
	}
	// -move
	if(_flags & CMD_MOVE) {
        MVGMesh mesh(_meshPath);
		if(!mesh.isValid())
			return MS::kFailure;
		
		for(int i = 0; i < _oldPoints.length(); ++i)
		{
			mesh.setPoint(_indexes[i], _oldPoints[i]);
		}
	}
	MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();
	MVGProjectWrapper::instance().rebuildCacheFromMaya();
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
	if(_flags & CMD_CREATE) {
		command.addArg(MString(createFlagLong));
	}
	// -move
	if(_flags & CMD_MOVE) {
		command.addArg(MString(moveFlagLong));
	}
	return MPxToolCommand::doFinalize(command);
}

void MVGEditCmd::doAddPolygon(const MDagPath& meshPath, const MPointArray& points)
{
	_flags |= CMD_CREATE;
	_meshPath = meshPath;
    _meshName = _meshPath.fullPathName();
	_points = points;
}

void MVGEditCmd::doMove(const MDagPath& meshPath, const MPointArray& points, const MIntArray& verticesIndexes)
{
	_flags |= CMD_MOVE;
	_meshPath = meshPath;
    _meshName = _meshPath.fullPathName();
	_points = points;
	_indexes = verticesIndexes;
}

void MVGEditCmd::doDelete()
{
	_flags |= CMD_DELETE;
}

} //mayaMVG