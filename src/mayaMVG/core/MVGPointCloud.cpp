#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MSelectionList.h>
#include <stdexcept>

using namespace mayaMVG;

MVGPointCloud::MVGPointCloud(const std::string& name)
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
	
MVGPointCloud::MVGPointCloud(const MDagPath& dagPath)
	: _dagpath(dagPath)
{
}

MVGPointCloud MVGPointCloud::create(const std::string& name)
{
	MStatus status;
	MFnParticleSystem fnParticle;
	MObject transform = fnParticle.create(&status);
	
	// register dag path
	MDagPath path;
	MDagPath::getAPathTo(transform, path);
	MVGPointCloud cloud(path);
	cloud.setName(name);
	return cloud;
}

MVGPointCloud::~MVGPointCloud()
{
}

const std::string& MVGPointCloud::name() const
{
	return _name;
}

void MVGPointCloud::setName(const std::string& name)
{
	_name = name;

	// set maya camera name
	MFnDependencyNode depNode(_dagpath.node());
	depNode.setName(_name.c_str());
}

void MVGPointCloud::setItems(const std::vector<MVGPointCloudItem>& items)
{
	_items = items;

	if(!_dagpath.isValid())
		return;

	MStatus status;
	MFnParticleSystem fnParticle(_dagpath.node(), &status);
	MVectorArray array_position;
	MVectorArray array_color;
	// as vectorArray
	std::vector<MVGPointCloudItem>::const_iterator it = _items.begin();
	for(; it != _items.end(); it++)
	{
		array_position.append(MVector(it->_position[0], it->_position[1], it->_position[2]));
		array_color.append(MVector(it->_color[0], it->_color[1], it->_color[2]));
	}

	// set particle attributes
	fnParticle.setPerParticleAttribute(MString("position"), array_position, &status);
	fnParticle.setPerParticleAttribute(MString("rgbPP"), array_color, &status);
	status = fnParticle.saveInitialState();
}

void MVGPointCloud::getItemsFromProjection(std::vector<MVGPointCloudItem>& items, MVGCamera& camera, MVGFace2D& face2D) const
{
	// items.push_back();
}
