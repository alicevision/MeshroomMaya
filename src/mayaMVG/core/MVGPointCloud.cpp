#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <stdexcept>

using namespace mayaMVG;

MVGPointCloud::MVGPointCloud(const std::string& name)
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

const std::string MVGPointCloud::name() const
{
	MFnDependencyNode depNode(_dagpath.node());
	return depNode.name().asChar();
}

void MVGPointCloud::setName(const std::string& name)
{
	MFnDependencyNode depNode(_dagpath.node());
	depNode.setName(name.c_str());
}

void MVGPointCloud::setItems(const std::vector<MVGPointCloudItem>& items)
{
	if(!_dagpath.isValid())
		return;

	MStatus status;
	MFnParticleSystem fnParticle(_dagpath.node(), &status);

	// as MVectorArray
	MPointArray array_position;
	MVectorArray array_color;
	std::vector<MVGPointCloudItem>::const_iterator it = items.begin();
	for(; it != items.end(); it++)
	{
		array_position.append(MPoint(it->_position[0], it->_position[1], it->_position[2]));
		array_color.append(MVector(it->_color[0], it->_color[1], it->_color[2]));
	}

	// emit particles
	status = fnParticle.emit(array_position);
	
	MDagModifier dagModifier;
	MFnTypedAttribute tAttr;

	MObject rgbpp = tAttr.create("rgbPP", "rgb", MFnData::kVectorArray);
	dagModifier.addAttribute(_dagpath.node(), rgbpp);
	dagModifier.doIt();

	fnParticle.setPerParticleAttribute("rgbPP", array_color, &status);

	status = fnParticle.saveInitialState();

}

std::vector<MVGPointCloudItem> MVGPointCloud::getItems() const
{
	MStatus status;
	std::vector<MVGPointCloudItem> items;

	MFnParticleSystem fnParticle(_dagpath.child(0), &status);
	if(!status) {
		LOG_INFO(status.errorString().asChar());
	}

	MVectorArray positionArray;
	fnParticle.position(positionArray);
	for(int i = 0; i < positionArray.length(); ++i)
	{
		MVGPointCloudItem item;
		item._position = positionArray[i];
		items.push_back(item);
	}
	return items;
}
