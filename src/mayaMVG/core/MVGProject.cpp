#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/io/cameraIO.h"
#include "mayaMVG/io/pointCloudIO.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MFnTransform.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::string MVGProject::_CLOUD = "mvgPointCloud";
std::string MVGProject::_MESH = "mvgMesh";
std::string MVGProject::_PROJECT = "mayaMVG";

MString	MVGProject::_PROJECTPATH = "project";

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
	
	// root node
	MObject transform = fn.create(MObject::kNullObj, &status);	
	// root/cameras
	fn.create(transform, &status);
	fn.setName("cameras");
	// root/clouds
	fn.create(transform, &status);
	fn.setName("clouds");
	// root/meshes
	fn.create(transform, &status);
	fn.setName("meshes");
	CHECK(status)

	MDagPath::getAPathTo(transform, path);
	MVGProject project(path);
	project.setName(name);
	
	// Add root attributes
	MDagModifier dagModifier;
	MFnTypedAttribute tAttr;
	MObject projectAttr = tAttr.create(_PROJECTPATH, "project", MFnData::kString);
	dagModifier.addAttribute(path.node(), projectAttr);
	dagModifier.doIt();
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
	MString directory;
	MVGMayaUtil::getStringAttribute(_dagpath.node(), _PROJECTPATH, directory);
	
	return directory.asChar();
}

void MVGProject::setProjectDirectory(const std::string& directory) const
{
	MVGMayaUtil::setStringAttribute(_dagpath.node(), _PROJECTPATH, directory.c_str());
}

std::string MVGProject::cameraFile() const
{
	return stlplus::folder_append_separator(projectDirectory())
					+ stlplus::folder_append_separator("outIncremental")
					+ stlplus::folder_append_separator("SfM_output")
					+ "views.txt";
}

std::string MVGProject::cameraBinary(const std::string& bin) const
{
	return stlplus::create_filespec(cameraDirectory(), bin);
}

std::string MVGProject::cameraDirectory() const
{
	return stlplus::folder_append_separator(projectDirectory())
					+ stlplus::folder_append_separator("outIncremental")
					+ stlplus::folder_append_separator("SfM_output")
					+ stlplus::folder_append_separator("cameras");
}

std::string MVGProject::imageFile(const std::string& img) const
{
	return stlplus::create_filespec(imageDirectory(), img);
}

std::string MVGProject::imageDirectory() const
{
	return stlplus::folder_append_separator(projectDirectory())
					+ stlplus::folder_append_separator("outIncremental")
					+ stlplus::folder_append_separator("SfM_output")
					+ stlplus::folder_append_separator("images");
}

std::string MVGProject::pointCloudFile() const
{
	return stlplus::folder_append_separator(projectDirectory())
					+ stlplus::folder_append_separator("outIncremental")
					+ stlplus::folder_append_separator("SfM_output")
					+ stlplus::folder_append_separator("clouds")
					+ "calib.ply";
					//+ "FinalColorized.ply";
}

std::vector<MVGCamera> MVGProject::cameras() const
{
	return MVGCamera::list();
}
