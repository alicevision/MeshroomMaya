#include <QWidget>
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/qt/MVGMayaEvent.h"

using namespace mayaMVG;

MVGContext::MVGContext()
	: m_eventFilter(NULL)
{
	setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent & event)
{
	updateManipulators(this);
	// install context event filter
	QWidget* leftViewport = MVGMayaUtil::getMVGLeftViewportLayout();
	QWidget* rightViewport = MVGMayaUtil::getMVGRightViewportLayout();
	if(!leftViewport || !rightViewport)
		return;
	m_eventFilter = new MVGContextEventFilter(this, leftViewport);
	leftViewport->installEventFilter(m_eventFilter);
	rightViewport->installEventFilter(m_eventFilter);
}

void MVGContext::toolOffCleanup()
{
	// remove context event filter
	QWidget* leftViewport = MVGMayaUtil::getMVGLeftViewportLayout();
	QWidget* rightViewport = MVGMayaUtil::getMVGRightViewportLayout();
	if(leftViewport)
		leftViewport->installEventFilter(m_eventFilter);
	if(rightViewport)
		rightViewport->installEventFilter(m_eventFilter);
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
	name.set("MVGTool");
}

// static
void MVGContext::updateManipulators(void * data)
{
	// delete all manipulators
	MVGContext* ctxPtr = static_cast<MVGContext*>(data);
	ctxPtr->deleteManipulators();

	// then add a new one
	MString manipName("MVGBuildFaceManipulator");
	MObject manipObject;
	MPxManipulatorNode::newManipulator(manipName, manipObject);
	ctxPtr->addManipulator(manipObject);
}
