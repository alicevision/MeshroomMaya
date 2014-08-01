#include "mayaMVG/maya/context/MVGMoveManipulator.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "openMVG/multiview/triangulation.hpp"

namespace mayaMVG {
	
namespace {
	double crossProduct2d(MVector& A, MVector& B) {
		return A.x*B.y - A.y*B.x;
	}

	double dotProduct2d(MVector& A, MVector& B) {
		return A.x*B.x - A.y*B.y;
	}
	
	bool edgesIntersection(MPoint A, MPoint B, MVector AD,  MVector BC)
	{		
		// r x s = 0
		double cross = crossProduct2d(AD, BC);
		double eps = 0.00001;
		if(cross < eps && cross > -eps)
			return false;

		MVector AB = B - A;

		double x =  crossProduct2d(AB, BC) / crossProduct2d(AD, BC);
		double y = crossProduct2d(AB, AD) / crossProduct2d(AD, BC);

		if( x >= 0 
			&& x <= 1 
			&& y >= 0 
			&& y <= 1)
		{
			return true;
		}
		return false;
	}
}

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME /!\ 


MVGMoveManipulator::MVGMoveManipulator() :
	_moveState(eMoveNone)
{	
	_manipUtils.intersectionData().pointIndex = -1;
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
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;
	
	short mousex, mousey;
	updateMouse(view, data, mousex, mousey);
	
	view.beginGL();

	// Enable gl picking
	// Will call manipulator::doPress/doRelease 
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());
		
	// Preview 3D 
	_manipUtils.drawPreview3D();
	
	// Draw	
	MVGDrawUtil::begin2DDrawing(view);
	MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
						
	if(MVGMayaUtil::isActiveView(view))
	{			
		drawCursor(mousex, mousey);
		drawIntersections(view);

		switch(_moveState)
		{
			case eMoveNone:
				break;
			case eMovePoint:
				if(_manipUtils.getContext()->getKeyPressed() == MVGContext::eKeyNone)
				{
//						glColor3f(0.9f, 0.5f, 0.4f);
//						MVGDrawUtil::drawFullCross(mousex, mousey, 10, 2);
				}
				break;
			case eMoveEdge:
				break;
		}
	}
		
	MVGDrawUtil::end2DDrawing();
	view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
	// Left Button
	Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
	if(!(mouseButtons & Qt::LeftButton))
		return MS::kFailure;

	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	short mousex, mousey;
	MPoint mousePoint = updateMouse(view, data, mousex, mousey);
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();	
	
	_manipUtils.updateIntersectionState(view, data, mousex, mousey);	
	switch(_manipUtils.intersectionState())
	{
		case MVGManipulatorUtil::eIntersectionNone:
			break;
		case MVGManipulatorUtil::eIntersectionPoint:
		{	
			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
					break;
				case MVGContext::eKeyCtrl:
				case MVGContext::eKeyShift:
					// Update face informations
					MVGMesh mesh(intersectionData.meshName);
					intersectionData.facePointIndexes.clear();
					MIntArray connectedFaceIndex = mesh.getConnectedFacesToVertex(intersectionData.pointIndex);
					if(connectedFaceIndex.length() > 0)
						intersectionData.facePointIndexes = mesh.getFaceVertices(connectedFaceIndex[0]);				

					break;					
			}

			_moveState = eMovePoint;
			break;
		}
		case MVGManipulatorUtil::eIntersectionEdge:
			
			_manipUtils.computeEdgeIntersectionData(view, data, mousePoint);	
			
