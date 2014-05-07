#include "MVGProject.h"

#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/io/cameraIO.h"
#include "mayaMVG/io/pointCloudIO.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

using namespace mayaMVG;

MVGProject::MVGProject()
{
}

MVGProject::~MVGProject()
{
}

bool MVGProject::load(std::string projectDirectoryPath)
{	
	
	setProjectDirectory(projectDirectoryPath);
	
	// cameras
	if(!loadCameras(fullPath(_projectDirectory, "views.txt")))
		return false;
	// point cloud
	if(!loadPointCloud(fullPath(fullPath(_projectDirectory, "clouds"), "calib.ply")))
		return false;
	// mesh
	MVGMesh::create("mvgMesh");
	return true;
}

bool MVGProject::loadCameras(std::string filePath)
{
	return readCamera(_cameras, filePath, _imageDirectoryName, _cameraDirectoryName);	
}

bool MVGProject::loadPointCloud(std::string filePath)
{
	MVGPointCloud pointCloud = MVGPointCloud::create("mvgPointCloud");
	
	return readPointCloud(pointCloud, filePath);
}

const std::vector<MVGCamera>& MVGProject::cameras()
{
	return _cameras;
}

void MVGProject::setProjectDirectory(const std::string& directory)
{
	_projectDirectory = stlplus::folder_append_separator(directory);
}

void MVGProject::setCameraDirectoryName(const std::string& name)
{
	_cameraDirectoryName = stlplus::folder_append_separator(name);
}

void MVGProject::setImageDirectoryName(const std::string& name)
{
	_imageDirectoryName = stlplus::folder_append_separator(name);
}

const std::string& MVGProject::projectDirectory() const
{
	return _projectDirectory;
}

std::string MVGProject::cameraDirectory() const
{
	return stlplus::create_filespec(_projectDirectory, _cameraDirectoryName);
}

const std::string& MVGProject::cameraDirectoryName() const
{
	return _cameraDirectoryName;
}

std::string MVGProject::imageDirectory() const
{
	return stlplus::create_filespec(_projectDirectory, _imageDirectoryName);
}

const std::string& MVGProject::imageDirectoryName() const
{
	return _imageDirectoryName;
}

std::string MVGProject::fullPath(const std::string& directory, const  std::string& file) const
{
	return stlplus::create_filespec(directory, file);
}
