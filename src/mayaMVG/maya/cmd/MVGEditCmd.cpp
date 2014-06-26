#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/core/MVGLog.h"
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
		MGlobal::executeCommand("");
	}
	// -move
	if(_flags & CMD_MOVE) {
		MGlobal::executeCommand("");
	}
	return status;
}

MStatus MVGEditCmd::undoIt()
{
	MStatus status;
	// -create
	if(_flags & CMD_CREATE) {
		MGlobal::executeCommand("");
	}
	// -move
	if(_flags & CMD_MOVE) {
		MGlobal::executeCommand("");
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
		command.addArg(createFlagLong);
	}
	// -move
	if(_flags & CMD_MOVE) {
		command.addArg(moveFlagLong);
	}
	return MPxToolCommand::doFinalize(command);
}

void MVGEditCmd::doCreate()
{
	_flags |= CMD_CREATE;
}

void MVGEditCmd::doMove()
{
	_flags |= CMD_MOVE;
}

void MVGEditCmd::doDelete()
{
	_flags |= CMD_DELETE;
}
