#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MMatrix.h>

namespace mayaMVG {
	
MVGManipulatorUtil::MVGManipulatorUtil()
	: _intersectionState(eIntersectionNone)
	, _ctx(NULL)
{
	_intersectionData.pointIndex = -1;
}
		
MVGManipulatorUtil::~MVGManipulatorUtil()
{
}

bool MVGManipulatorUtil::intersectPoint(M3dView& view, DisplayData* displayData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	
	double threshold = (2*POINT_RADIUS*displayData->camera.getZoom())/view.portHeight();
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, x, y, mousePoint);

	std::map<std::string, std::vector<MVGPoint2D> >& meshMap = displayData->allPoints2D;
	for(std::map<std::string, std::vector<MVGPoint2D> >::iterator it = meshMap.begin(); it != meshMap.end(); ++it)
	{
		std::vector<MVGPoint2D>& meshPoints = it->second;
		for(int i = 0; i < meshPoints.size(); ++i)
		{
			//MVGGeometryUtil::worldToCamera(view, displayData->camera, meshPoints[i], pointCameraCoord);
			if(meshPoints[i].projectedPoint3D.x <= mousePoint.x + threshold && meshPoints[i].projectedPoint3D.x >= mousePoint.x - threshold
			&& meshPoints[i].projectedPoint3D.y <= mousePoint.y + threshold && meshPoints[i].projectedPoint3D.y >= mousePoint.y - threshold)
			{
				_intersectionData.pointIndex = i;
				_intersectionData.meshName =  it->first;

				return true;
			}
		}
	}
		
	_intersectionData.pointIndex = -1;
	return false;
}

bool MVGManipulatorUtil::intersectEdge(M3dView& view, DisplayData* displayData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, x, y, mousePoint);
	
	double minDistanceFound = -1.0;
	double tolerance = 0.001 * displayData->camera.getZoom() * 30;
	double distance;
	MIntArray tmp;
	std::string tmpMesh;
	
	// Browse meshes
	std::map<std::string, std::vector<MIntArray> >& meshMap= MVGProjectWrapper::instance().getCacheMeshToEdgeArray();
	for(std::map<std::string, std::vector<MIntArray> >::iterator it = meshMap.begin(); it != meshMap.end(); ++it)
	{
		std::vector<MIntArray>& edgesArray = it->second;
		if(edgesArray.size() < 1)
			return false;
		
		// Browse edges
		std::vector<MVGPoint2D>& mvgPoints = displayData->allPoints2D[it->first];
		for(std::vector<MIntArray>::iterator edgeIt = edgesArray.begin(); edgeIt != edgesArray.end(); ++edgeIt)
		{
			MPoint A = mvgPoints[(*edgeIt)[0]].projectedPoint3D;
			MPoint B = mvgPoints[(*edgeIt)[1]].projectedPoint3D;
				
			MVector AB = B - A;
			MVector PA = A - mousePoint;
			MVector AP = mousePoint - A;
			MVector BP = mousePoint - B;
			MVector BA = A -B;
			// Dot signs			
			int sign1, sign2;
			((AP*AB) > 0) ? sign1 = 1 : sign1 = -1;
			((BP*BA) > 0) ? sign2 = 1 : sign2 = -1;
			if(sign1 != sign2)
				continue;
			// Lenght of orthogonal projection on edge
			double s = crossProduct2D(AB, PA) / (AB.length()*AB.length());
			if(s < 0)
				s *= -1;
			distance = s * AB.length();
			if(minDistanceFound < 0.0 || distance < minDistanceFound)
			{
				tmp.clear();
				tmp.append((*edgeIt)[0]);
				tmp.append((*edgeIt)[1]);
				tmpMesh = it->first;

				minDistanceFound = distance;
			}
		}
	}

	if(minDistanceFound < -tolerance || minDistanceFound > tolerance)
	{
		_intersectionData.edgePointIndexes.clear();
		_intersectionData.meshName = "";
		return false;
	}
	
	_intersectionData.edgePointIndexes = tmp;
	_intersectionData.meshName = tmpMesh;
	return true;
}

