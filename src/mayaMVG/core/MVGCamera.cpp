#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/core/MVGPointCloudItem.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/maya/cmd/MVGImagePlaneCmd.hpp"
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MDagModifier.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MItDependencyNodes.h>

namespace mayaMVG
{

// dynamic attributes
MString MVGCamera::_MVG_ITEMS = "mvg_visibleItems";

MString MVGCamera::_MVG_INTRINSIC_ID = "mvg_intrinsicId";
MString MVGCamera::_MVG_VIEW_ID = "mvg_viewId";
MString MVGCamera::_MVG_INTRINSIC_TYPE = "mvg_intrinsicType";
MString MVGCamera::_MVG_INTRINSICS_PARAMS = "mvg_intrinsicParams";
MString MVGCamera::_MVG_IMAGE_PATH = "mvg_imagePath";
MString MVGCamera::_MVG_THUMBNAIL_PATH = "mvg_thumbnailPath";
MString MVGCamera::_MVG_SENSOR_WIDTH = "mvg_sensorWidth_pix";

MVGCamera::MVGCamera()
    : MVGNodeWrapper()
{
}

MVGCamera::MVGCamera(const std::string& dagPathAsString)
    : MVGNodeWrapper(dagPathAsString)
{
}

MVGCamera::MVGCamera(const MDagPath& dagPath)
    : MVGNodeWrapper(dagPath)
{
}

MVGCamera::MVGCamera(const int& id)
    : MVGNodeWrapper()
{
    MStatus status;
    MDagPath path;
    MItDependencyNodes it(MFn::kCamera);
    for(; !it.isDone(); it.next())
    {
        MFnDependencyNode fn(it.thisNode());
        status = MDagPath::getAPathTo(fn.object(), path);
        CHECK_RETURN(status)
        MVGCamera camera(path);
        if(camera.isValid() && (camera.getId() == id))
        {
            status = MDagPath::getAPathTo(fn.object(), _dagpath);
            return;
        }
    }
    LOG_ERROR("Unable to find camera with id " << id)
}

MVGCamera::~MVGCamera()
{
}

bool MVGCamera::operator<(const MVGCamera& src) const
{
    return (getId() < src.getId());
}

bool MVGCamera::isValid() const
{
    if(!_dagpath.isValid() || (_dagpath.apiType() != MFn::kCamera))
        return false;
    MFnCamera fn(_dagpath);
    MStatus status;
    fn.findPlug(_MVG_INTRINSIC_ID, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_VIEW_ID, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_INTRINSIC_TYPE, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_INTRINSICS_PARAMS, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_IMAGE_PATH, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_SENSOR_WIDTH, false, &status);
    if(!status)
        return false;
    fn.findPlug(_MVG_ITEMS, false, &status);
    if(!status)
        return false;
    return true;
}

MVGCamera MVGCamera::create(MDagPath& cameraDagPath, std::map<int, MIntArray>& itemsPerCamera)
{
    MStatus status;

    cameraDagPath.extendToShape();
    MObject cameraNode = cameraDagPath.node();

    MVGCamera camera(cameraDagPath);
    //    camera.setName(name);

    // Configure camera
    MFnCamera fnCamera(cameraDagPath);
    fnCamera.setPanZoomEnabled(true);

    // Add MVG attributes
    MDagModifier dagModifier;
    MFnTypedAttribute tAttr;
    // Visibility
    int viewID;
    MVGMayaUtil::getIntAttribute(cameraNode, "mvg_viewId", viewID);
    MObject itemsAttr = tAttr.create(MVGCamera::_MVG_ITEMS, "itm", MFnData::kIntArray);
    dagModifier.addAttribute(cameraNode, itemsAttr);
    MObject thumbnailAttr = tAttr.create(MVGCamera::_MVG_THUMBNAIL_PATH, "mtp", MFnData::kString);
    dagModifier.addAttribute(cameraNode, thumbnailAttr);
    dagModifier.doIt();

    // Set MVG attributes
    MVGMayaUtil::setIntArrayAttribute(cameraNode, MVGCamera::_MVG_ITEMS, itemsPerCamera[viewID]);

    // create, reparent & connect image plane
    MString cmd;
    MGlobal::executePythonCommand("from mayaMVG import camera");
    MString fileName = "";
    cmd.format("camera.mvgSetImagePlane(\'^1s\', \'^2s\')", cameraDagPath.fullPathName(), fileName);
    MGlobal::executePythonCommand(cmd);

    // Configure image plane
    camera.setImagePlane();
    return camera;
}

/**
 * Retrieve valid MVGCamera under mayaMVG 'cameras' node
 * @return
 */
std::vector<MVGCamera> MVGCamera::getCameras()
{
    MStatus status;
    std::vector<MVGCamera> list;
    // Retrieve mayaMVG camera node
    MDagPath cameraDagPath;
    status = MVGMayaUtil::getDagPathByName(MVGProject::_CAMERAS_GROUP.c_str(), cameraDagPath);
    CHECK(status);
    MFnDagNode cameraDagNode(cameraDagPath);
    for(int i = 0; i < cameraDagNode.childCount(); ++i)
    {
        // Retrieve transform node
        MObject cameraObject = cameraDagNode.child(i);
        MDagPath cameraPath;
        status = MDagPath::getAPathTo(cameraObject, cameraPath);
        CHECK(status);
        cameraPath.extendToShape();
        MVGCamera camera(cameraPath);
        if(camera.isValid())
            list.push_back(camera);
    }
    return list;
}

int MVGCamera::getId() const
{
    int id = -1;
    MStatus status;
    status = MVGMayaUtil::getIntAttribute(_dagpath.node(), _MVG_VIEW_ID, id);
    CHECK(status)
    return id;
}

void MVGCamera::setId(const int& id) const
{
    MStatus status;
    status = MVGMayaUtil::setIntAttribute(_dagpath.node(), _MVG_VIEW_ID, id);
    CHECK(status)
}

MDagPath MVGCamera::getImagePlaneShapeDagPath() const
{

    MStatus status;
    MDagPath path;
    MFnDagNode fn(_dagpath, &status);
    MPlug imagePlanePlug = fn.findPlug("imagePlane", status);
    CHECK(status)
    MPlug imagePlug = imagePlanePlug.elementByLogicalIndex(0, &status);
    MPlugArray connectedPlugs;
    imagePlug.connectedTo(connectedPlugs, true, true, &status);
    CHECK(status)
    if(connectedPlugs.length() == 0)
    {
        LOG_ERROR("No plug connected to the plug for camera : " << getName())
        return path;
    }
    status = MDagPath::getAPathTo(connectedPlugs[0].node(), path);
    CHECK(status)
    return path;
}

std::string MVGCamera::getThumbnailPath() const
{
    MStatus status;
    MFnDagNode fn(_dagpath, &status);
    std::string imageName(fn.findPlug(MVGCamera::_MVG_THUMBNAIL_PATH).asString().asChar());

    return imageName;
}

void MVGCamera::setImagePlane() const
{
    MStatus status;
    // Configure image plane
    MDagPath imagePath = getImagePlaneShapeDagPath();

    MFnDagNode fnImage(imagePath, &status);
    CHECK_RETURN(status)
    fnImage.findPlug("depth").setValue(50);
    fnImage.findPlug("dic").setValue(1);
    fnImage.findPlug("fit").setValue(2);
    const std::pair<double, double> imageSize = getImageSize();
    fnImage.findPlug("width").setValue(imageSize.first);
    fnImage.findPlug("height").setValue(imageSize.second);
    fnImage.findPlug("displayOnlyIfCurrent").setValue(1);
}

void MVGCamera::unloadImagePlane() const
{
    MStatus status;
    MFnDagNode fnImage(getImagePlaneShapeDagPath(), &status);
    CHECK_RETURN(status)
    MPlug imageNamePlug = fnImage.findPlug("imageName", &status);
    CHECK_RETURN(status)
    MString name = imageNamePlug.asString();
    if(name.length() == 0)
        return;
    status = imageNamePlug.setValue("");
    CHECK_RETURN(status)
}

MPoint MVGCamera::getCenter(MSpace::Space space) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    return fnCamera.eyePoint(space);
}

