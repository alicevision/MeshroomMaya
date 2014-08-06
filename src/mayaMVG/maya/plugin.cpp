#include <maya/MFnPlugin.h>
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/cmd/MVGCmd.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/context/MVGContextCmd.h"
#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/maya/context/MVGMoveManipulator.h"


using namespace mayaMVG;

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// register commands
	status = plugin.registerCommand("MVGCmd", MVGCmd::creator);
	// register context
	status = plugin.registerContextCommand(MVGContextCmd::name, &MVGContextCmd::creator, 
								MVGEditCmd::name, MVGEditCmd::creator, MVGEditCmd::newSyntax);
	// register nodes
	status = plugin.registerNode("MVGCreateManipulator", MVGCreateManipulator::_id, 
								&MVGCreateManipulator::creator, &MVGCreateManipulator::initialize, 
								MPxNode::kManipulatorNode);
	status = plugin.registerNode("MVGMoveManipulator", MVGMoveManipulator::_id, 
								&MVGMoveManipulator::creator, &MVGMoveManipulator::initialize, 
								MPxNode::kManipulatorNode);
	// register ui
	status = plugin.registerUI("mayaMVGCreateUI", "mayaMVGDeleteUI");

	MVGMayaUtil::createMVGContext();

	if (!status)
		LOG_ERROR("unexpected error");
	return status;
}


MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	MVGMayaUtil::deleteMVGContext();
	MVGMayaUtil::deleteMVGWindow();

     // Remove callbacks
    status = MMessage::removeCallbacks(MVGCmd::_callbackIDs);
    
	// deregister commands
	status = plugin.deregisterCommand("MVGCmd");
	// deregister context
	status = plugin.deregisterContextCommand(MVGContextCmd::name, MVGEditCmd::name);
	// deregister nodes
	status = plugin.deregisterNode(MVGCreateManipulator::_id);
	status = plugin.deregisterNode(MVGMoveManipulator::_id);
    
	MVGMayaUtil::deleteMVGContext();

	if (!status)
		LOG_ERROR("unexpected error");
	return status;
}
