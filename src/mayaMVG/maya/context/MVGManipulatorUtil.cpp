#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MMatrix.h>
// #include <maya/MQtUtil.h>
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
	for(std::map<std::string, std::vector<MIntArray> >::iterator it = _cacheMeshToEdgeArray.begin(); it != _cacheMeshToEdgeArray.end(); ++it)
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

bool MVGManipulatorUtil::addCreateFaceCommand(MDagPath& meshPath, const MPointArray& facePoints3D)
{
	if(facePoints3D.length() < 4)
		return false;
	MVGEditCmd* cmd = (MVGEditCmd*)_context->newCmd();
	if(!cmd) {
	  LOG_ERROR("invalid command object.")
	  return false;
	}

	// FIX ME - Convert to KObject space
	MPointArray objectPoints;
	for(int i = 0; i < facePoints3D.length(); ++i) {
		MPoint objPoint = facePoints3D[i] * meshPath.inclusiveMatrixInverse();	
		objectPoints.append(objPoint);
	}
	
	// Create face
	cmd->doAddPolygon(meshPath, objectPoints);
	if(cmd->redoIt())
		cmd->finalize();
	
	return true;
}

bool MVGManipulatorUtil::addUpdateFaceCommand(MDagPath& meshPath, const MPointArray& newFacePoints3D, const MIntArray& verticesIndexes)
{
	
	if(newFacePoints3D.length() != verticesIndexes.length()) {
		LOG_ERROR("Need an ID per point")
		return false;
	}
	// Undo/redo
	MVGEditCmd* cmd = (MVGEditCmd*)_context->newCmd();
	if(!cmd) {
	  LOG_ERROR("invalid command object.")
	  return false;
	}
	cmd->doMove(meshPath, newFacePoints3D, verticesIndexes);
	if(cmd->redoIt())
		cmd->finalize();
}


MVGManipulatorUtil::DisplayData* MVGManipulatorUtil::getCachedDisplayData(M3dView& view)
{		
	if(!MVGMayaUtil::isMVGView(view))
		return NULL;
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	std::map<std::string, DisplayData>::iterator it = _cacheCameraToDisplayData.find(cameraPath.fullPathName().asChar());
	if(it != _cacheCameraToDisplayData.end())
		return &(it->second);
	return NULL;
}

void MVGManipulatorUtil::rebuildCacheFromMaya() 
{	
	LOG_INFO("MVGManipulatorUtil::rebuildCacheFromMaya")
	_cacheCameraToDisplayData.clear();
	// Rebuild for temporary cache
	// TODO: remove to use directly data from Maya
	std::vector<std::string> visiblePanelNames;
	visiblePanelNames.push_back("mvgLPanel");
	visiblePanelNames.push_back("mvgRPanel");

	M3dView view;
	MStatus status;
	MDagPath cameraPath;
	for(std::vector<std::string>::const_iterator panelIt = visiblePanelNames.begin(); panelIt!= visiblePanelNames.end(); ++panelIt)
	{
		status = M3dView::getM3dViewFromModelPanel(panelIt->c_str(), view);
		CHECK(status);
		view.getCamera(cameraPath);
		view.updateViewingParameters();
		MVGCamera c(cameraPath);
		if(c.isValid()) {
			DisplayData data;
			data.camera = c;
			// Browse meshes
			std::map<std::string, std::vector<MVGPoint2D> > newMap;
			for(std::map<std::string, MPointArray>::iterator it = _cacheMeshToPointArray.begin(); it != _cacheMeshToPointArray.end(); ++it)
			{
				std::vector<MVGPoint2D> points2D;
				// Browse points
				MPointArray& points3D = it->second;
				for(int i = 0; i < points3D.length(); ++i)
				{
					MVGPoint2D mvgPoint;;
					mvgPoint.point3D = points3D[i];
					MPoint point2D;
					MVGGeometryUtil::worldToCamera(view, points3D[i], point2D);
					mvgPoint.projectedPoint3D = point2D;
					mvgPoint.movableState = _cacheMeshToMovablePoint[it->first].at(i);
					points2D.push_back(mvgPoint);
				}
				newMap[it->first] = points2D;
			}
			data.allPoints2D = newMap;		
			_cacheCameraToDisplayData[cameraPath.fullPathName().asChar()] = data;
		}
	}
}

MStatus MVGManipulatorUtil::rebuildAllMeshesCacheFromMaya()
{
	LOG_INFO("MVGManipulatorUtil::rebuildAllMeshesCacheFromMaya")
	MStatus status;
	_cacheMeshToPointArray.clear();
	_cacheMeshToMovablePoint.clear();
	_cacheMeshToEdgeArray.clear();
	// Retrieves all meshes
	MDagPath path;
	MItDependencyNodes it(MFn::kMesh);
	for(; !it.isDone(); it.next()) {
		MFnDependencyNode fn(it.thisNode());
		MDagPath::getAPathTo(fn.object(), path);
		status = rebuildMeshCacheFromMaya(path);
	}
	
	return status;
}

MStatus MVGManipulatorUtil::rebuildMeshCacheFromMaya(MDagPath& meshPath)
{	
	MStatus status;
	MFnMesh fnMesh(meshPath);
	MPointArray meshPoints;
	std::vector<MIntArray> meshEdges;
	
	// Mesh points
	if(!fnMesh.getPoints(meshPoints, MSpace::kWorld))
		return MS::kFailure;
	_cacheMeshToPointArray[meshPath.fullPathName().asChar()] = meshPoints;
	
	// Connected face
	std::vector<EPointState> movableStates;
	MItMeshVertex vertexIt(meshPath, MObject::kNullObj, &status);
	if(!status)
		return MS::kFailure;
	
	MIntArray faceList;
	while(!vertexIt.isDone())
	{
		vertexIt.getConnectedFaces(faceList);
		// Point connected to several faces
		if(faceList.length() > 1)
			movableStates.push_back(eUnMovable);
		
		// Face points connected to several face
		else if(faceList.length() > 0)
		{			
			// Get the points of the first face
			MItMeshPolygon faceIt(meshPath, MObject::kNullObj);
			int prev;
			faceIt.setIndex(faceList[0], prev);
			MIntArray faceVerticesIndexes;
			faceIt.getVertices(faceVerticesIndexes);
			
			// For each point, check number of connected faces
			int numConnectedFace;
			bool check = false;
			for(int i = 0; i < faceVerticesIndexes.length(); ++i)
			{
				MItMeshVertex vertexIter(meshPath, MObject::kNullObj);
				vertexIter.setIndex(faceVerticesIndexes[i], prev);
				vertexIter.numConnectedFaces(numConnectedFace);
				if(numConnectedFace > 1)
				{
					movableStates.push_back(eMovableInSamePlane);
					check = true;
					break;
				}		
			}
			if(!check)
				movableStates.push_back(eMovableRecompute);
		}
		vertexIt.next();
	}
	_cacheMeshToMovablePoint[meshPath.fullPathName().asChar()] = movableStates;	
	
	// Mesh edges
	MItMeshEdge edgeIt(meshPath, MObject::kNullObj, &status);
	if(!status)
		return MS::kFailure;
	
	while(!edgeIt.isDone())
	{
		MIntArray pointIndexArray;
		pointIndexArray.append(edgeIt.index(0));
		pointIndexArray.append(edgeIt.index(1));
		meshEdges.push_back(pointIndexArray);
		edgeIt.next();
	}
	_cacheMeshToEdgeArray[meshPath.fullPathName().asChar()] = meshEdges;	
	return status;
}
