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
	_intersectionData.pointIndex = -1;
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
	MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
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
		
		// Draw preview 2D
		drawPreview2D(view, data);
		
		// Draw Camera points
		short x, y;
		MPointArray points = data->cameraPoints2D;
		glColor3f(1.f, 0.5f, 0.f);
		for(size_t i = 0; i < points.length(); ++i)
		{
			MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
			MVGDrawUtil::drawFullCross(x, y);
			
			// Draw point intersection
			if(i == _intersectionData.pointIndex)
			{
				glColor3f(0.f, 1.f, 0.f);
				MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
				glColor3f(1.f, 0.5f, 0.f);
			}		
		}
		
		// Draw edges intersections	
		if(points.length() > 0 && _intersectionData.edgePointIndexes.length() > 1)
		{
			short x, y;
			glColor3f(0.f, 1.f, 0.f);
			glBegin(GL_LINES);
				MVGGeometryUtil::cameraToView(view, data->camera, points[_intersectionData.edgePointIndexes[0]], x, y);
				glVertex2f(x, y);
				MVGGeometryUtil::cameraToView(view, data->camera, points[_intersectionData.edgePointIndexes[1]], x, y);
				glVertex2f(x, y);
			glEnd();
		}
	

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

	short mousex, mousey;
	mousePosition(mousex, mousey);
			
	switch(_selectionState) {
		case SS_NONE: {
			MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
			if(!data)
				return MS::kFailure;
			
			MPoint point;
			MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, point);
			data->buildPoints2D.append(point);
			_selectionState = SS_POINT;
			
			// Undo/redo
			if(data->buildPoints2D.length() < 4)
				break;
			cmd = (MVGEditCmd *)_ctx->newCmd();
			if(!cmd) {
			  LOG_ERROR("invalid command object.")
			  return MS::kFailure;
			}
			
			// Create face
			MPointArray facePoints3D;	
			MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D);
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(MVGProject::_MESH.c_str(), meshPath);
			cmd->doAddPolygon(meshPath, facePoints3D);
			if(cmd->redoIt())
				cmd->finalize();

			// Add points to camera
			MPoint point2D;
			for(size_t i = 0; i < facePoints3D.length(); ++i)
			{
				MVGGeometryUtil::worldToCamera(view, data->camera, facePoints3D[i], point2D);
				data->camera.addClickedPoint(point2D);
				data->cameraPoints2D.append(point2D);
			}	
			data->buildPoints2D.clear();
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
	MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
	if(!data)
		return MS::kFailure;
	if(data->buildPoints2D.length() == 0 && data->cameraPoints2D.length() == 0)
		return MS::kSuccess;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	if(MVGManipulatorUtil::intersectPoint(view, data, _intersectionData, mousex, mousey)) {
		_selectionState = SS_POINT;
	} else if(MVGManipulatorUtil::intersectEdge(view, data, _intersectionData, mousex, mousey)) {
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

void MVGCreateManipulator::drawPreview2D(M3dView& view, MVGManipulatorUtil::DisplayData* data)
{
	short x, y;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	
	MPointArray points = data->buildPoints2D;
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