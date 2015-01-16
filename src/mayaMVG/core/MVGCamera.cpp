#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/core/MVGPointCloudItem.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
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
MString MVGCamera::_ID = "cameraId";
MString MVGCamera::_PINHOLE = "pinholeProjectionMatrix";
MString MVGCamera::_ITEMS = "visibleItems";
MString MVGCamera::_DEFERRED = "deferredLoading";

MVGCamera::MVGCamera()
    : MVGNodeWrapper()
{
}

MVGCamera::MVGCamera(const std::string& name)
    : MVGNodeWrapper(name)
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
    fn.findPlug(_ID, false, &status);
    if(!status)
        return false;
    fn.findPlug(_PINHOLE, false, &status);
    if(!status)
        return false;
    fn.findPlug(_ITEMS, false, &status);
    if(!status)
        return false;
    return true;
}

MVGCamera MVGCamera::create(const std::string& name)
{
    MStatus status;
    MFnCamera fnCamera;
    MDagPath path;

    // get project root node
    MVGProject project(MVGProject::_PROJECT);
    MObject parent = project.getDagPath().child(0); // cameras transform

    // create maya camera
    MObject transform = fnCamera.create(parent, &status);
    CHECK(status)

    // initialize MVGCamera from maya node
    MDagPath::getAPathTo(transform, path);
    path.extendToShape();
    MVGCamera camera(path);
    camera.setName(name);

    // add MayaMVG attributes
    MDagModifier dagModifier;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MObject idAttr = nAttr.create(_ID, "id", MFnNumericData::kInt);
    dagModifier.addAttribute(path.node(), idAttr);
    MObject pinholeAttr = tAttr.create(_PINHOLE, "ppm", MFnData::kDoubleArray);
    dagModifier.addAttribute(path.node(), pinholeAttr);
    MObject itemsAttr = tAttr.create(_ITEMS, "itm", MFnData::kIntArray);
    dagModifier.addAttribute(path.node(), itemsAttr);
    dagModifier.doIt();

    // create, reparent & connect image plane
    MString cmd;
    MGlobal::executePythonCommand("from mayaMVG import camera");
    MString fileName = "";
    cmd.format("camera.mvgSetImagePlane(\'^1s\', \'^2s\')", path.fullPathName(), fileName);
    MGlobal::executePythonCommand(cmd);

    return camera;
}

std::vector<MVGCamera> MVGCamera::getCameras()
{
    std::vector<MVGCamera> list;
    MDagPath path;
    MItDependencyNodes it(MFn::kCamera);
    for(; !it.isDone(); it.next())
    {
        MFnDependencyNode fn(it.thisNode());
        MDagPath::getAPathTo(fn.object(), path);
        MVGCamera camera(path);
        if(camera.isValid())
            list.push_back(camera);
    }
    return list;
}

int MVGCamera::getId() const
{
    int id = -1;
    MStatus status;
    status = MVGMayaUtil::getIntAttribute(_dagpath.node(), _ID, id);
    CHECK(status)
    return id;
}

void MVGCamera::setId(const int& id) const
{
    MStatus status;
    status = MVGMayaUtil::setIntAttribute(_dagpath.node(), _ID, id);
    CHECK(status)
}

MDagPath MVGCamera::getImagePath() const
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
        LOG_ERROR("No plug connected to the plug")
        return path;
    }
    status = MDagPath::getAPathTo(connectedPlugs[0].node(), path);
    CHECK(status)
    return path;
}

std::string MVGCamera::getImagePlane() const
{
    MStatus status;
    MFnDagNode fnImage(getImagePath(), &status);
    CHECK(status)
    std::string imageName(fnImage.findPlug(_DEFERRED).asString().asChar());
    return imageName;
}

void MVGCamera::setImagePlane(const std::string& img, int width, int height) const
{
    if(img.empty())
        return;

    // image plane parameters
    MStatus status;
    MDagPath imagePath = getImagePath();
    MFnDagNode fnImage(imagePath, &status);
    CHECK_RETURN(status)
    fnImage.findPlug("depth").setValue(50);
    fnImage.findPlug("dic").setValue(1);
    fnImage.findPlug("fit").setValue(2);
    fnImage.findPlug("width").setValue(width);
    fnImage.findPlug("height").setValue(height);
    fnImage.findPlug("displayOnlyIfCurrent").setValue(1);

    openMVG::PinholeCamera pinhole = getPinholeCamera();
    double offsetX = width * 0.5 - pinhole._K(0, 2);
    double offsetY = height * 0.5 - pinhole._K(1, 2);
    double hAperture = getHorizontalFilmAperture();
    fnImage.findPlug("offsetX").setValue(offsetX / width * hAperture);
    fnImage.findPlug("offsetY").setValue(-offsetY / width * hAperture);

    // handling deferred loading
    if(fnImage.findPlug(_DEFERRED).isNull())
    {
        MDagModifier dagModifier;
        MFnTypedAttribute tAttr;
        MObject dynAttr = tAttr.create(_DEFERRED, "def", MFnData::kString);
        dagModifier.addAttribute(imagePath.node(), dynAttr);
        dagModifier.doIt();
        fnImage.findPlug(_DEFERRED).setValue(img.c_str());
    }
    else
    {
        fnImage.findPlug(_DEFERRED).setValue(img.c_str());
        fnImage.findPlug("imageName").setValue(img.c_str());
    }
}

