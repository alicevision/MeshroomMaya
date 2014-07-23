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
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;

	view.beginGL();

	// enable gl picking
	// will call manipulator::doPress/doRelease 
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());

	// Preview 3D (while extending edge)
	drawPreview3D();
	
	// Draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
		
		// Draw only in active view
		if(MVGMayaUtil::isActiveView(view))
		{
			glColor3f(1.f, 0.f, 0.f);
			drawPreview2D(view, data);
		
			MVGManipulatorUtil::drawIntersections(view, data, _intersectionData, _intersectionState);
		}

	MVGDrawUtil::end2DDrawing();
	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{	
	// Left Button
	Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
	if(!(mouseButtons & Qt::LeftButton))
		return MS::kFailure;
	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	// Undo/Redo
	MVGEditCmd* cmd = NULL;
	if(!_ctx) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
	
	switch(_intersectionState) {
		case MVGManipulatorUtil::eIntersectionNone: {			
			
			data->buildPoints2D.append(mousePoint);
						
			// Create face if enough points (4))
			if(data->buildPoints2D.length() < 4)
				break;
			
			// Compute 3D face
			MPointArray facePoints3D;	
			MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D);

			if(!addCreateFaceCommand(view, data, cmd, facePoints3D))
				return MS::kFailure;
	
			data->buildPoints2D.clear();
			break;
		}
		case MVGManipulatorUtil::eIntersectionPoint:
			LOG_INFO("SELECT POINT")
			break;
		case MVGManipulatorUtil::eIntersectionEdge:
			computeEdgeIntersectionData(view, data, mousePoint);		
			break;
	}

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	// Undo/Redo
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
			//LOG_INFO("CREATE POLYGON W/ TMP EDGE")
			
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
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	//Needed ? 
	if(data->buildPoints2D.length() == 0 && MVGProjectWrapper::instance().getCacheMeshToPointArray().size() == 0)
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
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
		
	switch(_intersectionState) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:
			break;
		case MVGManipulatorUtil::eIntersectionEdge: 
		{
			//LOG_INFO("MOVE TMP EDGE")
			computeTmpFaceOnEdgeExtend(view, data, mousePoint);
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

void MVGCreateManipulator::updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey)
{
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	if(MVGManipulatorUtil::intersectPoint(view, data, _intersectionData, mousex, mousey)) {
		_intersectionState = MVGManipulatorUtil::eIntersectionPoint;
	} 
	else if(MVGManipulatorUtil::intersectEdge(view, data, _intersectionData, mousex, mousey)) {
	 	_intersectionState = MVGManipulatorUtil::eIntersectionEdge;
	} 
	else {
		_intersectionState = MVGManipulatorUtil::eIntersectionNone;
	}
}
	
void MVGCreateManipulator::drawPreview2D(M3dView& view, DisplayData* data)
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

void MVGCreateManipulator::drawPreview3D()
{
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
}

void MVGCreateManipulator::computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	std::map<std::string, MPointArray>& meshCache = MVGProjectWrapper::instance().getCacheMeshToPointArray();
	
	// Compute height and ratio 2D
	MPoint edgePoint3D_0 = meshCache.at(_intersectionData.meshName)[_intersectionData.edgePointIndexes[0]];
	MPoint edgePoint3D_1 = meshCache.at(_intersectionData.meshName)[_intersectionData.edgePointIndexes[1]];
	MPoint edgePoint0, edgePoint1;

	MVGGeometryUtil::worldToCamera(view, data->camera, edgePoint3D_0, edgePoint0);
	MVGGeometryUtil::worldToCamera(view, data->camera, edgePoint3D_1, edgePoint1);

	MVector ratioVector2D = edgePoint1 - mousePointInCameraCoord;
	_intersectionData.edgeHeight2D =  edgePoint1 - edgePoint0;
	_intersectionData.edgeRatio = ratioVector2D.length() / _intersectionData.edgeHeight2D.length();

	// Compute height 3D
	_intersectionData.edgeHeight3D = edgePoint3D_1 - edgePoint3D_0;
}

void MVGCreateManipulator::computeTmpFaceOnEdgeExtend(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	// Get edge 3D points 
	std::map<std::string, MPointArray>& pointsCache = MVGProjectWrapper::instance().getCacheMeshToPointArray();
	MPoint edgePoint3D_0 = pointsCache.at(_intersectionData.meshName)[_intersectionData.edgePointIndexes[0]];
	MPoint edgePoint3D_1 = pointsCache.at(_intersectionData.meshName)[_intersectionData.edgePointIndexes[1]];
	MPoint edgePoint0, edgePoint1;

	// Compute edge points in camera coords
	MVGGeometryUtil::worldToCamera(view, data->camera, edgePoint3D_0, edgePoint0);
	MVGGeometryUtil::worldToCamera(view, data->camera, edgePoint3D_1, edgePoint1);

	// Build 2D points preview to compute 3D face
	MPointArray previewPoints2D;
	previewPoints2D.append(edgePoint1);
	previewPoints2D.append(edgePoint0);
	MPoint P3 = mousePointInCameraCoord - (1 - _intersectionData.edgeRatio) * _intersectionData.edgeHeight2D;
	MPoint P4 = mousePointInCameraCoord + _intersectionData.edgeRatio * _intersectionData.edgeHeight2D;
	previewPoints2D.append(P3);
	previewPoints2D.append(P4);

	// Compute 3D face
	_previewFace3D.clear();
	if(MVGGeometryUtil::projectFace2D(view, _previewFace3D, data->camera, previewPoints2D, true, _intersectionData.edgeHeight3D))
	{
		// Keep the old first 2 points to have a connected face
		_previewFace3D[0] = edgePoint3D_1;
		_previewFace3D[1] = edgePoint3D_0;
	}

	

	// TODO[2] : compute plane with straight line constraint
}

bool MVGCreateManipulator::addCreateFaceCommand(M3dView& view, DisplayData* data, MVGEditCmd* cmd, const MPointArray& facePoints3D)
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