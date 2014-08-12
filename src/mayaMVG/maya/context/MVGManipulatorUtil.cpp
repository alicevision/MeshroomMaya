#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MMatrix.h>

namespace mayaMVG {
	
MVGManipulatorUtil::MVGManipulatorUtil()
	: _intersectionState(eIntersectionNone)
	, _ctx(NULL)
{
	_intersectionData.pointIndex = -1;
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
			double s = MVGGeometryUtil::crossProduct2D(AB, PA) / (AB.length()*AB.length());
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
bool MVGManipulatorUtil::computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
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
    
    // Update face informations
    MVGMesh mesh(_intersectionData.meshName);
   _intersectionData.facePointIndexes.clear();
    MIntArray connectedFaceIndex = mesh.getConnectedFacesToVertex(_intersectionData.edgePointIndexes[0]);
    if(connectedFaceIndex.length() > 0)
    {
       _intersectionData.facePointIndexes = mesh.getFaceVertices(connectedFaceIndex[0]);
       return true;
    }
    
    return false;
}
	

void MVGManipulatorUtil::drawPreview3D()
{
	if(_previewFace3D.length() > 0)
	{
        MVector color(0.f, 0.f, 1.f);
        MVGDrawUtil::drawLineLoop3D(_previewFace3D, color);
        color = MVector(1.f, 1.f, 1.f);
        MVGDrawUtil::drawPolygon3D(_previewFace3D, color, 0.6f);
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

}	// mayaMVG