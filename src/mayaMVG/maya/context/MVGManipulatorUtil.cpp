#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGMesh.h"
#include <maya/MMatrix.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>

using namespace mayaMVG;
	
MVGManipulatorUtil::MVGManipulatorUtil(MVGContext* context)
	: _context(context)
	, _intersectionState(eIntersectionNone)
{
	_intersectionData.pointIndex = -1;
}

bool MVGManipulatorUtil::intersectPoint(M3dView& view, DisplayData* displayData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	
	double threshold = (POINT_RADIUS*displayData->camera.getZoom())/(double)view.portWidth();
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, x, y, mousePoint);
	std::map<std::string, std::vector<MVGPoint2D> >& meshMap = displayData->allPoints2D;
	for(std::map<std::string, std::vector<MVGPoint2D> >::iterator it = meshMap.begin(); it != meshMap.end(); ++it)
	{
		std::vector<MVGPoint2D>& meshPoints = it->second;
		for(int i = 0; i < meshPoints.size(); ++i)
		{
			MPoint p;
			MVGGeometryUtil::worldToCamera(view, meshPoints[i].point3D, p);
			if(p.x <= mousePoint.x + threshold && p.x >= mousePoint.x - threshold
			&& p.y <= mousePoint.y + threshold && p.y >= mousePoint.y - threshold)
			{
				_intersectionData.pointIndex = i;
				_intersectionData.meshName =  it->first;
				return true;
			}
		}
	}
		
	resetIntersections();
	return false;
}

bool MVGManipulatorUtil::intersectEdge(M3dView& view, DisplayData* displayData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, x, y, mousePoint);
	
	double minDistanceFound = -1.0;
	double distance;
	MIntArray tmp;
	std::string tmpMesh;
	
	// Browse meshes
	for(std::map<std::string, std::vector<MIntArray> >::iterator it = _cacheMeshToEdgeArray.begin(); it != _cacheMeshToEdgeArray.end(); ++it)
	{
		std::vector<MIntArray>& edgesArray = it->second;
		if(edgesArray.size() < 1)
			return false;
		
		// Browse edges
		std::vector<MVGPoint2D>& mvgPoints = displayData->allPoints2D[it->first];
		for(std::vector<MIntArray>::iterator edgeIt = edgesArray.begin(); edgeIt != edgesArray.end(); ++edgeIt)
		{
			if(mvgPoints.size() < (*edgeIt)[0] || mvgPoints.size() < (*edgeIt)[1])
				continue;
			MPoint A, B;
			MVGGeometryUtil::worldToCamera(view, mvgPoints[(*edgeIt)[0]].point3D, A);
			MVGGeometryUtil::worldToCamera(view, mvgPoints[(*edgeIt)[1]].point3D, B);

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
			// length of orthogonal projection on edge
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

	double threshold = (POINT_RADIUS*displayData->camera.getZoom())/(double)view.portWidth();
	if(minDistanceFound < -threshold || minDistanceFound > threshold)
	{
		resetIntersections();
		return false;
	}
	
	_intersectionData.edgePointIndexes = tmp;
	_intersectionData.meshName = tmpMesh;
	return true;
}

void MVGManipulatorUtil::updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey)
{
    if(data->allPoints2D.empty())
        return;
	resetIntersections();
	if(intersectPoint(view, data, mousex, mousey))
		_intersectionState = MVGManipulatorUtil::eIntersectionPoint;
	else if(intersectEdge(view, data, mousex, mousey))
		_intersectionState = MVGManipulatorUtil::eIntersectionEdge;
}

