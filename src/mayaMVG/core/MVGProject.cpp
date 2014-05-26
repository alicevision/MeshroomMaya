#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/io/cameraIO.h"
#include "mayaMVG/io/pointCloudIO.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MFnTransform.h>
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::string MVGProject::_CLOUD = "mvgPointCloud";
std::string MVGProject::_MESH = "mvgMesh";
std::string MVGProject::_PREVIEW_MESH = "previewMesh";
std::string MVGProject::_PROJECT = "mayaMVG";

MVGProject::MVGProject(const std::string& name)
	: MVGNodeWrapper(name)
{
}

MVGProject::MVGProject(const MDagPath& dagPath)
	: MVGNodeWrapper(dagPath)
{
}

MVGProject::~MVGProject()
{
}

// virtual
bool MVGProject::isValid() const
{
	if(!_dagpath.isValid() || (_dagpath.apiType()!=MFn::kTransform))
		return false;
	return true;
}

// static
MVGProject MVGProject::create(const std::string& name)
{
	MStatus status;
	MFnTransform fn;
	MDagPath path;
	MObject transform = fn.create(MObject::kNullObj, &status);
	CHECK(status)
	MDagPath::getAPathTo(transform, path);
	MVGProject project(path);
	project.setName(name);
	return project;
}

bool MVGProject::load()
{
	if(!loadCameras())
		return false;
	if(!loadPointCloud())
		return false;
	return true;
}

bool MVGProject::loadCameras()
{
	return readCamera(cameraFile(), imageDirectory(), cameraDirectory());
}

bool MVGProject::loadPointCloud()
{

	return readPointCloud(pointCloudFile());
}

void MVGProject::setLeftView(const MVGCamera& camera) const
{
	camera.select();
	camera.loadImagePlane();
	MVGMayaUtil::setMVGLeftCamera(camera);
}

void MVGProject::setRightView(const MVGCamera& camera) const
{
	camera.select();
	camera.loadImagePlane();
	MVGMayaUtil::setMVGRightCamera(camera);
}

std::string MVGProject::moduleDirectory() const
{
	return MVGMayaUtil::getModulePath().asChar();
}

std::string MVGProject::projectDirectory() const
{
	MStringArray result;
	MGlobal::executeCommand("fileInfo -q \"openMVG_root_dir\";", result);
	return (result.length() > 0) ? result[0].asChar() : "";
}

void MVGProject::setProjectDirectory(const std::string& directory) const
{
	MGlobal::executeCommand(
		MString("fileInfo \"openMVG_root_dir\" \"")+directory.c_str()+"\";");
}

std::string MVGProject::cameraFile() const
{
	return stlplus::create_filespec(projectDirectory(), "views.txt");
}

std::string MVGProject::cameraBinary(const std::string& bin) const
{
	return stlplus::create_filespec(cameraDirectory(), bin);
}

std::string MVGProject::cameraDirectory() const
{
	return stlplus::create_filespec(projectDirectory(), "cameras");
}

std::string MVGProject::imageFile(const std::string& img) const
{
	return stlplus::create_filespec(imageDirectory(), img);
}

std::string MVGProject::imageDirectory() const
{
	return stlplus::create_filespec(projectDirectory(), "images");
}

std::string MVGProject::pointCloudFile() const
{
	// return stlplus::create_filespec(
	// 		stlplus::create_filespec(
	// 			stlplus::create_filespec(
	// 				stlplus::create_filespec(
	// 					stlplus::create_filespec(
	// 						projectDirectory(), ".."), "PMVS"), "models"), "pmvs_optionMiMode"), "new.ply");
	return stlplus::create_filespec(
		stlplus::create_filespec(projectDirectory(), "clouds"), "calib.ply");
}

std::vector<MVGCamera> MVGProject::cameras() const
{
	return MVGCamera::list();
}