void MVGManipulatorUtil::updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey)
{
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	if(intersectPoint(view, data, mousex, mousey)) {
		_intersectionState = MVGManipulatorUtil::eIntersectionPoint;
	} 
	else if(intersectEdge(view, data, mousex, mousey)) {
		_intersectionState = MVGManipulatorUtil::eIntersectionEdge;
	} 
	else {
		_intersectionState = MVGManipulatorUtil::eIntersectionNone;
	}
}
void MVGManipulatorUtil::computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{	
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[_intersectionData.meshName];
	
	// Compute height and ratio 2D
	MPoint edgePoint3D_0 = mvgPoints[_intersectionData.edgePointIndexes[0]].point3D;
	MPoint edgePoint3D_1 = mvgPoints[_intersectionData.edgePointIndexes[1]].point3D;

	MVector ratioVector2D = mvgPoints[_intersectionData.edgePointIndexes[1]].projectedPoint3D - mousePointInCameraCoord;
	_intersectionData.edgeHeight2D =  mvgPoints[_intersectionData.edgePointIndexes[1]].projectedPoint3D - mvgPoints[_intersectionData.edgePointIndexes[0]].projectedPoint3D;
	_intersectionData.edgeRatio = ratioVector2D.length() / _intersectionData.edgeHeight2D.length();

	// Compute height 3D
	_intersectionData.edgeHeight3D = edgePoint3D_1 - edgePoint3D_0;
}
	

void MVGManipulatorUtil::drawPreview3D()
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

bool MVGManipulatorUtil::addCreateFaceCommand(MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& facePoints3D)
{
	// Undo/redo
	if(facePoints3D.length() < 4)
		return false;
	cmd = (MVGEditCmd *)_ctx->newCmd();
	if(!cmd) {
	  LOG_ERROR("invalid command object.")
	  return false;
	}

	// FIX ME - Convert to KObject space
	MPointArray objectPoints;
	for(int i = 0; i < facePoints3D.length(); ++i)
	{
		MPoint objPoint = facePoints3D[i] * meshPath.inclusiveMatrixInverse();	
		objectPoints.append(objPoint);
	}
	
	// Create face
	cmd->doAddPolygon(meshPath, objectPoints);
	if(cmd->redoIt())
		cmd->finalize();
	
	return true;
}

bool MVGManipulatorUtil::addUpdateFaceCommand(MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& newFacePoints3D, const MIntArray& verticesIndexes)
{
	
	if(newFacePoints3D.length() != verticesIndexes.length())
	{
		LOG_ERROR("Need an ID per point")
		return false;
	}
	
	// Undo/redo
	cmd = (MVGEditCmd *)_ctx->newCmd();
	if(!cmd) {
	  LOG_ERROR("invalid command object.")
	  return false;
	}
	
	cmd->doMove(meshPath, newFacePoints3D, verticesIndexes);
	if(cmd->redoIt())
		cmd->finalize();
}
//void MVGManipulatorUtil::drawCameraPoints(M3dView& view, DisplayData* data)
//{
//	short x, y;
//	MPoint wPoint;
//	MVector wdir;
//	
//	// To test reloadProjectFromMaya (since map are not reload for the moment)
////	const MPointArray& points = data->cameraPoints2D;
////	for(int i = 0; i < points.length(); ++i)
////	{
////		MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
////		MVGDrawUtil::drawFullCross(x, y);
////	}
////	
//	for(Map3Dto2D::iterator mapIt = MVGProjectWrapper::instance().getMap3Dto2D().begin(); mapIt != MVGProjectWrapper::instance().getMap3Dto2D().end(); ++mapIt)
//	{
//		for(std::vector<PairStringToPoint>::iterator vecIt = mapIt->second.begin(); vecIt != mapIt->second.end(); ++vecIt)
//		{
//			if(data->camera.name() == vecIt->first)
//			{
//				// Draw full cross
//				MVGGeometryUtil::cameraToView(view, data->camera, vecIt->second, x, y);
//				MVGDrawUtil::drawFullCross(x, y);
//				
//				// Line toward 3D point
//				MPoint point3D_view = MVGGeometryUtil::worldToView(view, mapIt->first.second);	
//				glEnable(GL_LINE_STIPPLE);
//				glLineStipple(1.f, 0x5555);
//				glBegin(GL_LINES);
//					glVertex2f(x, y);
//					glVertex2f(point3D_view.x, point3D_view.y);
//				glEnd();
//				glDisable(GL_LINE_STIPPLE);
//				
//				view.viewToWorld(x + 5, y - 15, wPoint, wdir);
//			}
//			else
//			{
//				MPoint point3D_view =  MVGGeometryUtil::worldToView(view, mapIt->first.second);
//				MVGDrawUtil::drawEmptyCross(point3D_view.x, point3D_view.y);
//				
//				view.viewToWorld(point3D_view.x + 5, point3D_view.y - 15, wPoint, wdir);
//			}
//				
//			// Text
//			MString str;
//			str += (int)mapIt->second.size();
//			view.drawText(str, wPoint);			
//		}
//	}
//}
}	// mayaMVG