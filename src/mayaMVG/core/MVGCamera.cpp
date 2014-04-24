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
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MSelectionList.h>
#include <stdexcept>

using namespace mayaMVG;

MVGCamera::MVGCamera(const std::string& name)
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

MVGCamera::MVGCamera(const MDagPath& dagPath)
	: _dagpath(dagPath)
{
}

MVGCamera::~MVGCamera()
{
}

MVGCamera MVGCamera::create(const std::string& name)
{
	MStatus status;
	MFnCamera fnCamera;
	MObject transform = fnCamera.create(&status);
	
	// register dag path
	MDagPath path;
	MDagPath::getAPathTo(transform, path);
	MVGCamera camera(path);
	camera.setName(name);
	return camera;
}

const MDagPath& MVGCamera::dagPath() const
{
	return _dagpath;
}

void MVGCamera::setDagPath(const MDagPath& dagpath)
{
	_dagpath = dagpath;
}

const std::string MVGCamera::name() const
{
	MFnDependencyNode depNode(_dagpath.node());
	return depNode.name().asChar();
}

void MVGCamera::setName(const std::string& name)
{
	MFnDependencyNode depNode(_dagpath.node());
	depNode.setName(name.c_str());
}

const std::string& MVGCamera::imagePlane() const
{
	return _imageName;
}

