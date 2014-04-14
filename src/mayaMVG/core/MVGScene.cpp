#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/io/MVGCameraReader.h"
#include "mayaMVG/io/MVGPointCloudReader.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::string MVGScene::_projectDirectory;
std::string MVGScene::_cameraDirectoryName;
std::string MVGScene::_imageDirectoryName;
std::vector<MVGCamera> MVGScene::_cameras;

MVGScene::MVGScene()
{
}

MVGScene::~MVGScene()
{
}

bool MVGScene::load()
{
	// cameras
	if(!loadCameras())
		return false;
	// point cloud
	if(!loadPointCloud())
		return false;
	// mesh
	MVGMesh::create("mvgMesh");
	return true;
}

bool MVGScene::loadCameras()
{
	return MVGCameraReader::read(_cameras);
}

bool MVGScene::loadPointCloud()
{
	MVGPointCloud pointCloud = MVGPointCloud::create("mvgPointCloud");
	return MVGPointCloudReader::read(pointCloud);
}

const std::vector<MVGCamera>& MVGScene::cameras()
{
	return _cameras;
}

void MVGScene::setProjectDirectory(const std::string& directory)
{
	_projectDirectory = stlplus::folder_append_separator(directory);
}

void MVGScene::setCameraDirectoryName(const std::string& name)
{
	_cameraDirectoryName = stlplus::folder_append_separator(name);
}

void MVGScene::setImageDirectoryName(const std::string& name)
{
	_imageDirectoryName = stlplus::folder_append_separator(name);
}

const std::string& MVGScene::projectDirectory()
{
	return _projectDirectory;
}

std::string MVGScene::cameraDirectory()
{
	return stlplus::create_filespec(_projectDirectory, _cameraDirectoryName);
}

const std::string& MVGScene::cameraDirectoryName()
{
	return _cameraDirectoryName;
}

std::string MVGScene::imageDirectory()
{
	return stlplus::create_filespec(_projectDirectory, _imageDirectoryName);
}

const std::string& MVGScene::imageDirectoryName()
{
	return _imageDirectoryName;
}

std::string MVGScene::fullPath(const std::string& directory, const  std::string& file)
{
	return stlplus::create_filespec(directory, file);
}
