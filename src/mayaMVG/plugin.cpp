#include <maya/MFnPlugin.h>
#include "cmd/MVGCmd.h"

using namespace mayaMVG;

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// register commands
	status = plugin.registerCommand("MVGCmd", MVGCmd::creator);

	return status;
}


MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	// deregister commands
	status = plugin.deregisterCommand("MVGCmd");

	return status;
}
