#include <maya/MFnPlugin.h>
#include "mayaMVG/maya/cmd/MVGCmd.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "context/MVGContextCmd.h"
#include "context/MVGBuildFaceManipulator.h"

using namespace mayaMVG;

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// commands
	status = plugin.registerCommand("MVGCmd", MVGCmd::creator);

	// context
	status = plugin.registerContextCommand(MVGContextCmd::name, &MVGContextCmd::creator);

	// nodes
	status = plugin.registerNode("MVGBuildFaceManipulator", MVGBuildFaceManipulator::_id, &MVGBuildFaceManipulator::creator
								, &MVGBuildFaceManipulator::initialize, MPxNode::kManipulatorNode);
	if (!status)
		LOG_ERROR("initializePlugin", "unexpected error");
	return status;
}


MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	MVGMayaUtil::deleteMVGContext();
	MVGMayaUtil::deleteMVGWindow();

	// deregister commands
	status = plugin.deregisterCommand("MVGCmd");

	// context
	status = plugin.deregisterContextCommand(MVGContextCmd::name);

	// nodes
	status = plugin.deregisterNode(MVGBuildFaceManipulator::_id);

	if (!status)
		LOG_ERROR("uninitializePlugin", "unexpected error");
	return status;
}
