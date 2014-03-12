#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MPoint.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>

using namespace mayaMVG;

MVGCamera::MVGCamera(const std::string& name)
	: _name(name)
	, _step(STEP_NEW)
{
}

MVGCamera::~MVGCamera()
{
}

const std::string& MVGCamera::name() const
{
	return _name;
}

void MVGCamera::setName(const std::string& name)
{
	_name = name;

	// set maya camera name
	if(!_dagpath.isValid())
		instantiate();
	MFnDependencyNode depNode(_dagpath.node());
	depNode.setName(_name.c_str());
}

const std::string& MVGCamera::imageName() const
{
	return _imageName;
}

void MVGCamera::setImageName(const std::string& img)
{
	_imageName = img;

	if(_imageName.empty())
		return;

	if(!_dagpath.isValid())
		instantiate();
	// FIXME : check if imageplane already exists

	// image plane creation
	MStatus status;
	MDagModifier dagModifier;
	// create image plane
	MObject transform = dagModifier.createNode("imagePlane", MObject::kNullObj, &status);
	dagModifier.doIt();	

	_dagpathImg = MDagPath::getAPathTo(transform);
	_dagpathImg.extendToShape();

	// // then add a dynamic attribute
	// MFnTypedAttribute tAttr;
	// MObject dynAdd = tAttr.create("mvgImageName", "min", MFnData::kString);
	// dagModifier.addAttribute(_dagpathImg.node(), dynAdd);
	// dagModifier.doIt();	

	// image plane parameters
	MFnDependencyNode fnDep(_dagpathImg.node(), &status);
	//fnDep.findPlug("imageName").setValue(MVGScene::fullPath(MVGScene::imageDirectory(), _imageName).c_str());
	fnDep.findPlug("depth").setValue(50);
	fnDep.findPlug("alphaGain").setValue(0.55);
	fnDep.findPlug("dic").setValue(1);
	fnDep.findPlug("displayOnlyIfCurrent").setValue(1);
	fnDep.findPlug("fit").setValue(2);
	// fnDep.findPlug("width").setValue((int)width);
	// fnDep.findPlug("height").setValue((int)height);
	// fnDep.findPlug("verticalFilmAperture").setValue(defVFA);

	// Reparent & connect image plane to camera
	MFnCamera fnCamera(_dagpath);
	dagModifier.reparentNode(transform, _dagpath.node());
	dagModifier.connect(fnDep.findPlug("message"), fnCamera.findPlug("imagePlane"));
	dagModifier.doIt();
}

const openMVG::PinholeCamera& MVGCamera::pinholeCamera() const
{
	return _pinhole;
}

void MVGCamera::setPinholeCamera(const openMVG::PinholeCamera& cam)
{
	_pinhole = cam;

	if(!_dagpath.isValid())
		instantiate();

	// set maya camera position
	MFnTransform fnTransform(_dagpath);
	openMVG::Vec3 pos = (-1) * _pinhole._R.transpose() * _pinhole._t;
	fnTransform.setTranslation(MVector(pos(0), pos(1), pos(2)), MSpace::kTransform);

	// set maya camera orientation 
	MMatrix m = MMatrix::identity;
	m[0][0] = _pinhole._R(0, 0);
	m[0][1] = _pinhole._R(0, 1);
	m[0][2] = _pinhole._R(0, 2);
	m[1][0] = _pinhole._R(1, 0) * -1; // FIXME
	m[1][1] = _pinhole._R(1, 1) * -1; // ?
	m[1][2] = _pinhole._R(1, 2) * -1; // ?
	m[2][0] = _pinhole._R(2, 0) * -1; // ?
	m[2][1] = _pinhole._R(2, 1) * -1; // ?
	m[2][2] = _pinhole._R(2, 2) * -1; // ?
	MQuaternion quaternion;
	quaternion = m;
	fnTransform.setRotation(quaternion, MSpace::kTransform);

	// set maya camera intrinsic parameters
	size_t focal = _pinhole._K(0, 0);
	size_t width = _pinhole._K(0, 2) * 2;
	size_t height = _pinhole._K(1, 2) * 2;
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
}

void MVGCamera::setZoom(float z)
{
}

void MVGCamera::setPan(float x, float y)
{
}

void MVGCamera::add2DPoint(const MPoint&)
{
}

void MVGCamera::move2DPoint(const MPoint&)
{
}

void MVGCamera::select() const
{
	MSelectionList list;
	list.add(_dagpath);
	MGlobal::setActiveSelectionList(list);

	// set imageName attribute on imagePlane
	// starts loading...
	if(_dagpathImg.isValid())
	{
		MFnDependencyNode fn(_dagpathImg.node());
		fn.findPlug("imageName").setValue(MVGScene::fullPath(MVGScene::imageDirectory(), _imageName).c_str());
	}
}

void MVGCamera::instantiate() 
{
	if(_dagpath.isValid())
		return;

	MStatus status;
	MFnCamera fnCamera;
	MObject transform = fnCamera.create(&status);
	
	// register dag path
	MDagPath::getAPathTo(transform, _dagpath);
	if(!_dagpath.isValid())
		return;

	// set camera parameters
	setName(name());
	setImageName(imageName());
	setPinholeCamera(pinholeCamera());

}