#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGGeometryUtil.h"

#include <exception>

using namespace mayaMVG;

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME /!\ 

MVGCreateManipulator::MVGCreateManipulator()
	: _intersectionState(MVGManipulatorUtil::eIntersectionNone)
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
		
	// Preview 3D (while extending edge)
	if(_previewFace3D.length() > 0)
	{
		glLineWidth(1.5f);
		glBegin(GL_LINE_LOOP);
			glVertex3f(_previewFace3D[0].x, _previewFace3D[0].y, _previewFace3D[0].z);
			glVertex3f(_previewFace3D[1].x, _previewFace3D[1].y, _previewFace3D[1].z);
			glVertex3f(_previewFace3D[2].x, _previewFace3D[2].y, _previewFace3D[2].z);
			glVertex3f(_previewFace3D[3].x, _previewFace3D[3].y, _previewFace3D[3].z);
		glEnd();
		glLineWidth(1.f);
		glDisable(GL_LINE_STIPPLE);
			glColor4f(1.f, 1.f, 1.f, 0.6f);
			glBegin(GL_POLYGON);
				glVertex3f(_previewFace3D[0].x, _previewFace3D[0].y, _previewFace3D[0].z);
				glVertex3f(_previewFace3D[1].x, _previewFace3D[1].y, _previewFace3D[1].z);
				glVertex3f(_previewFace3D[2].x, _previewFace3D[2].y, _previewFace3D[2].z);
				glVertex3f(_previewFace3D[3].x, _previewFace3D[3].y, _previewFace3D[3].z);
			glEnd();
	}
	
	// Draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
		
		// Draw preview 2D
		drawPreview2D(view, data);
		
		// Draw Camera points
		glColor3f(1.f, 0.5f, 0.f);
		MVGManipulatorUtil::drawCameraPoints(view, data);
		
		// Draw only in active view
		if(MVGMayaUtil::isActiveView(view))
		{
			// Draw intersections
			MVGManipulatorUtil::drawIntersections(view, data, _intersectionData, _intersectionState);
		}
				
	MVGDrawUtil::end2DDrawing();
	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
	MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
	if(!data)
		return MS::kFailure;
	
	MVGEditCmd* cmd = NULL;
	if(!_ctx) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}

	short mousex, mousey;
	mousePosition(mousex, mousey);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
	
	// Define selectionState
	updateIntersectionState(view, data, mousex, mousey);
			
	switch(_intersectionState) {
		case MVGManipulatorUtil::eIntersectionNone: {			
			
			data->buildPoints2D.append(mousePoint);
			
			MPointArray facePoints3D;	
			MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D);

			if(!addCreateFaceCommand(view, data, cmd, facePoints3D))
				return MS::kFailure;

			// Add points to camera
			MPoint point2D;
			for(size_t i = 0; i < facePoints3D.length(); ++i)
			{
				MVGGeometryUtil::worldToCamera(view, data->camera, facePoints3D[i], point2D);
				data->camera.addClickedPoint(point2D);
				data->cameraPoints2D.append(point2D);
			}
			
			// Add points to map
			for(size_t i = 0; i < facePoints3D.length(); ++i)
			{
				MVGGeometryUtil::worldToCamera(view, data->camera, facePoints3D[i], point2D);
				const std::pair<std::string, MPoint> cameraPair = std::make_pair(data->camera.name(), point2D);
				const std::pair<std::string, MPoint> meshPair = std::make_pair(MVGProject::_MESH, facePoints3D[i]);
				MVGProject::_pointsMap.insert(std::make_pair(cameraPair, meshPair));
			}
			
			data->buildPoints2D.clear();
			break;
		}
		case MVGManipulatorUtil::eIntersectionPoint: {
			LOG_INFO("SELECT POINT")
			break;
		}
		case MVGManipulatorUtil::eIntersectionEdge: {
			LOG_INFO("SELECT EDGE")
				
			MPointArray cameraPoints = data->cameraPoints2D;
			
			// Compute height and ratio 2D
			MPoint& edgePoint0 = cameraPoints[_intersectionData.edgePointIndexes[0]];
			MPoint& edgePoint1 = cameraPoints[_intersectionData.edgePointIndexes[1]];
			MVector ratioVector2D = edgePoint1 - mousePoint;
			_intersectionData.edgeHeight2D =  edgePoint1 - edgePoint0;
			_intersectionData.edgeRatio = ratioVector2D.length() / _intersectionData.edgeHeight2D.length();
			
			// Compute height 3D
			std::pair<std::string, MPoint> cameraPair = std::make_pair(data->camera.name(), edgePoint0);
			MPoint& edgePoint3D_0 = MVGProject::_pointsMap.at(cameraPair).second;
			cameraPair = std::make_pair(data->camera.name(), edgePoint1);
			MPoint& edgePoint3D_1 = MVGProject::_pointsMap.at(cameraPair).second;	
			_intersectionData.edgeHeight3D = edgePoint3D_1 - edgePoint3D_0;
			
			break;
		}
	}

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{	
	MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
	if(!data)
		return MS::kFailure;
	
	MVGEditCmd* cmd = NULL;
	if(!_ctx) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
		
	switch(_intersectionState) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:	
			break;
		case MVGManipulatorUtil::eIntersectionEdge: {
			LOG_INFO("CREATE POLYGON W/ TMP EDGE")
			
			if(!addCreateFaceCommand(view, data, cmd, _previewFace3D))
				return MS::kFailure;
			
			_previewFace3D.clear();
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
	updateIntersectionState(view, data, mousex, mousey);
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{		
	MVGManipulatorUtil::DisplayData* data = MVGManipulatorUtil::getCachedDisplayData(view, _cache);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
	
	MPointArray cameraPoints = data->cameraPoints2D;
	
	switch(_intersectionState) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:
			break;
		case MVGManipulatorUtil::eIntersectionEdge: {
			{
			//LOG_INFO("MOVE TMP EDGE")
			MPointArray previewPoints2D;
			
			previewPoints2D.append(cameraPoints[_intersectionData.edgePointIndexes[1]]);
			previewPoints2D.append(cameraPoints[_intersectionData.edgePointIndexes[0]]);
			MPoint P3 = mousePoint - (1 - _intersectionData.edgeRatio) * _intersectionData.edgeHeight2D;
			MPoint P4 = mousePoint + _intersectionData.edgeRatio * _intersectionData.edgeHeight2D;
			previewPoints2D.append(P3);
			previewPoints2D.append(P4);
			
			_previewFace3D.clear();
			MVGGeometryUtil::projectFace2D(view, _previewFace3D, data->camera, previewPoints2D, true, _intersectionData.edgeHeight3D);
				
			// TODO[1]: Keep the old first 2 points to have a connected face
			// TODO[2] : compute plane with straight line constraint

			break;
			}
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

void MVGCreateManipulator::updateIntersectionState(M3dView& view, MVGManipulatorUtil::DisplayData* data, double mousex, double mousey)
{
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	if(MVGManipulatorUtil::intersectPoint(view, data, _intersectionData, mousex, mousey)) {
		_intersectionState = MVGManipulatorUtil::eIntersectionPoint;
	} else if(MVGManipulatorUtil::intersectEdge(view, data, _intersectionData, mousex, mousey)) {
	 	_intersectionState = MVGManipulatorUtil::eIntersectionEdge;
	} else {
		_intersectionState = MVGManipulatorUtil::eIntersectionNone;
	}
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

bool MVGCreateManipulator::addCreateFaceCommand(M3dView& view, MVGManipulatorUtil::DisplayData* data, MVGEditCmd* cmd, MPointArray& facePoints3D)
{
	// Undo/redo
	if(facePoints3D.length() < 4)
		return false;
	cmd = (MVGEditCmd *)_ctx->newCmd();
	if(!cmd) {
	  LOG_ERROR("invalid command object.")
	  return false;
	}

	// Create face
	MDagPath meshPath;
	MVGMayaUtil::getDagPathByName(MVGProject::_MESH.c_str(), meshPath);
	cmd->doAddPolygon(meshPath, facePoints3D);
	if(cmd->redoIt())
		cmd->finalize();
	
	return true;
}