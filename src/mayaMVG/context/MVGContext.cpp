#include <QWidget>
#include "mayaMVG/qt/MVGEventFilter.h"
#include "mayaMVG/util/MVGUtil.h"
#include "mayaMVG/util/MVGLog.h"
#include "mayaMVG/context/MVGManipContainer.h"
#include "mayaMVG/context/MVGContext.h"
#include <maya/M3dView.h>
#include <maya/MToolsInfo.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>



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
	
	if (event.mouseButton() == MEvent::kLeftMouse) {
		if (!event.isModifierShift() && !event.isModifierControl()
		        && !event.isModifierMiddleMouseButton()) {

			short x, y;
			event.getPosition(x, y);
			if(m_points.size() > 3) {
				
				// get distance to origin
				double distance = m_points[0].wpos.distanceTo(MPoint(0,0,0));

				// as MPointArray
				MPointArray vertices;
				for(size_t i = 0; i < m_points.size(); ++i)
					vertices.append(m_points[i].wpos+m_points[i].wdir*10); // FIXME

				MFnMesh fn;
				MObject m = fn.addPolygon(vertices, true, kMFnMeshPointTolerance, m_mesh);
				if(m_mesh == MObject::kNullObj) {
					MDagPath p;
					MDagPath::getAPathTo(m, p);
					p.extendToShape();
					m_mesh = p.node();
				}

				m_points.clear();
				return MPxContext::doPress(event);
			}
			MVGPoint p;
			currentView().viewToWorld(x, y, p.wpos, p.wdir);
			p.wdir.normalize();
			m_points.push_back(p);
			currentView().refresh();
		}
	}

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

MVGContext::vpoint_t MVGContext::points() const
{
	return m_points;	
}
