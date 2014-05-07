#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/io/cameraIO.h"
#include "mayaMVG/io/pointCloudIO.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::string MVGProject::_CLOUD = "mvgPointCloud";
std::string MVGProject::_MESH = "mvgMesh";

MVGProject::MVGProject()
{
}

MVGProject::~MVGProject()
{
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

std::string MVGProject::moduleDirectory()
{
	return MVGMayaUtil::getModulePath().asChar();
}

std::string MVGProject::projectDirectory()
{
	MStringArray result;
	MGlobal::executeCommand("fileInfo -q \"openMVG_root_dir\";", result);
	return (result.length() > 0) ? result[0].asChar() : "";
}

void MVGProject::setProjectDirectory(const std::string& directory)
{
	MGlobal::executeCommand(
		MString("fileInfo \"openMVG_root_dir\" \"")+directory.c_str()+"\";");
}

std::string MVGProject::cameraFile()
{
	return stlplus::create_filespec(projectDirectory(), "views.txt");
}

std::string MVGProject::cameraBinary(const std::string& bin)
{
	return stlplus::create_filespec(cameraDirectory(), bin);
}

std::string MVGProject::cameraDirectory()
{
	return stlplus::create_filespec(projectDirectory(), "cameras");
}

std::string MVGProject::imageFile(const std::string& img)
{
	return stlplus::create_filespec(imageDirectory(), img);
}

std::string MVGProject::imageDirectory()
{
	return stlplus::create_filespec(projectDirectory(), "images");
}

std::string MVGProject::pointCloudFile()
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

std::string getPath(const std::string& dir, const std::string& filename)
{
	return stlplus::create_filespec(dir, filename);
}

std::vector<MVGCamera> MVGProject::cameras()
{
	return MVGCamera::list();
}
