#include "mayaMVG/maya/context/MVGContextCmd.h"
#include "mayaMVG/maya/context/MVGContext.h"

using namespace mayaMVG;

MString MVGContextCmd::name("mayaMVGTool");

MVGContextCmd::MVGContextCmd() : m_pContext(NULL) {
}

MPxContext* MVGContextCmd::makeObj() {
    m_pContext = new MVGContext();
    return m_pContext;
}

void* MVGContextCmd::creator() {
	return new MVGContextCmd;
}

MStatus MVGContextCmd::doEditFlags() {
	return MS::kSuccess;
}

MStatus MVGContextCmd::doQueryFlags() {
	return MS::kSuccess;
}

MStatus MVGContextCmd::appendSyntax() {
	return MS::kSuccess;
}