double MVGCamera::getSensorWidth() const
{
    MStatus status;
    double sensorWidth;
    status = MVGMayaUtil::getDoubleAttribute(_dagpath.node(), _MVG_SENSOR_WIDTH, sensorWidth);
    CHECK(status)
    return sensorWidth;
}

void MVGCamera::setSensorWidth(const double sensorWidth) const
{
    MStatus status;
    status = MVGMayaUtil::setDoubleAttribute(_dagpath.node(), _MVG_SENSOR_WIDTH, sensorWidth);
    CHECK_RETURN(status)
}

void MVGCamera::getVisibleItems(std::vector<MVGPointCloudItem>& visibleItems) const
{
    MStatus status;
    MIntArray visibleIndexes;
    status = MVGMayaUtil::getIntArrayAttribute(_dagpath.node(), _MVG_ITEMS, visibleIndexes);
    CHECK_RETURN(status)
    MVGPointCloud pointCloud(MVGProject::_CLOUD);
    pointCloud.getItems(visibleItems, visibleIndexes);
}

void MVGCamera::setVisibleItems(const std::vector<MVGPointCloudItem>& items) const
{
    MIntArray intArray;
    intArray.setLength(items.size());
    for(size_t i = 0; i < items.size(); ++i)
        intArray.set(items[i]._id, i);
    MVGMayaUtil::setIntArrayAttribute(_dagpath.node(), _MVG_ITEMS, intArray);
}

