#include <maya/M3dView.h>
#include <maya/MToolsInfo.h>
#include "MVGManipContainer.h"
#include "MVGContext.h"

using namespace mayaMVG;

namespace {
	M3dView currentView() {
		return M3dView::active3dView();
	}
}

MVGContext::MVGContext()
{
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
	if (event.mouseButton() == MEvent::kLeftMouse) {
		if (!event.isModifierShift() && !event.isModifierControl()
		        && !event.isModifierMiddleMouseButton()) {
			short x, y;
			event.getPosition(x, y);
			return MS::kSuccess;
		}
	}
	return MPxContext::doPress(event);
}

MStatus MVGContext::doDrag(MEvent & event)
{
	currentView().refresh();
	return MS::kSuccess;
}

MStatus MVGContext::doRelease(MEvent & event)
{
	return MPxContext::doRelease(event);
}

void MVGContext::getClassName(MString & name) const
{
	name.set("MVGTool");
}

// static
void MVGContext::updateManipulators(void * data)
{
	// delete all manipulators
	MVGContext * ctxPtr = static_cast<MVGContext *>(data);
	ctxPtr->deleteManipulators();

	// then add a new one
	MString manipName("MVGManip");
	MObject manipObject;
	MVGManipContainer * manipulator =
	    static_cast<MVGManipContainer *>(MVGManipContainer::newManipulator(
	            manipName, manipObject));
	if (manipulator) {
		ctxPtr->addManipulator(manipObject);
		manipulator->setContext(ctxPtr);
	}
}
