#include <QWidget>
#include "qt/MVGEventFilter.h"
#include "util/MVGUtil.h"
#include "util/MVGLog.h"
#include "MVGManipContainer.h"
#include "MVGContext.h"
#include <maya/M3dView.h>
#include <maya/MToolsInfo.h>



using namespace mayaMVG;

namespace {
	M3dView currentView() {
		return M3dView::active3dView();
	}
}

MVGContext::MVGContext()
	: m_eventFilter(NULL)
{
	setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent & event)
{
	updateManipulators(this);
	// install context event filter
	m_eventFilter = new MVGContextEventFilter(this);
	QWidget* leftViewport = MVGUtil::getMVGLeftViewportLayout();
	QWidget* rightViewport = MVGUtil::getMVGRightViewportLayout();
	if(!leftViewport || !rightViewport)
		return;
	leftViewport->installEventFilter(m_eventFilter);
	rightViewport->installEventFilter(m_eventFilter);
}

void MVGContext::toolOffCleanup()
{
	// remove context event filter
	QWidget* leftViewport = MVGUtil::getMVGLeftViewportLayout();
	QWidget* rightViewport = MVGUtil::getMVGRightViewportLayout();
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

void MVGContext::setMousePos(size_t x, size_t y) {
	m_mousePosX = x;
	m_mousePosY = currentView().portHeight() - y;
	updateManipulators(this);
}

size_t MVGContext::mousePosX() const 
{
	return m_mousePosX;
}

size_t MVGContext::mousePosY() const 
{
	return m_mousePosY;
}
