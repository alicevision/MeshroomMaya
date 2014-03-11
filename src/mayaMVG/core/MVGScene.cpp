#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/io/MVGCameraReader.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

std::vector<MVGCamera> MVGScene::_cameras;
std::string MVGScene::_projectDirectory;
std::string MVGScene::_cameraDirectoryName;
std::string MVGScene::_imageDirectoryName;

MVGScene::MVGScene()
{
}

MVGScene::~MVGScene()
{
}

bool MVGScene::load()
{
	bool result;
	result = MVGCameraReader::read(_cameras);
	return result;
}

bool MVGScene::loadCameras()
{
	return MVGCameraReader::read(_cameras);
}

bool MVGScene::loadPointCloud()
{
	return false;
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

// std::vector<MVGPointCloud>& MVGScene::getPointClouds()
// {
// 	std::vector<MVGPointCloud> v;
// 	return v;
// }

// std::vector<MVGMesh>& MVGScene::getMeshes()
// {
// 	std::vector<MVGMesh> v;
// 	return v;
// }