			// Update face informations
			MVGMesh mesh(intersectionData.meshName);
			intersectionData.facePointIndexes.clear();
			MIntArray connectedFaceIndex = mesh.getConnectedFacesToVertex(intersectionData.edgePointIndexes[0]);
			if(connectedFaceIndex.length() > 0)
			{
				intersectionData.facePointIndexes = mesh.getFaceVertices(connectedFaceIndex[0]);	
				_moveState = eMoveEdge;
			}
			break;
	}
	
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
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
	MPoint mousePoint = updateMouse(view, data, mousex, mousey);
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	switch(_moveState) {
		case eMoveNone:
			break;
		case eMovePoint:
		{
			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
				{
					MPoint triangulatedPoint;
					if(!triangulate(view, intersectionData, mousePoint, triangulatedPoint))
						break;
					
					MDagPath meshPath;
					MVGMayaUtil::getDagPathByName(intersectionData.meshName.c_str(), meshPath);

					MPointArray newPoints;
					newPoints.append(triangulatedPoint);
					MIntArray indexes;
					indexes.append(intersectionData.pointIndex);

					_manipUtils.addUpdateFaceCommand(cmd, meshPath, newPoints, indexes);
					intersectionData.facePointIndexes.clear();
					break;
				}
				case MVGContext::eKeyCtrl:
				case MVGContext::eKeyShift:
				{
					if(_manipUtils.previewFace3D().length() == 0)
						break;

					MDagPath meshPath;
					MVGMayaUtil::getDagPathByName(intersectionData.meshName.c_str(), meshPath);

					_manipUtils.addUpdateFaceCommand(cmd, meshPath, _manipUtils.previewFace3D(), intersectionData.facePointIndexes);
					_manipUtils.previewFace3D().clear();
					intersectionData.facePointIndexes.clear();
					break;		
				}
			}
			break;
		}		
		case eMoveEdge: 
		{
			if(_manipUtils.getContext()->getKeyPressed() ==  MVGContext::eKeyNone)
				break;			
			if(_manipUtils.previewFace3D().length() == 0)
				break;
			
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(intersectionData.meshName.c_str(), meshPath);
			
			MPointArray edgePoints;
			edgePoints.append(_manipUtils.previewFace3D()[2]);
			edgePoints.append(_manipUtils.previewFace3D()[3]);
			_manipUtils.addUpdateFaceCommand(cmd, meshPath, edgePoints, intersectionData.edgePointIndexes);
			_manipUtils.previewFace3D().clear();
			intersectionData.facePointIndexes.clear();
			break;
		}
	}

	_moveState = eMoveNone;
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{	
	refresh = true;
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;

	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	//if(MVGProjectWrapper::instance().getCacheMeshToPointArray().size() > 0)
	if(data->allPoints2D.size() > 0)
		_manipUtils.updateIntersectionState(view, data, mousex, mousey);
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{		
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	MPoint mousePoint = updateMouse(view, data, mousex, mousey);
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& meshPoints = data->allPoints2D[intersectionData.meshName];
	switch(_moveState) {
		case eMoveNone:
			break;
		case eMovePoint:
			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
					break;
				case MVGContext::eKeyCtrl:
					if(meshPoints[intersectionData.pointIndex].movableState >= eMovableInSamePlane)
						computeTmpFaceOnMovePoint(view, data, mousePoint);
					break;
				case MVGContext::eKeyShift:
					if(meshPoints[intersectionData.pointIndex].movableState == eMovableRecompute)
						computeTmpFaceOnMovePoint(view, data, mousePoint, true);
					break;					
			}
			break;
		case eMoveEdge: 
		{
			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
					break;
				case MVGContext::eKeyCtrl:
					if(meshPoints[intersectionData.edgePointIndexes[0]].movableState >= eMovableInSamePlane
						&& meshPoints[intersectionData.edgePointIndexes[1]].movableState >= eMovableInSamePlane)
					{
						computeTmpFaceOnMoveEdge(view, data, mousePoint);
					}	
					break;
				case MVGContext::eKeyShift:
					if(meshPoints[intersectionData.edgePointIndexes[0]].movableState >= eMovableInSamePlane
						&& meshPoints[intersectionData.edgePointIndexes[1]].movableState >= eMovableInSamePlane)
					{
						computeTmpFaceOnMoveEdge(view, data, mousePoint, true);
					}
					break;					
			}
			break;
		}
	}
	
	return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::preDrawUI(const M3dView& view)
{
}

void MVGMoveManipulator::setContext(MVGContext* ctx)
{
	_manipUtils.setContext(ctx);
}

MPoint MVGMoveManipulator::updateMouse(M3dView& view, DisplayData* data, short& mousex, short& mousey)
{
	mousePosition(mousex, mousey);
	MPoint mousePointInCameraCoord;
	MVGGeometryUtil::viewToCamera(view, mousex, mousey, mousePointInCameraCoord);
	
	return mousePointInCameraCoord;
}

void MVGMoveManipulator::drawCursor(float mousex, float mousey)
{
	glColor4f(0.f, 0.f, 0.f, 0.8f);
	MVGDrawUtil::drawArrowsCursor(mousex, mousey);
	
	switch(_manipUtils.getContext()->getKeyPressed())
	{
		case MVGContext::eKeyNone:
			drawTriangulateCursor(mousex, mousey);
			break;
		case MVGContext::eKeyCtrl:
			drawMoveInPlaneCursor(mousex, mousey);
			break;
		case MVGContext::eKeyShift:
			drawMoveRecomputePlaneCursor(mousex, mousey);
			break;					
	}
}



void MVGMoveManipulator::drawTriangulateCursor(float mousex, float mousey)
{
	glColor3f(0.9f, 0.5f, 0.4f);
	MVGDrawUtil::drawFullCross(mousex + 10, mousey + 10, 5, 1);
}

void MVGMoveManipulator::drawMoveInPlaneCursor(float mousex, float mousey)
{
	glColor3f(0.f, 1.f, 0.f);
	MVGDrawUtil::drawPlaneItem(mousex + 12, mousey + 10);
}