void MVGCamera::setImagePlane(const std::string& img)
{
	_imageName = img;

	if(_imageName.empty())
		return;


	assert(_dagpath.isValid());
	// FIXME : check if imageplane already exists

	// image plane creation
	MStatus status;
	MDagModifier dagModifier;
	// create image plane
	MObject transform = dagModifier.createNode("imagePlane", MObject::kNullObj, &status);
	dagModifier.doIt();	

	_dagpathImg = MDagPath::getAPathTo(transform);
	_dagpathImg.extendToShape();

	// image plane parameters
	MFnDependencyNode fnDep(_dagpathImg.node(), &status);
	// disabling image loading : do not fill the 'imageName' field
	// fnDep.findPlug("imageName").setValue(MVGScene::fullPath(MVGScene::imageDirectory(), _imageName).c_str());
	fnDep.findPlug("depth").setValue(50);
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

void MVGCamera::loadImagePlane() const
{
	// set imageName attribute on imagePlane
	// starts loading...
	if(_dagpathImg.isValid())
	{
		MFnDependencyNode fn(_dagpathImg.node());
		fn.findPlug("imageName").setValue(MVGScene::fullPath(MVGScene::imageDirectory(), _imageName).c_str());
	}
}

const openMVG::PinholeCamera& MVGCamera::pinholeCamera()
{
	// get pinhole if exists
	if(_pinhole._P != openMVG::Mat34::Identity())
		return _pinhole;
	
	// or retrieve it from maya attributes
	MFnDependencyNode fn(_dagpath.node());
	openMVG::Mat34 P;
	P(0, 0) = fn.findPlug("p00").asFloat();
	P(0, 1) = fn.findPlug("p01").asFloat();
	P(0, 2) = fn.findPlug("p02").asFloat();
	P(0, 3) = fn.findPlug("p03").asFloat();
	P(1, 0) = fn.findPlug("p10").asFloat();
	P(1, 1) = fn.findPlug("p11").asFloat();
	P(1, 2) = fn.findPlug("p12").asFloat();
	P(1, 3) = fn.findPlug("p13").asFloat();
	P(2, 0) = fn.findPlug("p20").asFloat();
	P(2, 1) = fn.findPlug("p21").asFloat();
	P(2, 2) = fn.findPlug("p22").asFloat();
	P(2, 3) = fn.findPlug("p23").asFloat();
	_pinhole = openMVG::PinholeCamera(P);
	return _pinhole;
}

void MVGCamera::setPinholeCamera(const openMVG::PinholeCamera& cam)
{
	assert(_dagpath.isValid());
	_pinhole = cam;

	// set maya camera position
	MFnTransform fnTransform(_dagpath);
	fnTransform.setTranslation(MVector(_pinhole._C(0), _pinhole._C(1), _pinhole._C(2)), MSpace::kTransform);

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

	// register pinhole attribute P (projection matrix P = K[R|t])
	MDagModifier dagModifier;
	MFnCompoundAttribute cAttr;
	MObject dynP = cAttr.create("pinholeProjectionMatrix", "ppm");
	MFnNumericAttribute nAttr;
	MObject o00 = nAttr.create("pinholeP00", "p00", MFnNumericData::kFloat, _pinhole._P(0,0));
	dagModifier.addAttribute(_dagpath.node(), o00);
	cAttr.addChild(o00);
	MObject o01 = nAttr.create("pinholeP01", "p01", MFnNumericData::kFloat, _pinhole._P(0,1));
	dagModifier.addAttribute(_dagpath.node(), o01);
	cAttr.addChild(o01);
	MObject o02 = nAttr.create("pinholeP02", "p02", MFnNumericData::kFloat, _pinhole._P(0,2));
	dagModifier.addAttribute(_dagpath.node(), o02);
	cAttr.addChild(o02);
	MObject o03 = nAttr.create("pinholeP03", "p03", MFnNumericData::kFloat, _pinhole._P(0,3));
	dagModifier.addAttribute(_dagpath.node(), o03);
	cAttr.addChild(o03);
	MObject o10 = nAttr.create("pinholeP10", "p10", MFnNumericData::kFloat, _pinhole._P(1,0));
	dagModifier.addAttribute(_dagpath.node(), o10);
	cAttr.addChild(o10);
	MObject o11 = nAttr.create("pinholeP11", "p11", MFnNumericData::kFloat, _pinhole._P(1,1));
	dagModifier.addAttribute(_dagpath.node(), o11);
	cAttr.addChild(o11);
	MObject o12 = nAttr.create("pinholeP12", "p12", MFnNumericData::kFloat, _pinhole._P(1,2));
	dagModifier.addAttribute(_dagpath.node(), o12);
	cAttr.addChild(o12);
	MObject o13 = nAttr.create("pinholeP13", "p13", MFnNumericData::kFloat, _pinhole._P(1,3));
	dagModifier.addAttribute(_dagpath.node(), o13);
	cAttr.addChild(o13);
	MObject o20 = nAttr.create("pinholeP20", "p20", MFnNumericData::kFloat, _pinhole._P(2,0));
	dagModifier.addAttribute(_dagpath.node(), o20);
	cAttr.addChild(o20);
	MObject o21 = nAttr.create("pinholeP21", "p21", MFnNumericData::kFloat, _pinhole._P(2,1));
	dagModifier.addAttribute(_dagpath.node(), o21);
	cAttr.addChild(o21);
	MObject o22 = nAttr.create("pinholeP22", "p22", MFnNumericData::kFloat, _pinhole._P(2,2));
	dagModifier.addAttribute(_dagpath.node(), o22);
	cAttr.addChild(o22);
	MObject o23 = nAttr.create("pinholeP23", "p23", MFnNumericData::kFloat, _pinhole._P(2,3));
	dagModifier.addAttribute(_dagpath.node(), o23);
	cAttr.addChild(o23);
	dagModifier.addAttribute(_dagpath.node(), dynP);
	dagModifier.doIt();
}

// double MVGCamera::zoom() const
// {
// 	assert(_dagpath.isValid());
// 	MFnCamera fnCamera(_dagpath.child(0));
// 	return fnCamera.zoom();
// }

// void MVGCamera::setZoom(double z)
// {
// 	assert(_dagpath.isValid());
// 	MFnCamera fnCamera(_dagpath.child(0));
// 	fnCamera.setZoom(z);
// }

// void MVGCamera::pan(float x, float y)
// {
// 	assert(_dagpath.isValid());
// 	MFnCamera fnCamera(_dagpath.child(0));
// 	return fnCamera.zoom();
// }

// void MVGCamera::setPan(float x, float y)
// {
// 	assert(_dagpath.isValid());
// 	MFnCamera fnCamera(_dagpath.child(0));
// 	return fnCamera.zoom();
// }

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
}
