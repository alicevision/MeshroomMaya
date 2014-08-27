#include "mayaMVG/maya/context/MVGContextCmd.h"
#include "mayaMVG/maya/context/MVGContext.h"

namespace mayaMVG {

MString MVGContextCmd::name("mayaMVGTool");

namespace {

static const char * rebuildFlag = "-r";
static const char * rebuildFlagLong = "-rebuild";

}

MVGContextCmd::MVGContextCmd() : _pContext(NULL) {
}

MPxContext* MVGContextCmd::makeObj() {
    _pContext = new MVGContext();
    return _pContext;
}

void* MVGContextCmd::creator() {
	return new MVGContextCmd;
}

MStatus MVGContextCmd::doEditFlags() {
	MStatus status = MS::kSuccess;
	MArgParser argData = parser();
	// -rebuild: rebuild cache
	if (argData.isFlagSet(rebuildFlag)) {
		MVGManipulatorUtil& cache =_pContext->getCache();
		cache.rebuildAllMeshesCacheFromMaya();
		cache.rebuild();
	}
	return status;
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