void MVGMoveManipulator::drawMoveRecomputePlaneCursor(float mousex, float mousey)
{
	glColor3f(0.f, 1.f, 1.f);
	MVGDrawUtil::drawPointCloudItem(mousex + 10, mousey + 10);
}
		
void MVGMoveManipulator::drawIntersections(M3dView& view)
{
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
		{
			EPointState movableState = meshPoints[intersectionData.pointIndex].movableState;

			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
					glColor4f(0.3f, 0.3f, 0.6f, 0.8f);	// Grey
					break;
				case MVGContext::eKeyCtrl:
					if(movableState >= eMovableInSamePlane)
						glColor3f(0.f, 1.f, 0.f);
					else
						glColor3f(1.f, 0.f, 0.f);
					break;
				case MVGContext::eKeyShift:
					if(movableState == eMovableRecompute)
						glColor4f(0.f, 1.f, 1.f, 0.8f);
					else
						glColor3f(1.f, 0.f, 0.f);
					break;	
			}

			MVGGeometryUtil::cameraToView(view, meshPoints[intersectionData.pointIndex].projectedPoint3D, x, y);
			MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
			break;
		}
		case MVGManipulatorUtil::eIntersectionEdge:	
			std::vector<EPointState> movableStates;
			movableStates.push_back(meshPoints[intersectionData.edgePointIndexes[0]].movableState);
			movableStates.push_back(meshPoints[intersectionData.edgePointIndexes[1]].movableState);

			switch(_manipUtils.getContext()->getKeyPressed())
			{
				case MVGContext::eKeyNone:
					glColor4f(0.3f, 0.3f, 0.6f, 0.8f);	// Grey
					break;
				case MVGContext::eKeyCtrl:
					if((movableStates[0] >= eMovableInSamePlane)
						&& (movableStates[1] >= eMovableInSamePlane))
					{
						glColor3f(0.f, 1.f, 0.f);
					}
					else
					{
						glColor3f(1.f, 0.f, 0.f);
					}
					break;
				case MVGContext::eKeyShift:
					if((movableStates[0] >= eMovableInSamePlane)
						&& (movableStates[1] >= eMovableInSamePlane))
					{
						glColor4f(0.f, 1.f, 1.f, 0.8f);
					}
					else
					{
						glColor3f(1.f, 0.f, 0.f);
					}
					break;	
			}

			glLineWidth(1.5f);
			glBegin(GL_LINES);
				MVGGeometryUtil::cameraToView(view, meshPoints[intersectionData.edgePointIndexes[0]].projectedPoint3D, x, y);
				glVertex2f(x, y);
				MVGGeometryUtil::cameraToView(view, meshPoints[intersectionData.edgePointIndexes[1]].projectedPoint3D, x, y);
				glVertex2f(x, y);
			glEnd();	
			break;
	}	
}

void MVGMoveManipulator::computeTmpFaceOnMovePoint(M3dView& view, DisplayData* data, MPoint& mousePoint, bool recompute)
{
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[intersectionData.meshName];

	MIntArray verticesId = intersectionData.facePointIndexes;
	MPointArray& previewFace3D = _manipUtils.previewFace3D();

	if(recompute)
	{
		MPointArray previewPoints2D;
		MPoint wpos;
		for(int i = 0; i < verticesId.length(); ++i)
		{
			if(intersectionData.pointIndex == verticesId[i])
				previewPoints2D.append(mousePoint);
			else
				previewPoints2D.append(mvgPoints[verticesId[i]].projectedPoint3D);
		}
		
		// Compute face		
		previewFace3D.clear();
		MVGGeometryUtil::projectFace2D(view, previewFace3D, data->camera, previewPoints2D);
	}
	else {
		// Fill previewFace3D
		MPointArray facePoints3D;
		for(int i = 0; i < verticesId.length(); ++i)
		{
			facePoints3D.append(mvgPoints[verticesId[i]].point3D);
		}
		MPoint movedPoint;
		PlaneKernel::Model model;

		MVGGeometryUtil::computePlane(facePoints3D, model);
		MVGGeometryUtil::projectPointOnPlane(mousePoint, view, model, data->camera, movedPoint);

		previewFace3D.clear();
		previewFace3D.setLength(4);
		for(int i = 0; i < verticesId.length(); ++i)
		{
			if(intersectionData.pointIndex == verticesId[i])
				previewFace3D[i] = movedPoint;
			else
				previewFace3D[i] = facePoints3D[i];
		}		
	}
}

