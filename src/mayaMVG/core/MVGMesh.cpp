#include "mayaMVG/core/MVGMesh.h"
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MSelectionList.h>
#include <stdexcept>

using namespace mayaMVG;

		
MVGMesh::MVGMesh(const std::string& name)
	: _name(name)
{
	if(name.empty())
		throw std::invalid_argument(name);
	MSelectionList list;
	MStatus status = list.add(name.c_str());
	if(!status)
		throw std::invalid_argument(name);
	list.getDagPath(0, _dagpath);
	if(!_dagpath.isValid())
		throw std::invalid_argument(name);
	_dagpath.pop(); // registering the transform node
}
	
MVGMesh::MVGMesh(const MDagPath& dagPath)
	: _dagpath(dagPath)
{
}

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
	MVGMesh mesh(path);
	mesh.setName(name);
	return mesh;
}

MVGMesh::~MVGMesh()
{
}

const std::string& MVGMesh::name() const
{
	return _name;
}

void MVGMesh::setName(const std::string& name)
{
	_name = name;

	// set maya camera name
	MFnDependencyNode depNode(_dagpath.node());
	depNode.setName(_name.c_str());
}

void MVGMesh::add3DPoint(const MPoint&)
{
}

void MVGMesh::move3DPoint(const MPoint&)
{
}
