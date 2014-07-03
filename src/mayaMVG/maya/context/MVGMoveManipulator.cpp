#include "mayaMVG/maya/context/MVGMoveManipulator.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"


using namespace mayaMVG;

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME /!\ 


MVGMoveManipulator::MVGMoveManipulator()
{	
}

MVGMoveManipulator::~MVGMoveManipulator()
{
}

void * MVGMoveManipulator::creator()
{
	return new MVGMoveManipulator();
}

MStatus MVGMoveManipulator::initialize()
{
	return MS::kSuccess;
}

void MVGMoveManipulator::postConstructor()
{
	registerForMouseMove();
}

void MVGMoveManipulator::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	// draw only in current view
	if(!MVGMayaUtil::isActiveView(view)	|| (!MVGMayaUtil::isMVGView(view)))
		return;

	// draw something
	view.beginGL();
	view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
{	
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{	
	refresh = true;
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{	
	return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::preDrawUI(const M3dView& view)
{
}

void MVGMoveManipulator::setContext(MVGContext* ctx)
{
	_ctx = ctx;
}

void MVGMoveManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}
