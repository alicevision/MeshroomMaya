#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MPoint.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MDagModifier.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyNodes.h>


using namespace mayaMVG;

// dynamic attributes
MString MVGCamera::_ID = "cameraId";
MString MVGCamera::_PINHOLE = "pinholeProjectionMatrix";
MString MVGCamera::_ITEMS = "visibleItems";
MString MVGCamera::_DEFERRED = "deferredLoading";

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
	for (; !it.isDone(); it.next()) {
		MFnDependencyNode fn(it.thisNode());
		MDagPath::getAPathTo(fn.object(), path);
		MVGCamera camera(path);
		if(camera.isValid() && (camera.id()==id)) {
			MDagPath::getAPathTo(fn.object(), _dagpath);
			return;
		}
	}
	LOG_ERROR("Unable to find camera with id " << id)
}

MVGCamera::~MVGCamera()
{
}

bool MVGCamera::operator< (const MVGCamera& src) const 
{
	return (id() < src.id());
}

// virtual
bool MVGCamera::isValid() const
{
	if(!_dagpath.isValid() || (_dagpath.apiType()!=MFn::kCamera))
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

// static
MVGCamera MVGCamera::create(const std::string& name)
{
	MStatus status;
	MFnCamera fnCamera;
	MDagPath path, imgpath;

	// create maya camera
	MObject transform = fnCamera.create(&status);
	CHECK(status)

	MDagPath::getAPathTo(transform, path);
	path.extendToShape();
	
	// initialize MVGCamera from maya node
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
	MObject imagePlane = dagModifier.createNode("imagePlane", transform, &status);
	dagModifier.doIt();
	MFnDependencyNode fnImage(imagePlane);
	status = dagModifier.connect(fnImage.findPlug("message"), fnCamera.findPlug("imagePlane"));
	CHECK(status)
	dagModifier.doIt();

	return camera;
}

// static
std::vector<MVGCamera> MVGCamera::list()
{
	std::vector<MVGCamera> list;
	MStatus status;
	MDagPath path;
	MItDependencyNodes it(MFn::kCamera);
	for (; !it.isDone(); it.next()) {
		MFnDependencyNode fn(it.thisNode());
		MDagPath::getAPathTo(fn.object(), path);
		MVGCamera camera(path);
		if(camera.isValid())
			list.push_back(camera);
	}
	return list;
}

int MVGCamera::id() const
{
	int id = -1;
	MVGMayaUtil::getIntAttribute(_dagpath.node(), _ID, id);
	return id;
}

void MVGCamera::setId(const int& id) const
{
	MVGMayaUtil::setIntAttribute(_dagpath.node(), _ID, id);
}

MDagPath MVGCamera::imagePath() const
{
	// FIXME, use node connections
	MDagPath path; 
	MFnDagNode fn(_dagpath.transform());
	MDagPath::getAPathTo(fn.child(1), path);
	return path;
}

std::string MVGCamera::imagePlane() const
{
	MFnDagNode fnImage(imagePath());
	return fnImage.findPlug(_DEFERRED).asString().asChar();
}

void MVGCamera::setImagePlane(const std::string& img) const
{
	if(img.empty())
		return;

	// image plane parameters
	MFnDagNode fnImage(imagePath());
	fnImage.findPlug("depth").setValue(50);
	fnImage.findPlug("dic").setValue(1);
	fnImage.findPlug("displayOnlyIfCurrent").setValue(1);
	fnImage.findPlug("fit").setValue(2);
	// fnImage.findPlug("width").setValue((int)width);
	// fnImage.findPlug("height").setValue((int)height);
	// fnImage.findPlug("verticalFilmAperture").setValue(defVFA);

	// handling deferred loading
	if(fnImage.findPlug(_DEFERRED).isNull()){
		MDagModifier dagModifier;
		MFnTypedAttribute tAttr;
		MObject dynAttr = tAttr.create(_DEFERRED, "def", MFnData::kString);
		dagModifier.addAttribute(imagePath().node(), dynAttr);
		dagModifier.doIt();
		fnImage.findPlug(_DEFERRED).setValue(img.c_str());
	} else {
		fnImage.findPlug(_DEFERRED).setValue(img.c_str());
		fnImage.findPlug("imageName").setValue(img.c_str());
	}
}

void MVGCamera::loadImagePlane() const
{
	MFnDagNode fnImage(imagePath());
	MString img = fnImage.findPlug(_DEFERRED).asString();
	fnImage.findPlug("imageName").setValue(img);
}

openMVG::PinholeCamera MVGCamera::pinholeCamera() const
{
	// Retrieve pinhole from 'pinholeProjectionMatrix' attribute
	MDoubleArray doubleArray;
	MVGMayaUtil::getDoubleArrayAttribute(_dagpath.node(), _PINHOLE, doubleArray);
	if(doubleArray.length() < 12) {
		LOG_ERROR("Unable to read a valid pinholeProjectionMatrix attribute (camera: " << name() << ")")
		return openMVG::PinholeCamera();
	}
	openMVG::Mat34 P;
	for(size_t i=0; i<3; ++i)
		for(size_t j=0; j<4; ++j)
			P(i, j) = doubleArray[4*i+j];
	return openMVG::PinholeCamera(P);
}

void MVGCamera::setPinholeCamera(const openMVG::PinholeCamera& pinhole) const
{
	// set maya camera position
	MFnTransform fnTransform(_dagpath.transform());
	fnTransform.setTranslation(MVector(pinhole._C(0), pinhole._C(1), pinhole._C(2)), MSpace::kTransform);

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
	fnCamera.setVerticalFilmAperture(fnCamera.horizontalFilmAperture() * ((double)height / (double)width));
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
	for(size_t i=0; i<3; ++i)
		for(size_t j=0; j<4; ++j)
			doubleArray.append(pinhole._P(i, j));
	MVGMayaUtil::setDoubleArrayAttribute(_dagpath.node(), _PINHOLE, doubleArray);
}

std::vector<MVGPointCloudItem> MVGCamera::visibleItems() const
{
	std::vector<MVGPointCloudItem> allItems, items;
	MVGPointCloud pointCloud(MVGScene::_CLOUD);
	allItems = pointCloud.getItems();

	MIntArray intArray;
	MVGMayaUtil::getIntArrayAttribute(_dagpath.node(), _ITEMS, intArray);
	for(size_t i = 0; i < intArray.length(); ++i) {
		int index = intArray[i];
		if(index < allItems.size()){
			items.push_back(allItems[index]);
		}
	}
	return items;
}

void MVGCamera::addVisibleItem(const MVGPointCloudItem& item) const
{
	MIntArray intArray;
	MVGMayaUtil::getIntArrayAttribute(_dagpath.node(), _ITEMS, intArray);
	intArray.append(item._id);
	MVGMayaUtil::setIntArrayAttribute(_dagpath.node(), _ITEMS, intArray);
}
