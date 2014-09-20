#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGPointCloudItem.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MPlug.h>
#include <stdexcept>

namespace mayaMVG
{

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

bool MVGPointCloud::isValid() const
{
    return _dagpath.isValid();
}

MVGPointCloud MVGPointCloud::create(const std::string& name)
{
    MStatus status;
    MFnParticleSystem fnParticle;

    // get project root node
    MVGProject project(MVGProject::_PROJECT);
    MObject parent = project.getDagPath().child(1); // clouds transform

    // create maya particle system node
    MDagPath path;
    MObject particleSystem = fnParticle.create(&status);
    CHECK(status)
    MDagPath::getAPathTo(particleSystem, path);

    MFnDagNode fn(path.transform());
    fn.findPlug("translateX").setLocked(true);
    fn.findPlug("translateY").setLocked(true);
    fn.findPlug("translateZ").setLocked(true);
    fn.findPlug("rotateX").setLocked(true);
    fn.findPlug("rotateY").setLocked(true);
    fn.findPlug("rotateZ").setLocked(true);

    // add dynamic attributes & reparent
    MDagModifier dagModifier;
    MFnTypedAttribute tAttr;
    MObject rgbAttr = tAttr.create(_RGBPP, "rgb", MFnData::kVectorArray);
    dagModifier.addAttribute(path.node(), rgbAttr);
    dagModifier.reparentNode(path.transform(), parent);
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
    MFnParticleSystem fnParticle(_dagpath, &status);
    CHECK(status)

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
    CHECK(status)

    // set color
    fnParticle.setPerParticleAttribute(_RGBPP, array_color, &status);
    status = fnParticle.saveInitialState();
    CHECK(status)
}

std::vector<MVGPointCloudItem> MVGPointCloud::getItems() const
{
    MStatus status;
    std::vector<MVGPointCloudItem> items;
    MFnParticleSystem fnParticle(_dagpath, &status);
    if(!status)
        return items;
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

} // namespace
