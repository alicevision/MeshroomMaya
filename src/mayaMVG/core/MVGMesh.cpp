#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDagModifier.h>

#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>

using namespace mayaMVG;

MVGMesh::MVGMesh(const std::string& name)
	: MVGNodeWrapper(name)
{
}

MVGMesh::MVGMesh(const MDagPath& dagPath)
	: MVGNodeWrapper(dagPath)
{
}

MVGMesh::~MVGMesh()
{
}

// virtual
bool MVGMesh::isValid() const
{
	return _dagpath.isValid();
}

// static
MVGMesh MVGMesh::create(const std::string& name)
{
	MStatus status;
	MFnMesh fnMesh;

	// get project root node
	MVGProject project(MVGProject::_PROJECT);
	MObject rootObj = project.dagPath().node();

	// create empty mesh
	MPointArray vertexArray;
	MIntArray polygonCounts, polygonConnects;
	MObject transform = fnMesh.create(0, 0, vertexArray, polygonCounts, 
					polygonConnects, MObject::kNullObj, &status);
	
	// register dag path
	MDagPath path;
	MDagPath::getAPathTo(transform, path);
	path.extendToShape();

	// connect to initialShadingGroup
	MObject sgObj;
	MSelectionList list;
	status = MGlobal::getSelectionListByName("initialShadingGroup", list);
	status = list.getDependNode(0, sgObj);
	MFnSet fnSet(sgObj, &status);
 	status = fnSet.addMember(path);
	CHECK(status)

	// reparent under root node
	MDagModifier dagModifier;
	dagModifier.reparentNode(transform, rootObj);
	dagModifier.doIt();

	// rename and return
	MVGMesh mesh(path);
	mesh.setName(name);
	return mesh;
}

void MVGMesh::addPolygon(const MVGFace3D& face3d) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	MPointArray pointArray;
	pointArray.append(face3d._p[0]);
	pointArray.append(face3d._p[1]);
	pointArray.append(face3d._p[2]);
	pointArray.append(face3d._p[3]);
	fnMesh.addPolygon(pointArray, true, kMFnMeshPointTolerance, MObject::kNullObj, &status);
	CHECK(status);
}

void MVGMesh::deleteFace(const int index) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	
	status = fnMesh.deleteFace(index);
}

void MVGMesh::getPoints(MPointArray& pointArray) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	
	fnMesh.getPoints(pointArray, MSpace::kPostTransform);
	CHECK(status);
}

int MVGMesh::getVerticesCount() const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	
	int count = fnMesh.numVertices(&status);
	CHECK(status);
	
	return count;
}

bool MVGMesh::intersect(MPoint& point, MVector& dir, MPointArray&points) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	
	bool intersect = fnMesh.intersect(point, dir, points, status);
	CHECK(status);
	return intersect;
}

int MVGMesh::getNumConnectedFacesToVertex(int vertexId)
{
	MStatus status;
	MItMeshVertex verticesIter(_dagpath, MObject::kNullObj, &status);
	int prev;
	status = verticesIter.setIndex(vertexId, prev);
	int faceCount;
	status = verticesIter.numConnectedFaces(faceCount);
	
	CHECK(status);
	return faceCount;
}

int MVGMesh::getNumConnectedFacesToEdge(int edgeId)
{
	MStatus status;
	MItMeshEdge edgeIter(_dagpath, MObject::kNullObj, &status);
	int prev;
	status = edgeIter.setIndex(edgeId, prev);
	int faceCount;
	status = edgeIter.numConnectedFaces(faceCount);
	
	CHECK(status);
	return faceCount;
}

MIntArray MVGMesh::getConnectedFacesToVertex(int vertexId)
{
	MIntArray connectedFacesId;
	MStatus status;
	MItMeshVertex verticesIter(_dagpath, MObject::kNullObj, &status);
	int prev;
	status = verticesIter.setIndex(vertexId, prev);
	
	status = verticesIter.getConnectedFaces(connectedFacesId);
	
	CHECK(status);
	return connectedFacesId;
}

int MVGMesh::getConnectedFacesToEdge(MIntArray& facesId, int edgeId)
{
	MStatus status;
	MItMeshEdge edgeIter(_dagpath, MObject::kNullObj, &status);
	int prev;
	status = edgeIter.setIndex(edgeId, prev);
	
	int faceCount;
	faceCount = edgeIter.getConnectedFaces(facesId, &status);
	
	CHECK(status);
	return faceCount;
}


MIntArray MVGMesh::getFaceVertices(int faceId)
{
	MStatus status;
	MItMeshPolygon faceIter(_dagpath.node());
	int prev;
	status = faceIter.setIndex(faceId, prev);

	MIntArray vertices;
	status = faceIter.getVertices(vertices);
	
	CHECK(status);
	return vertices;
}

MIntArray MVGMesh::getEdgeVertices(int edgeId)
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	
	int2 edgeVertices;
	status = fnMesh.getEdgeVertices(edgeId, edgeVertices);
	CHECK(status);
	
	MIntArray edgeVerticesArray;
	edgeVerticesArray.append(edgeVertices[0]);
	edgeVerticesArray.append(edgeVertices[1]);
	return edgeVerticesArray;
}

void MVGMesh::setPoint(int vertexId, MPoint& point)
{
	MStatus status;
	MFnMesh fnMesh(_dagpath.node(), &status);
	fnMesh.setPoint(vertexId, point);
	CHECK(status);
}
			