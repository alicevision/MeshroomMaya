#include <maya/MFnPlugin.h>
#include "cmd/MVGCmd.h"
#include "util/MVGLog.h"
#include "context/MVGContextCmd.h"
#include "context/MVGManipContainer.h"

using namespace mayaMVG;

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// commands
	status = plugin.registerCommand("MVGCmd", MVGCmd::creator);

	// context
	status = plugin.registerContextCommand(MVGContextCmd::name, &MVGContextCmd::creator);

	// nodes
	status = plugin.registerNode("MVGManip", MVGManipContainer::id, &MVGManipContainer::creator
								, &MVGManipContainer::initialize, MPxNode::kManipContainer
								/*, &PaintManipContainer::drawDbClassification*/);
	if (!status)
		LOG_ERROR("initializePlugin", "unexpected error");
	return status;
}


MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	// deregister commands
	status = plugin.deregisterCommand("MVGCmd");

	// context
	status = plugin.deregisterContextCommand(MVGContextCmd::name);

	// nodes
	status = plugin.deregisterNode(MVGManipContainer::id);

	if (!status)
		LOG_ERROR("uninitializePlugin", "unexpected error");
	return status;
}
