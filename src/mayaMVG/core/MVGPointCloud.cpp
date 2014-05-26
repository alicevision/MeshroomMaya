#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <stdexcept>

using namespace mayaMVG;

// dynamic attributes
MString MVGPointCloud::_RGBPP = "rgbPP";

MVGPointCloud::MVGPointCloud(const std::string& name)
	: MVGNodeWrapper(name)
{
}
	
MVGPointCloud::MVGPointCloud(const MDagPath& dagPath)
	: MVGNodeWrapper(dagPath)
{
}

MVGPointCloud::~MVGPointCloud()
{
}

// virtual
bool MVGPointCloud::isValid() const
{
	return _dagpath.isValid();
}

// static
MVGPointCloud MVGPointCloud::create(const std::string& name)
{
	MStatus status;
	MFnParticleSystem fnParticle;

	// get project root node
	MVGProject project(MVGProject::_PROJECT);
	MObject rootObj = project.dagPath().node();

	// create maya particle system node
	MDagPath path;
	MObject particleSystem = fnParticle.create(&status);
	MDagPath::getAPathTo(particleSystem, path);

	// add dynamic attributes & reparent under root node
	MDagModifier dagModifier;
	MFnTypedAttribute tAttr;
	MObject rgbAttr = tAttr.create(_RGBPP, "rgb", MFnData::kVectorArray);
	dagModifier.addAttribute(path.node(), rgbAttr);
	dagModifier.reparentNode(path.transform(), rootObj);
	dagModifier.doIt();

	// rename and return
	MVGPointCloud cloud(path);
	cloud.setName(name);
	return cloud;
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
		array_position.append(it->_position);
		array_color.append(it->_color);
	}

	// emit particles
	status = fnParticle.emit(array_position);
	
	// set color
	fnParticle.setPerParticleAttribute(_RGBPP, array_color, &status);
	status = fnParticle.saveInitialState();
}

std::vector<MVGPointCloudItem> MVGPointCloud::getItems() const
{
	MStatus status;
	std::vector<MVGPointCloudItem> items;

	MFnParticleSystem fnParticle(_dagpath.node(), &status);
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
