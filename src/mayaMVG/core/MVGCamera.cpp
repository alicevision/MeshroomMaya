#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGScene.h"
#include <maya/MPoint.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MDagModifier.h>

using namespace mayaMVG;

MVGCamera::MVGCamera(const std::string& name)
	: _name(name)
	, _step(STEP_NEW)
{
}

MVGCamera::~MVGCamera()
{
}

void MVGCamera::instantiate() 
{
	MStatus status;
	MFnCamera fnCamera;
	MObject transform = fnCamera.create(&status);
	
	// register dag path
	fnCamera.getPath(_dagpath);

	// name
	MFnDependencyNode depNode(transform);
	depNode.setName(_name.c_str());

	// position
	MFnTransform fnTransform(transform);
	openMVG::Vec3 pos = (-1) * _pinhole._R.transpose() * _pinhole._t;
	fnTransform.setTranslation(MVector(pos(0), pos(1), pos(2)), MSpace::kTransform);

	// orientation
	MMatrix m = MMatrix::identity;
	m[0][0] = _pinhole._R(0, 0);
	m[0][1] = _pinhole._R(0, 1);
	m[0][2] = _pinhole._R(0, 2);
	m[1][0] = _pinhole._R(1, 0) * -1; // ???
	m[1][1] = _pinhole._R(1, 1) * -1;
	m[1][2] = _pinhole._R(1, 2) * -1;
	m[2][0] = _pinhole._R(2, 0) * -1;
	m[2][1] = _pinhole._R(2, 1) * -1;
	m[2][2] = _pinhole._R(2, 2) * -1;
	MQuaternion quaternion;
	quaternion = m;
	fnTransform.setRotation(quaternion, MSpace::kTransform);

	// intrinsic parameters
	size_t focal = _pinhole._K(0, 0);
	size_t width = _pinhole._K(0, 2) * 2;
	size_t height = _pinhole._K(1, 2) * 2;
	fnCamera.setVerticalFilmAperture(fnCamera.horizontalFilmAperture() * ((double)height / (double)width));
	fnCamera.setHorizontalFieldOfView((2.0 * atan((double)width / (2.0 * (double)focal))));
	fnCamera.setPanZoomEnabled(true);
	fnCamera.setFilmFit(MFnCamera::kHorizontalFilmFit);

	// lock
	depNode.findPlug("translateX").setLocked(true);
	depNode.findPlug("translateY").setLocked(true);
	depNode.findPlug("translateZ").setLocked(true);
	depNode.findPlug("rotateX").setLocked(true);
	depNode.findPlug("rotateY").setLocked(true);
	depNode.findPlug("rotateZ").setLocked(true);
}

const std::string& MVGCamera::name() const
{
	return _name;
}

void MVGCamera::setName(const std::string& name)
{
	_name = name;
}

const std::string& MVGCamera::imageName() const
{
	return _imageName;
}

void MVGCamera::setImageName(const std::string& img)
{
	_imageName = img;

	if(_dagpath.isValid())
	{
		// check if imageplane already exists
		//FIXME

		// create image plane
		MStatus status;
		MDagModifier dagModifier;
		MObject transform = dagModifier.createNode("imagePlane", MObject::kNullObj, &status);
		dagModifier.doIt();

		MDagPath path = MDagPath::getAPathTo(transform);
		path.extendToShape();
		MFnDependencyNode fnDep(path.node(), &status);

		// image plane parameters
		fnDep.findPlug("imageName").setValue(MVGScene::fullPath(MVGScene::imageDirectory(), _imageName).c_str());
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
}

const openMVG::PinholeCamera& MVGCamera::pinholeCamera() const
{
	return _pinhole;
}

void MVGCamera::setPinholeCamera(const openMVG::PinholeCamera& cam)
{
	_pinhole = cam;
}

void MVGCamera::add2DPoint(const MPoint&)
{
}

void MVGCamera::move2DPoint(const MPoint&)
{
}

void MVGCamera::setZoom(float z)
{
}

void MVGCamera::setPan(float x, float y)
{
}
