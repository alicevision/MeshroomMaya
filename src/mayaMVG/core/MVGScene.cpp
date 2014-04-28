#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/io/MVGCameraReader.h"
#include "mayaMVG/io/MVGPointCloudReader.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::string MVGScene::_CLOUD = "mvgPointCloud";
std::string MVGScene::_MESH = "mvgMesh";

bool MVGScene::load()
{
	if(!loadCameras())
		return false;
	if(!loadPointCloud())
		return false;
	return true;
}

bool MVGScene::loadCameras()
{
	return MVGCameraReader::read();
}

bool MVGScene::loadPointCloud()
{
	return MVGPointCloudReader::read();
}

std::string MVGScene::projectDirectory()
{
	MStringArray result;
	MGlobal::executeCommand("fileInfo -q \"openMVG_root_dir\";", result);
	return (result.length() > 0) ? result[0].asChar() : "";
}

void MVGScene::setProjectDirectory(const std::string& directory)
{
	MGlobal::executeCommand(
		MString("fileInfo \"openMVG_root_dir\" \"")+directory.c_str()+"\";");
}

std::string MVGScene::cameraFile()
{
	return stlplus::create_filespec(projectDirectory(), "views.txt");
}

std::string MVGScene::cameraBinary(const std::string& bin)
{
	return stlplus::create_filespec(cameraDirectory(), bin);
}

std::string MVGScene::cameraDirectory()
{
	return stlplus::create_filespec(projectDirectory(), "cameras");
}

std::string MVGScene::imageFile(const std::string& img)
{
	return stlplus::create_filespec(imageDirectory(), img);
}

std::string MVGScene::imageDirectory()
{
	return stlplus::create_filespec(projectDirectory(), "images");
}

std::string MVGScene::pointCloudFile()
{
	return stlplus::create_filespec(
			stlplus::create_filespec(
				stlplus::create_filespec(
					stlplus::create_filespec(
						stlplus::create_filespec(
							projectDirectory(), ".."), "PMVS"), "models"), "pmvs_optionMiMode"), "new.ply");
	// return stlplus::create_filespec(
	// 	stlplus::create_filespec(projectDirectory(), "clouds"), "calib.ply");
}
