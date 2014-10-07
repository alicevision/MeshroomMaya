#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/core/MVGPointCloudItem.hpp"
#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGPlaneKernel.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MPlug.h>
#include <maya/MMatrix.h>
#include <stdexcept>

namespace mayaMVG
{

// dynamic attributes
MString MVGPointCloud::_RGBPP = "rgbPP";

namespace
{ // empty namespace

int isLeft(MPoint P0, MPoint P1, MPoint P2)
{
    // isLeft(): tests if a point is Left|On|Right of an infinite line.
    //    Input:  three points P0, P1, and P2
    //    Return: >0 for P2 left of the line through P0 and P1
    //            =0 for P2  on the line
    //            <0 for P2  right of the line
    //    See: Algorithm 1 "Area of Triangles and Polygons"
    return ((P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y));
}

int wn_PnPoly(MPoint P, MPointArray V)
{
    // wn_PnPoly(): winding number test for a point in a polygon
    //      Input:   P = a point,
    //               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
    //      Return:  wn = the winding number (=0 only when P is outside)
    int wn = 0;
    for(int i = 0; i < V.length() - 1; i++)
    {
        if(V[i].y <= P.y)
        {
            if(V[i + 1].y > P.y)
                if(isLeft(V[i], V[i + 1], P) > 0)
                    ++wn;
        }
        else
        {
            if(V[i + 1].y <= P.y)
                if(isLeft(V[i], V[i + 1], P) < 0)
                    --wn;
        }
    }
    return wn;
}

} // empty namespace

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

bool MVGPointCloud::projectPoints(M3dView& view, const MPointArray& cameraSpacePoints,
                                  MPointArray& worldSpacePoints)
{
    if(!isValid())
        return false;

    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);
    if(!camera.isValid())
        return false;

    if(cameraSpacePoints.length() < 3)
        return false;

    std::vector<MVGPointCloudItem> items = camera.getVisibleItems();
    if(items.size() < 3)
        return false;

    MPointArray closedVSPolygon(MVGGeometryUtil::cameraToViewSpace(view, cameraSpacePoints));
    closedVSPolygon.append(closedVSPolygon[0]); // add an extra point (to describe a closed shape)

    // get enclosed items in pointcloud
    MPointArray enclosedWSPoints;
    std::vector<MVGPointCloudItem>::const_iterator it = items.begin();
    int windingNumber = 0;
    for(; it != items.end(); ++it)
    {
        windingNumber =
            wn_PnPoly(MVGGeometryUtil::worldToViewSpace(view, it->_position), closedVSPolygon);
        if(windingNumber != 0)
            enclosedWSPoints.append(it->_position);
    }
    if(enclosedWSPoints.length() < 3)
        return false;

    return MVGGeometryUtil::projectPointsOnPlane(view, cameraSpacePoints, enclosedWSPoints,
                                                 worldSpacePoints);
}

} // namespace
