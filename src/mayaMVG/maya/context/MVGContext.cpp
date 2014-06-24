#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"


using namespace mayaMVG;

MVGContext::MVGContext() {
	setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent & event)
{
	updateManipulators(this);
}

void MVGContext::toolOffCleanup()
{
	MPxContext::toolOffCleanup();
}

MStatus MVGContext::doPress(MEvent & event)
{
	return MPxContext::doPress(event);
}

MStatus MVGContext::doDrag(MEvent & event)
{
	return MS::kSuccess;
}

MStatus MVGContext::doRelease(MEvent & event)
{
	return MPxContext::doRelease(event);
}

void MVGContext::getClassName(MString & name) const
{
	name.set("mayaMVGTool");
}

void MVGContext::updateManipulators(void * data)
{
	// delete all manipulators
	MVGContext* ctxPtr = static_cast<MVGContext*>(data);
	ctxPtr->deleteManipulators();
	// then add a new one
	MString manipName("MVGBuildFaceManipulator");
	MObject manipObject;
	MPxManipulatorNode * manipNode = MPxManipulatorNode::newManipulator(manipName, manipObject);
	MVGBuildFaceManipulator* manipulator = dynamic_cast<MVGBuildFaceManipulator*>(manipNode);
	manipulator->_context = this;
	ctxPtr->addManipulator(manipObject);
}

void MVGContext::setCursor(MCursor cursor)
{
	MPxContext::setCursor(cursor);
}
