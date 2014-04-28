#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

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

	// create empty mesh
	MPointArray vertexArray;
	MIntArray polygonCounts, polygonConnects;
	MObject transform = fnMesh.create(0, 0, vertexArray, polygonCounts, polygonConnects, MObject::kNullObj, &status);
	
	// register dag path
	MDagPath path;
	MDagPath::getAPathTo(transform, path);
	path.extendToShape();

	MVGMesh mesh(path);
	mesh.setName(name);
	return mesh;
}

void MVGMesh::addPolygon(const MVGFace3D& face3d)
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