double MVGCamera::getZoom() const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    return fnCamera.zoom();
}

void MVGCamera::setZoom(const double zoom) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setZoom(zoom);
}

double MVGCamera::getHorizontalPan() const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    return fnCamera.horizontalPan();
}

void MVGCamera::setHorizontalPan(const double pan) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setHorizontalPan(pan);
}

double MVGCamera::getVerticalPan() const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    return fnCamera.verticalPan();
}

void MVGCamera::setVerticalPan(const double pan) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setVerticalPan(pan);
}

void MVGCamera::setPan(const double hpan, const double vpan) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setHorizontalPan(hpan);
    fnCamera.setVerticalPan(vpan);
}

void MVGCamera::setAspectRatio(const double ratio) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setAspectRatio(ratio);
}

double MVGCamera::getHorizontalFilmAperture() const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    return fnCamera.horizontalFilmAperture();
}

void MVGCamera::resetZoomAndPan() const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK(status)
    fnCamera.setZoom(1.f);
    fnCamera.setHorizontalPan(0.f);
    fnCamera.setVerticalPan(0.f);
}

void MVGCamera::setInView(const std::string& viewName) const
{
    MStatus status;
    status = MVGMayaUtil::setCameraInView(*this, viewName.c_str());
    CHECK_RETURN(status)
}

void MVGCamera::setNear(const double near) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK_RETURN(status)
    status = fnCamera.setNearClippingPlane(near);
    CHECK_RETURN(status)
}

void MVGCamera::setFar(const double far) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK_RETURN(status)
    status = fnCamera.setFarClippingPlane(far);
    CHECK_RETURN(status)
}

void MVGCamera::setLocatorScale(const double scale) const
{
    MStatus status;
    MFnCamera fnCamera(getDagPath(), &status);
    CHECK_RETURN(status)
    status = fnCamera.findPlug("locatorScale").setValue(scale);
    CHECK_RETURN(status)
}

const std::pair<double, double> MVGCamera::getImageSize() const
{
    std::pair<double, double> size;
    MStatus status;
    MFnDagNode fnImage(getImagePlaneShapeDagPath(), &status);
    CHECK(status)
    size.first = fnImage.findPlug("width").asDouble();
    size.second = fnImage.findPlug("height").asDouble();
    return size;
}

} // namespace