void MVGMoveManipulator::computeTmpFaceOnMoveEdge(M3dView& view, DisplayData* data, MPoint& mousePoint, bool recompute)
{
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[intersectionData.meshName];
	MPointArray& previewFace3D = _manipUtils.previewFace3D();
	MIntArray verticesId = intersectionData.facePointIndexes;
	MIntArray edgeVerticesId = intersectionData.edgePointIndexes;
	MIntArray fixedVerticesId;
	for(int i = 0; i < verticesId.length(); ++i)
	{
		int found = false;
		for(int j = 0; j < edgeVerticesId.length(); ++j)
		{
			if(verticesId[i] == edgeVerticesId[j])
				found = true;
		}

		if(!found)
			fixedVerticesId.append(verticesId[i]);
	}

	// Switch order if necessary
	if(fixedVerticesId[0] == verticesId[0]
		&& fixedVerticesId[1] == verticesId[verticesId.length() - 1])
	{
		MIntArray tmp = fixedVerticesId;
		fixedVerticesId[0] = tmp[1];
		fixedVerticesId[1] = tmp[0];
	}
		
	if(recompute)
	{
		MPointArray previewPoints2D;

		// First : fixed points
		previewPoints2D.append(mvgPoints[fixedVerticesId[0]].projectedPoint3D);
		previewPoints2D.append(mvgPoints[fixedVerticesId[1]].projectedPoint3D);

		// Then : mousePoints computed with egdeHeight and ratio							
		MPoint P3 = mousePoint + intersectionData.edgeRatio * intersectionData.edgeHeight2D;
		previewPoints2D.append(P3);
		MPoint P4 = mousePoint  - (1 - intersectionData.edgeRatio) * intersectionData.edgeHeight3D;
		previewPoints2D.append(P4);
		
		// Only set the new points to keep a connected face
		previewFace3D.clear();
		MVGGeometryUtil::projectFace2D(view, previewFace3D, data->camera, previewPoints2D, true, -intersectionData.edgeHeight3D);

		// Check points order	
		MVector AD =  previewFace3D[3] - previewFace3D[0];
		MVector BC =  previewFace3D[2] - previewFace3D[1];

		if(edgesIntersection(previewFace3D[0], previewFace3D[1], AD, BC))
		{
			MPointArray tmp = previewFace3D;
			previewFace3D[3] = tmp[2];
			previewFace3D[2] = tmp[3];
		}
	}
	else {
		
		// Fill previewFace3D
		MPointArray facePoints3D;
		for(int i = 0; i < verticesId.length(); ++i)
		{
			facePoints3D.append(mvgPoints[verticesId[i]].point3D);
		}

		// Compute plane with old face points
		MPoint movedPoint;
		PlaneKernel::Model model;
		MVGGeometryUtil::computePlane(facePoints3D, model);

		previewFace3D.clear();
		previewFace3D.setLength(4);		
		previewFace3D[0] = mvgPoints[fixedVerticesId[0]].point3D;
		previewFace3D[1] = mvgPoints[fixedVerticesId[1]].point3D;

		// Project new points on plane			
		MPoint P3 = mousePoint + _manipUtils.intersectionData().edgeRatio * _manipUtils.intersectionData().edgeHeight2D;
		MVGGeometryUtil::projectPointOnPlane(P3, view, model, data->camera, movedPoint);
		previewFace3D[3] = movedPoint;

		// Keep 3D length
		MPoint lastPoint3D = movedPoint - _manipUtils.intersectionData().edgeHeight3D;
		MPoint lastPoint2D;
		MVGGeometryUtil::worldToCamera(view, lastPoint3D, lastPoint2D);
		MVGGeometryUtil::projectPointOnPlane(lastPoint2D, view, model, data->camera, movedPoint);
		previewFace3D[2] = movedPoint;

	}
}

bool MVGMoveManipulator::triangulate(M3dView& view, MVGManipulatorUtil::IntersectionData& intersectionData, MPoint& mousePointInCameraCoord, MPoint& resultPoint3D)
{
	// Points in camera coordinates
	MPointArray points2D;
	points2D.append(mousePointInCameraCoord);

	std::map<std::string, DisplayData>& displayDataCache = MVGProjectWrapper::instance().getDisplayDataCache();
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	DisplayData* otherData;
	for(std::map<std::string, DisplayData>::iterator it = displayDataCache.begin(); it != displayDataCache.end(); ++it)
	{
		if(it->first != cameraPath.fullPathName().asChar())
			otherData = (&it->second);
	}
	if(!otherData)
		return false;
	std::vector<MVGPoint2D>& mvgPoints = otherData->allPoints2D[intersectionData.meshName];
	points2D.append(mvgPoints[intersectionData.pointIndex].projectedPoint3D);

	// MVGCameras
	 std::vector<MVGCamera> cameras;
	 cameras.push_back(MVGCamera(cameraPath));
	 cameras.push_back(otherData->camera);

	MVGGeometryUtil::triangulatePoint(points2D, cameras, resultPoint3D);

	return true;
}

void MVGMoveManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}
}	//mayaMVG
