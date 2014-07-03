#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGGeometryUtil.h"


using namespace mayaMVG;

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME /!\ 


MVGCreateManipulator::MVGCreateManipulator()
	: _selectionState(SS_NONE)
	, _ctx(NULL)
{
}

MVGCreateManipulator::~MVGCreateManipulator()
{
}

void * MVGCreateManipulator::creator()
{
	return new MVGCreateManipulator();
}

MStatus MVGCreateManipulator::initialize()
{
	return MS::kSuccess;
}

void MVGCreateManipulator::postConstructor()
{
	registerForMouseMove();
}

void MVGCreateManipulator::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	DisplayData* data = getCachedDisplayData(view);
	if(!data)
		return;

	view.beginGL();

	// enable gl picking
	// will call manipulator::doPress/doRelease 
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());
	
	// select color
	switch(_selectionState){
		case SS_NONE:
			glColor3f(1.f, 1.f, 1.f);
			break;
		case SS_POINT:
		case SS_EDGE:
			glColor3f(0.f, 1.f, 0.f);
			break;
	}
	
	// draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME

		MPointArray points = data->cameraPoints;
		for(int i = 0; i < points.length(); ++i) {
			short x, y;
			MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
			MVGDrawUtil::drawCircle(x, y, 10, 30);
		}

	MVGDrawUtil::end2DDrawing();
	
	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
	// MVGEditCmd* cmd = NULL;
	// if(!_ctx) {
	// 	LOG_ERROR("invalid context object.")
	// 	return MS::kFailure;
	// }
	// cmd = (MVGEditCmd *)_ctx->newCmd();
	// if(!cmd) {
	// 	LOG_ERROR("invalid command object.")
	// 	return MS::kFailure;
	// }
	// cmd->doCreate();
	// cmd->finalize();

	switch(_selectionState) {
		case SS_NONE: {
			DisplayData* data = getCachedDisplayData(view);
			if(!data)
				return MS::kFailure;
			short mousex, mousey;
			mousePosition(mousex, mousey);
			MPoint point;
			MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, point);
			data->camera.addClickedPoint(point);
			data->cameraPoints.append(point);
			_selectionState = SS_POINT;
			break;
		}
		case SS_POINT: {
			LOG_INFO("SELECT POINT")
			break;
		}
		case SS_EDGE: {
			LOG_INFO("SELECT EDGE")
			break;
		}
	}

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{	
	switch(_selectionState) {
		case SS_NONE:
		case SS_POINT:
			break;
		case SS_EDGE: {
			LOG_INFO("CREATE POLYGON W/ TMP EDGE")
			break;
		}
	}
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGCreateManipulator::doMove(M3dView& view, bool& refresh)
{	
	DisplayData* data = getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	if(data->cameraPoints.length() == 0)
		return MS::kSuccess;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	if(intersectPoint(view, data->camera, mousex, mousey)) {
		_selectionState = SS_POINT;
	} else if(intersectEdge(view, data->camera, mousex, mousey)) {
	 	_selectionState = SS_EDGE;
	} else {
		_selectionState = SS_NONE;
	}
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{	
	switch(_selectionState) {
		case SS_NONE:
		case SS_POINT:
			break;
		case SS_EDGE: {
			LOG_INFO("MOVE TMP EDGE")
			break;
		}
	}
	return MPxManipulatorNode::doDrag(view);
}

void MVGCreateManipulator::preDrawUI(const M3dView& view)
{
}

void MVGCreateManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}

void MVGCreateManipulator::setContext(MVGContext* ctx)
{
	_ctx = ctx;
}

MVGCreateManipulator::DisplayData* MVGCreateManipulator::getCachedDisplayData(M3dView& view)
{
	if(!MVGMayaUtil::isMVGView(view))
		return NULL;
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	std::map<std::string, DisplayData>::iterator it = _cache.find(cameraPath.fullPathName().asChar());
	if(it == _cache.end()) {
		MVGCamera c(cameraPath);
		if(c.isValid()) {
			DisplayData data;
			data.camera = c;
			data.cameraPoints = c.getClickedPoints();
			_cache[cameraPath.fullPathName().asChar()] = data;
			return &_cache[cameraPath.fullPathName().asChar()];
		}
	} else {
		return &(it->second);
	}
	return NULL;
}

bool MVGCreateManipulator::intersectPoint(M3dView& view, const MVGCamera& camera, const short&x, const short& y)
{
	double threshold = 4.0/view.portHeight();//(2 + camera.getZoom()) * 5;
	MPointArray points = camera.getClickedPoints();
	MPoint cameraPoint;
	MVGGeometryUtil::viewToCamera(view, camera, x, y, cameraPoint);
	for(int i = 0; i < points.length(); ++i) {
		if(points[i].x <= cameraPoint.x + threshold && points[i].x >= cameraPoint.x - threshold
			&& points[i].y <= cameraPoint.y + threshold && points[i].y >= cameraPoint.y - threshold)
			return true;
	}
	return false;
}

bool MVGCreateManipulator::intersectEdge(M3dView& view, const MVGCamera& camera, const short&x, const short& y)
{
	return false;
}