void MVGCamera::loadImagePlane() const
{
    MStatus status;
    MFnDagNode fnImage(getImagePath());
    MString deferred = fnImage.findPlug(_DEFERRED, &status).asString();
    CHECK_RETURN(status)
    MPlug imageNamePlug = fnImage.findPlug("imageName", &status);
    CHECK_RETURN(status)
    MString name = imageNamePlug.asString();
    if(name != deferred)
    {
        status = imageNamePlug.setValue(deferred);
        CHECK_RETURN(status)
    }
}

void MVGCamera::unloadImagePlane() const
{
    MStatus status;
    MFnDagNode fnImage(getImagePath(), &status);
    CHECK_RETURN(status)
    MPlug imageNamePlug = fnImage.findPlug("imageName", &status);
    CHECK_RETURN(status)
    MString name = imageNamePlug.asString();
    if(name.length() == 0)
        return;
    status = imageNamePlug.setValue("");
    CHECK_RETURN(status)
}

openMVG::PinholeCamera MVGCamera::getPinholeCamera() const
{
    // Retrieve pinhole from 'pinholeProjectionMatrix' attribute
    MDoubleArray doubleArray;
    MVGMayaUtil::getDoubleArrayAttribute(_dagpath.node(), _PINHOLE, doubleArray);
    if(doubleArray.length() < 12)
    {
        LOG_ERROR("Unable to read " << _dagpath.fullPathName().asChar()
                                    << "::pinholeProjectionMatrix attribute")
        return openMVG::PinholeCamera();
    }
    openMVG::Mat34 P;
    for(size_t i = 0; i < 3; ++i)
        for(size_t j = 0; j < 4; ++j)
            P(i, j) = doubleArray[4 * i + j];
    return openMVG::PinholeCamera(P);
}

void MVGCamera::setPinholeCamera(const openMVG::PinholeCamera& pinhole) const
{
    // set maya camera position
    MFnTransform fnTransform(_dagpath.transform());
    fnTransform.setTranslation(MVector(pinhole._C(0), pinhole._C(1), pinhole._C(2)),
                               MSpace::kTransform);

    // set maya camera orientation
    MMatrix m = MMatrix::identity;
    m[0][0] = pinhole._R(0, 0);
    m[0][1] = pinhole._R(0, 1);
    m[0][2] = pinhole._R(0, 2);
    m[1][0] = pinhole._R(1, 0) * -1; // FIXME
    m[1][1] = pinhole._R(1, 1) * -1; // ?
    m[1][2] = pinhole._R(1, 2) * -1; // ?
    m[2][0] = pinhole._R(2, 0) * -1; // ?
    m[2][1] = pinhole._R(2, 1) * -1; // ?
    m[2][2] = pinhole._R(2, 2) * -1; // ?
    MQuaternion quaternion;
    quaternion = m;
    fnTransform.setRotation(quaternion, MSpace::kTransform);

    // set maya camera intrinsic parameters
    size_t focal = pinhole._K(0, 0);
    size_t width = pinhole._K(0, 2) * 2;
    size_t height = pinhole._K(1, 2) * 2;
    MFnCamera fnCamera(_dagpath);
    fnCamera.setVerticalFilmAperture(fnCamera.horizontalFilmAperture() *
                                     ((double)height / (double)width));
    fnCamera.setHorizontalFieldOfView((2.0 * atan((double)width / (2.0 * (double)focal))));
    fnCamera.setPanZoomEnabled(true);
    fnCamera.setFilmFit(MFnCamera::kHorizontalFilmFit);

    // lock maya camera transform attributes
    fnTransform.findPlug("translateX").setLocked(true);
    fnTransform.findPlug("translateY").setLocked(true);
    fnTransform.findPlug("translateZ").setLocked(true);
    fnTransform.findPlug("rotateX").setLocked(true);
    fnTransform.findPlug("rotateY").setLocked(true);
    fnTransform.findPlug("rotateZ").setLocked(true);

    // set pinhole attribute P (projection matrix P = K[R|t])
    MDoubleArray doubleArray;
    for(size_t i = 0; i < 3; ++i)
        for(size_t j = 0; j < 4; ++j)
            doubleArray.append(pinhole._P(i, j));
    MVGMayaUtil::setDoubleArrayAttribute(_dagpath.node(), _PINHOLE, doubleArray);
}

std::vector<MVGPointCloudItem> MVGCamera::getVisibleItems() const
{
    std::vector<MVGPointCloudItem> allItems, items;
    MVGPointCloud pointCloud(MVGProject::_CLOUD);
    allItems = pointCloud.getItems();
    MIntArray intArray;
    MVGMayaUtil::getIntArrayAttribute(_dagpath.node(), _ITEMS, intArray);
    if(intArray.length() > 0)
    {
        for(size_t i = 0; i < intArray.length(); ++i)
        {
            int index = intArray[i];
            if(index < allItems.size())
                items.push_back(allItems[index]);
        }
        return items;
    }
    LOG_WARNING("No visibility information in pointCloud.");
    return allItems;
}

void MVGCamera::setVisibleItems(const std::vector<MVGPointCloudItem>& items) const
{
    MIntArray intArray;
    intArray.setLength(items.size());
    for(size_t i = 0; i < items.size(); ++i)
        intArray.set(items[i]._id, i);
    MVGMayaUtil::setIntArrayAttribute(_dagpath.node(), _ITEMS, intArray);
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
    MFnDagNode fnImage(getImagePath(), &status);
    CHECK(status)
    size.first = fnImage.findPlug("coverageX").asDouble();
    size.second = fnImage.findPlug("coverageY").asDouble();
    return size;
}

} // namespace
