#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>

namespace mayaMVG {

MVGMesh::MVGMesh(const std::string& name)
	: MVGNodeWrapper(name)
{
}

MVGMesh::MVGMesh(const MString& name)
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
	if(!_dagpath.isValid() || (_dagpath.apiType()!=MFn::kMesh))
		return false;
	MFnMesh fn(_dagpath);
	return !fn.isIntermediateObject();
}

// static
MVGMesh MVGMesh::create(const std::string& name)
{
	MStatus status;
	MFnMesh fnMesh;

	// get project root node
	MVGProject project(MVGProject::_PROJECT);
	MObject rootObj = project.getDagPath().child(2); // meshes transform

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

// static
std::vector<MVGMesh> MVGMesh::list()
{
	std::vector<MVGMesh> list;
	MDagPath path;
	MItDependencyNodes it(MFn::kMesh);
	for (; !it.isDone(); it.next()) {
		MFnDependencyNode fn(it.thisNode());
		MDagPath::getAPathTo(fn.object(), path);
		MVGMesh mesh(path);
		if(mesh.isValid())
			list.push_back(mesh);
	}
	return list;
}

bool MVGMesh::addPolygon(const MPointArray& pointArray, int& index) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	if(pointArray.length() < 3)
		return false;

	fnMesh.addPolygon(pointArray, index, true, 0.01, MObject::kNullObj, &status);
	CHECK_RETURN_VARIABLE(status, false)
	fnMesh.updateSurface();

	return true;
}

bool MVGMesh::deletePolygon(const int index) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	CHECK_RETURN_VARIABLE(status, false)
	status = fnMesh.deleteFace(index);
	CHECK_RETURN_VARIABLE(status, false)
	fnMesh.updateSurface();
	return true;
}

MStatus MVGMesh::getPoints(MPointArray& pointArray) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	CHECK_RETURN_STATUS(status);
	status = fnMesh.getPoints(pointArray, MSpace::kWorld);
	CHECK_RETURN_STATUS(status)
	return status;
}

int MVGMesh::getPolygonsCount() const
{
    MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	CHECK(status);
    int count = fnMesh.numPolygons(&status);
    CHECK(status);
	return count;
}

MStatus MVGMesh::getPolygonVertices(const int polygonId, MIntArray& vertexList) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	CHECK_RETURN_STATUS(status);
	status = fnMesh.getPolygonVertices(polygonId, vertexList);
	CHECK_RETURN_STATUS(status)
	return status;
}

const MIntArray MVGMesh::getConnectedFacesToVertex(int vertexId) const
{
	MIntArray connectedFacesId;
	MStatus status;
	MItMeshVertex verticesIter(_dagpath, MObject::kNullObj, &status);
	CHECK(status);
	int prev;
	status = verticesIter.setIndex(vertexId, prev);
	CHECK(status);
	status = verticesIter.getConnectedFaces(connectedFacesId);
	CHECK(status);
	return connectedFacesId;
}

const MIntArray MVGMesh::getFaceVertices(int faceId) const
{
	MStatus status;
	MItMeshPolygon faceIter(_dagpath);
	int prev;
	status = faceIter.setIndex(faceId, prev);
	CHECK(status);
	MIntArray vertices;
	status = faceIter.getVertices(vertices);
	CHECK(status);
	return vertices;
}

MStatus MVGMesh::setPoint(int vertexId, MPoint& point) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	CHECK_RETURN_STATUS(status);

	// FIXME - vertex merge issue
	//       - don't use the MFnMesh::setPoint
	
	MIntArray polygonCounts;
	MIntArray polygonConnects;
	status = fnMesh.getVertices(polygonCounts, polygonConnects);
	MFloatPointArray meshVertices;
	fnMesh.getPoints(meshVertices, MSpace::kObject);
	if(vertexId >= meshVertices.length())
		return MS::kFailure;
	meshVertices[vertexId] = MFloatPoint(point.x, point.y, point.z);
	status = fnMesh.createInPlace(fnMesh.numVertices(), fnMesh.numPolygons(), meshVertices, polygonCounts, polygonConnects);
	CHECK(status)
	fnMesh.updateSurface();

	CHECK_RETURN_STATUS(status)
	return status;
}

MStatus MVGMesh::getPoint(int vertexId, MPoint& point) const
{
	MStatus status;
	MFnMesh fnMesh(_dagpath, &status);
	status = fnMesh.getPoint(vertexId, point, MSpace::kWorld);
	CHECK_RETURN_STATUS(status)
	return status;
}

} // namespace