bool MVGManipulatorUtil::computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[_intersectionData.meshName];
	if(_intersectionData.edgePointIndexes.length() == 0 || mvgPoints.size() < _intersectionData.edgePointIndexes[0] || mvgPoints.size() < _intersectionData.edgePointIndexes[1])
		return false;
	// Compute height and ratio 2D
	MPoint pWorld0 = mvgPoints[_intersectionData.edgePointIndexes[0]].point3D;
	MPoint pWorld1 = mvgPoints[_intersectionData.edgePointIndexes[1]].point3D;

	MPoint pCamera0, pCamera1;
	MVGGeometryUtil::worldToCamera(view, pWorld0, pCamera0);
	MVGGeometryUtil::worldToCamera(view, pWorld1, pCamera1);

	MVector ratioVector2D = pCamera1 - mousePointInCameraCoord;
	_intersectionData.edgeHeight2D =  pCamera1 - pCamera0;
	_intersectionData.edgeRatio = ratioVector2D.length() / _intersectionData.edgeHeight2D.length();

	// Compute height 3D
	_intersectionData.edgeHeight3D = pWorld1 - pWorld0;

	// Update face informations
	MVGMesh mesh(_intersectionData.meshName);
	_intersectionData.facePointIndexes.clear();
	MIntArray connectedFaceIndex = mesh.getConnectedFacesToVertex(_intersectionData.edgePointIndexes[0]);
	if(connectedFaceIndex.length() > 0) {
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

void MVGManipulatorUtil::rebuild()
{
	// Clear cache
	_cacheCameraToDisplayData.clear();

	if(_cacheMeshToPointArray.size() != _cacheMeshToMovablePoint.size()
		|| _cacheMeshToMovablePoint.size() != _cacheMeshToEdgeArray.size())
		return; // assert

	M3dView view;
	MDagPath cameraPath;
	for(size_t i=0; i < M3dView::numberOf3dViews(); ++i) {
		// Get M3dView & update viewing parameters
		M3dView::get3dView(i, view);
		if(!MVGMayaUtil::isMVGView(view))
			continue;
		view.getCamera(cameraPath);
		view.updateViewingParameters();
		// Get associated camera
		MVGCamera c(cameraPath);
		if(!c.isValid())
			continue;
		DisplayData data;
		data.camera = c;
		// For each mesh
		std::map<std::string, std::vector<MVGPoint2D> > newMap;
		for(std::map<std::string, MPointArray>::iterator it = _cacheMeshToPointArray.begin(); it != _cacheMeshToPointArray.end(); ++it) {
			std::vector<MVGPoint2D> points2D;
			// For each vertex
			MPointArray& points3D = it->second;
			if(_cacheMeshToMovablePoint[it->first].size() != points3D.length())
				continue; // assert
			for(int i = 0; i < points3D.length(); ++i) {
				MVGPoint2D mvgPoint;
				mvgPoint.point3D = points3D[i];
				mvgPoint.movableState = _cacheMeshToMovablePoint[it->first].at(i);
				points2D.push_back(mvgPoint);
			}
			newMap[it->first] = points2D;
		}
		data.allPoints2D = newMap;		
		_cacheCameraToDisplayData[cameraPath.fullPathName().asChar()] = data;
	}
}

MStatus MVGManipulatorUtil::rebuildAllMeshesCacheFromMaya()
{
	MStatus status;
	// Clear all
	resetMeshCache();
	// Retrieves all meshes
	MDagPath path;
	std::vector<MVGMesh> meshes = MVGMesh::list();
	std::vector<MVGMesh>::const_iterator it = meshes.begin();
	for(; it != meshes.end(); ++it) {
		status = rebuildMeshCacheFromMaya(it->dagPath());
		CHECK(status)
	}
	return status;
}

MStatus MVGManipulatorUtil::rebuildMeshCacheFromMaya(const MDagPath& meshPath)
{
	MStatus status;
	MFnMesh fnMesh(meshPath, &status);
	CHECK_RETURN_STATUS(status)
	std::string meshName = meshPath.fullPathName().asChar();
	
	// // ??????????
	// fnMesh.syncObject();
	// fnMesh.updateSurface();

	// MPlug plugMesh;
	// MObject meshData;
	// plugMesh = meshfn.findPlug(MString("outMesh"), &status);
	// MDGContext dgContext(0.0);
	// status = plugMesh.getValue(meshData, dgContext);
	// MFnMeshn fnMesh(meshData, &status);

	// Save mesh points
	MPointArray meshPoints;
	if(!fnMesh.getPoints(meshPoints, MSpace::kWorld))
		return MS::kFailure;
	_cacheMeshToPointArray[meshName] = meshPoints;

	// Save movable state
	MIntArray faceList;
	std::vector<EPointState> movableStates;
	MItMeshVertex vertexIt(meshPath, MObject::kNullObj, &status);
	CHECK_RETURN_STATUS(status)
	while(!vertexIt.isDone()) {
		vertexIt.getConnectedFaces(faceList);
		if(faceList.length() > 1) // Point connected to several faces = unmovable
			movableStates.push_back(eUnMovable);
		else if(faceList.length() > 0) { // Face points connected to several face		
			MIntArray faceVerticesIndexes;
			fnMesh.getPolygonVertices(faceList[0], faceVerticesIndexes);
			// For each point, check number of connected faces
			int numConnectedFace;
			EPointState state = eMovableRecompute;
			for(int i = 0; i < faceVerticesIndexes.length(); ++i) {
				MItMeshVertex vertexIter(meshPath, MObject::kNullObj);
				int previousIndex;
				vertexIter.setIndex(faceVerticesIndexes[i], previousIndex);
				vertexIter.numConnectedFaces(numConnectedFace);
				if(numConnectedFace > 1) {
					state = eMovableInSamePlane;
					break;
				}
			}
			movableStates.push_back(state);
		}
		vertexIt.next();
	}
	_cacheMeshToMovablePoint[meshName] = movableStates;

	// Save mesh edges
	std::vector<MIntArray> meshEdges;
	MItMeshEdge edgeIt(meshPath, MObject::kNullObj, &status);
	CHECK_RETURN_STATUS(status)
	while(!edgeIt.isDone()) {
		MIntArray pointIndexArray;
		pointIndexArray.append(edgeIt.index(0));
		pointIndexArray.append(edgeIt.index(1));
		meshEdges.push_back(pointIndexArray);
		edgeIt.next();
	}
	_cacheMeshToEdgeArray[meshName] = meshEdges;	

	return status;
}

MVGManipulatorUtil::DisplayData* MVGManipulatorUtil::getDisplayData(M3dView& view)
{
	MDagPath path;
	view.getCamera(path);
	std::map<std::string, DisplayData>::iterator it = _cacheCameraToDisplayData.find(path.fullPathName().asChar());
	if(it != _cacheCameraToDisplayData.end())
		return &(it->second);
	return NULL;
}

MVGManipulatorUtil::DisplayData* MVGManipulatorUtil::getComplementaryDisplayData(M3dView& view)
{
	MDagPath path;
	view.getCamera(path);
	std::map<std::string, MVGManipulatorUtil::DisplayData>::iterator it = _cacheCameraToDisplayData.begin();
	for(;it != _cacheCameraToDisplayData.end(); ++it) {
		if(it->first != path.fullPathName().asChar()) {
			return &(it->second);
		}
	}
	return NULL;
}

void MVGManipulatorUtil::resetCache()
{
	_cacheCameraToDisplayData.clear();	
}

void MVGManipulatorUtil::resetMeshCache()
{
	_cacheMeshToPointArray.clear();
	_cacheMeshToMovablePoint.clear();
	_cacheMeshToEdgeArray.clear();
}

void MVGManipulatorUtil::resetTemporaryData()
{
	_previewFace3D.clear();
	for(std::map<std::string, DisplayData>::iterator it = _cacheCameraToDisplayData.begin(); it != _cacheCameraToDisplayData.end(); ++it)
		it->second.buildPoints2D.clear();
}

void MVGManipulatorUtil::resetIntersections()
{
	_intersectionState = eIntersectionNone;
	_intersectionData.meshName.clear();
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	_intersectionData.edgeHeight3D = MVector(0, 0, 0);
	_intersectionData.edgeHeight2D = MVector(0, 0, 0);
	_intersectionData.edgeRatio = 0;
	_intersectionData.facePointIndexes.clear();
}

bool MVGManipulatorUtil::addCreateFaceCommand(const MDagPath& meshPath, const MPointArray& facePoints3D)
{
	// Undo/redo
	if(facePoints3D.length() < 4)
		return false;

	MVGEditCmd* cmd = (MVGEditCmd *)_context->newCmd();
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
    
    rebuildAllMeshesCacheFromMaya();    // TODO : only rebuild created mesh
    rebuild();
	return true;
}

bool MVGManipulatorUtil::addUpdateFaceCommand(const MDagPath& meshPath, const MPointArray& newFacePoints3D, const MIntArray& verticesIndexes)
{
	if(newFacePoints3D.length() != verticesIndexes.length())
	{
		LOG_ERROR("Need an ID per point")
		return false;
	}
	
    MVGEditCmd* cmd = (MVGEditCmd *)_context->newCmd();
    if(!cmd) {
      LOG_ERROR("invalid command object.")
      return false;
    }

	cmd->doMove(meshPath, newFacePoints3D, verticesIndexes);
	if(cmd->redoIt())
		cmd->finalize();
    
    rebuildMeshCacheFromMaya(meshPath);
    rebuild();
	return true;
}
