#pragma once

#include <maya/MPxContextCommand.h>

namespace mayaMVG {

// forward declaration
class MVGContext;

class MVGContextCmd: public MPxContextCommand {
public:
	static MString name;
public:
	MVGContextCmd();
public:
	virtual MPxContext * makeObj();
	static void* creator();
	virtual MStatus doEditFlags();
	virtual MStatus doQueryFlags();
	virtual MStatus appendSyntax();
private:
	MVGContext* _context;
};

} // namespace
