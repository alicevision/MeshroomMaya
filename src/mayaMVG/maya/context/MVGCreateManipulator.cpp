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
	MVGManipulatorUtil::DisplayData* data = getCachedDisplayData(view);
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
		drawPreview2D(view, data);
	

	MVGDrawUtil::end2DDrawing();
	
	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
	 MVGEditCmd* cmd = NULL;
	 if(!_ctx) {
	 	LOG_ERROR("invalid context object.")
	 	return MS::kFailure;
	 }


	switch(_selectionState) {
		case SS_NONE: {
			MVGManipulatorUtil::DisplayData* data = getCachedDisplayData(view);
			if(!data)
				return MS::kFailure;
			short mousex, mousey;
			mousePosition(mousex, mousey);
			MPoint point;
			MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, point);
			data->camera.addClickedPoint(point);
			data->cameraPoints.append(point);
			_selectionState = SS_POINT;
			
			// undo/redo
			if(data->cameraPoints.length() < 4)
				break;
			cmd = (MVGEditCmd *)_ctx->newCmd();
			if(!cmd) {
			  LOG_ERROR("invalid command object.")
			  return MS::kFailure;
			}
			
			// Create face
			MPointArray facePoints3D;	
			MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->cameraPoints);
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(MVGProject::_MESH.c_str(), meshPath);
			cmd->doAddPolygon(meshPath, facePoints3D);
			if(cmd->redoIt())
				cmd->finalize();
			data->cameraPoints.clear();
			data->camera.clearClickedPoints();
			_selectionState = SS_NONE;
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
	MVGManipulatorUtil::DisplayData* data = getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	if(data->cameraPoints.length() == 0)
		return MS::kSuccess;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	if(MVGManipulatorUtil::intersectPoint(view, data, mousex, mousey)) {
		_selectionState = SS_POINT;
	} else if(MVGManipulatorUtil::intersectEdge(view, data, mousex, mousey)) {
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

MVGManipulatorUtil::DisplayData* MVGCreateManipulator::getCachedDisplayData(M3dView& view)
{
	if(!MVGMayaUtil::isMVGView(view))
		return NULL;
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	std::map<std::string, MVGManipulatorUtil::DisplayData>::iterator it = _cache.find(cameraPath.fullPathName().asChar());
	if(it == _cache.end()) {
		MVGCamera c(cameraPath);
		if(c.isValid()) {
			MVGManipulatorUtil::DisplayData data;
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



void MVGCreateManipulator::drawPreview2D(M3dView& view, MVGManipulatorUtil::DisplayData* data)
{
	short x, y;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	
	MPointArray points = data->cameraPoints;
	if(points.length() > 0)
	{
		for(int i = 0; i < points.length() - 1; ++i) {
			MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
			MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
			
			glBegin(GL_LINES);
				MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
				glVertex2f(x, y);
				MVGGeometryUtil::cameraToView(view, data->camera, points[i+1], x, y);
				glVertex2f(x, y);
			glEnd();
		}
		
		// Last point to mouse
		MVGGeometryUtil::cameraToView(view, data->camera, points[points.length() - 1], x, y);
		MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
		glBegin(GL_LINES);
			MVGGeometryUtil::cameraToView(view, data->camera, points[points.length() - 1], x, y);
			glVertex2f(x, y);
			glVertex2f(mousex, mousey);
		glEnd();	
		
		
	}
	if(points.length() > 2)
	{
		glColor4f(0.f, 0.f, 1.f, 0.8f);
		glLineWidth(1.5f);
		glBegin(GL_LINE_LOOP);
			for(int i = 0; i < 3; ++i) {			
				MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
				glVertex2f(x, y);
			}
			glVertex2f(mousex, mousey);
		glEnd();
		
		glColor4f(1.f, 1.f, 1.f, 0.6f);
		glBegin(GL_POLYGON);
			for(int i = 0; i < 3; ++i) {			
				MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
				glVertex2f(x, y);
			}
			glVertex2f(mousex, mousey);
		glEnd();
	}
}