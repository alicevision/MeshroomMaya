#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>

using namespace mayaMVG;

MString MVGEditCmd::name("MVGEditCmd");

namespace { // empty namespace

	static const char* createFlag = "-cr";
	static const char* createFlagLong = "-create";
	static const char* moveFlag = "-mv";
	static const char* moveFlagLong = "-move";

} // empty namespace

MVGEditCmd::MVGEditCmd() 
	: _flags(CMD_CREATE)
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
		MVGMesh mesh(_meshPath);
		if(!mesh.isValid()) {
			mesh = MVGMesh::create(MVGProject::_MESH);
			_meshPath = mesh.dagPath();
			if(!mesh.isValid())
				return MS::kFailure;
		}
		if(!mesh.addPolygon(_points, _index))
			return MS::kFailure;
	}
	// -move
	if(_flags & CMD_MOVE) {
	}
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
		if(!mesh.deletePolygon(_index))
			return MS::kFailure;	
	}
	// -move
	if(_flags & CMD_MOVE) {
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
	_points = points;
}

void MVGEditCmd::doMove()
{
	_flags |= CMD_MOVE;
}

void MVGEditCmd::doDelete()
{
	_flags |= CMD_DELETE;
}
