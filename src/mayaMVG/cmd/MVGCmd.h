#pragma once

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

namespace mayaMVG {

class MVGCmd: public MPxCommand {

public:
	MVGCmd();
	virtual ~MVGCmd();

public:
	virtual MStatus doIt(const MArgList& args);
	static void* creator();
	static MSyntax newSyntax();
};

} // mayaMVG
