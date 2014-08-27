#include "mayaMVG/maya/context/MVGContextCmd.h"
#include "mayaMVG/maya/context/MVGContext.h"

namespace mayaMVG {

MString MVGContextCmd::name("mayaMVGTool");

namespace {

static const char * rebuildFlag = "-r";
static const char * rebuildFlagLong = "-rebuild";

}

MVGContextCmd::MVGContextCmd() : _context(NULL) {
}

MPxContext* MVGContextCmd::makeObj() {
    _context = new MVGContext();
    return _context;
}

void* MVGContextCmd::creator() {
	return new MVGContextCmd;
}

MStatus MVGContextCmd::doEditFlags() {
	MArgParser argData = parser();
	// -rebuild: rebuild cache
	if (argData.isFlagSet(rebuildFlag)) {
		MVGManipulatorUtil& cache =_context->getCache();
		cache.rebuildAllMeshesCacheFromMaya();
		cache.rebuild();
	}
	return MS::kSuccess;
}

MStatus MVGContextCmd::doQueryFlags() {
	return MS::kSuccess;
}

MStatus MVGContextCmd::appendSyntax() {
	MSyntax mySyntax = syntax();
	if (MS::kSuccess
			!= mySyntax.addFlag(rebuildFlag, rebuildFlagLong)) {
		return MS::kFailure;
	}
	return MS::kSuccess;
}

} // namespace
