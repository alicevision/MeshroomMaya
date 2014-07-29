#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"

namespace mayaMVG {

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME /!\ 

MVGCreateManipulator::MVGCreateManipulator()
{
	_manipUtils.intersectionData().pointIndex = -1;
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
	_manipUtils.drawPreview3D();
	
	// Draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
		
		// Draw only in active view
		if(MVGMayaUtil::isActiveView(view))
		{
			glColor3f(1.f, 0.f, 0.f);
			drawPreview2D(view, data);
		
			drawIntersections(view);
			
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
	if(!_manipUtils.getContext()) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
	
	switch(_manipUtils.intersectionState()) {
		case MVGManipulatorUtil::eIntersectionNone: {			
			
			data->buildPoints2D.append(mousePoint);
						
			// Create face if enough points (4))
			if(data->buildPoints2D.length() < 4)
				break;
			
			// Compute 3D face
			MPointArray facePoints3D;	
			MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D);

			MDagPath emptyPath = MDagPath();
			if(!_manipUtils.addCreateFaceCommand(cmd, emptyPath, facePoints3D))
				return MS::kFailure;
	
			data->buildPoints2D.clear();
			break;
		}
		case MVGManipulatorUtil::eIntersectionPoint:
			LOG_INFO("SELECT POINT")
			break;
		case MVGManipulatorUtil::eIntersectionEdge:
			_manipUtils.computeEdgeIntersectionData(view, data, mousePoint);		
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
	if(!_manipUtils.getContext()) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
		
	switch(_manipUtils.intersectionState()) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:	
			break;
		case MVGManipulatorUtil::eIntersectionEdge: {
			//LOG_INFO("CREATE POLYGON W/ TMP EDGE")
			
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(_manipUtils.intersectionData().meshName.c_str(), meshPath);
			if(!_manipUtils.addCreateFaceCommand(cmd, meshPath, _manipUtils.previewFace3D()))
				return MS::kFailure;
			
			_manipUtils.previewFace3D().clear();
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
//	if(data->buildPoints2D.length() == 0 && MVGProjectWrapper::instance().getCacheMeshToPointArray().size() == 0)
//		return MS::kSuccess;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	_manipUtils.updateIntersectionState(view, data, mousex, mousey);
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
		
	switch(_manipUtils.intersectionState()) {
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
	_manipUtils.setContext(ctx);
}

void MVGCreateManipulator::drawIntersections(M3dView& view)
{
	// DISPLAY DATA
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;
	
	if(data->allPoints2D.size() == 0)
		return;
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& meshPoints = data->allPoints2D[intersectionData.meshName];
	
	short x, y;
	switch(_manipUtils.intersectionState())
		{
			case MVGManipulatorUtil::eIntersectionPoint:
				glColor4f(0.3f, 0.3f, 0.6f, 0.8f);	// Grey
				
				MVGGeometryUtil::cameraToView(view, data->camera, meshPoints[intersectionData.pointIndex].projectedPoint3D, x, y);
				MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
				
				break;
			case MVGManipulatorUtil::eIntersectionEdge:				
				glColor4f(0.9f, 0.9f, 0.1f, 0.8f);

				glBegin(GL_LINES);
					MVGGeometryUtil::cameraToView(view, data->camera, meshPoints[intersectionData.edgePointIndexes[0]].projectedPoint3D, x, y);
					glVertex2f(x, y);
					MVGGeometryUtil::cameraToView(view, data->camera, meshPoints[intersectionData.edgePointIndexes[1]].projectedPoint3D, x, y);
					glVertex2f(x, y);
				glEnd();	
				break;
		}	
	
	// Temporary caches
//	std::map<std::string, MPointArray>& meshCache = MVGProjectWrapper::instance().getCacheMeshToPointArray();
//	if(meshCache.size() > 0) 
//	{
//		MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
//		MPointArray meshPoints = meshCache[intersectionData.meshName];
//		MPoint pointViewCoord_0;
//		MPoint pointViewCoord_1;
//
//		switch(_manipUtils.intersectionState())
//		{
//			case MVGManipulatorUtil::eIntersectionPoint:
//				glColor4f(0.3f, 0.3f, 0.6f, 0.8f);	// Grey
//				pointViewCoord_0 = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.pointIndex]);
//
//				MVGDrawUtil::drawCircle(pointViewCoord_0.x, pointViewCoord_0.y, POINT_RADIUS, 30);
//				break;
//			case MVGManipulatorUtil::eIntersectionEdge:				
//				glColor4f(0.9f, 0.9f, 0.1f, 0.8f);
//
//				pointViewCoord_0 = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.edgePointIndexes[0]]);
//				pointViewCoord_1 = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.edgePointIndexes[1]]);
//				glBegin(GL_LINES);
//					glVertex2f(pointViewCoord_0.x, pointViewCoord_0.y);
//					glVertex2f(pointViewCoord_1.x, pointViewCoord_1.y);
//				glEnd();	
//				break;
//		}	
//	}
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

void MVGCreateManipulator::computeTmpFaceOnEdgeExtend(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[intersectionData.meshName];
	
	// Get edge 3D points 
	MPoint edgePoint3D_0 = mvgPoints[intersectionData.edgePointIndexes[0]].point3D;
	MPoint edgePoint3D_1 = mvgPoints[intersectionData.edgePointIndexes[1]].point3D;

	// Build 2D points preview to compute 3D face
	MPointArray previewPoints2D;
	previewPoints2D.append(mvgPoints[intersectionData.edgePointIndexes[0]].projectedPoint3D);
	previewPoints2D.append(mvgPoints[intersectionData.edgePointIndexes[1]].projectedPoint3D);
	MPoint P3 = mousePointInCameraCoord - (1 - intersectionData.edgeRatio) * intersectionData.edgeHeight2D;
	MPoint P4 = mousePointInCameraCoord + intersectionData.edgeRatio * intersectionData.edgeHeight2D;
	previewPoints2D.append(P3);
	previewPoints2D.append(P4);

	// Compute 3D face
	_manipUtils.previewFace3D().clear();
	if(MVGGeometryUtil::projectFace2D(view, _manipUtils.previewFace3D(), data->camera, previewPoints2D, true, intersectionData.edgeHeight3D))
	{
		// Keep the old first 2 points to have a connected face
		_manipUtils.previewFace3D()[0] = edgePoint3D_1;
		_manipUtils.previewFace3D()[1] = edgePoint3D_0;
	}
	
	// TODO[1] : If compute failed, use the plan of the extended fae
	// TODO[2] : compute plane with straight line constraint
}
}	//mayaMVG