#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGCameraReader.h"

using namespace mayaMVG;

MVGScene::MVGScene()
{
}

MVGScene::~MVGScene()
{
}

bool MVGScene::load(const std::string& directory)
{
	bool result;
	result = MVGCameraReader::read(_cameras, directory);
	return result;
}

bool MVGScene::loadCameras(const std::string& filename)
{
	return MVGCameraReader::read(_cameras, directory);
}

bool MVGScene::loadPointCloud(const std::string& filename)
{
	return false;
}

std::vector<MVGCamera> MVGScene::getCameras() const
{
	std::vector<MVGCamera> v;
	return v;
}

std::vector<MVGPointCloud> MVGScene::getPointClouds() const
{
	std::vector<MVGPointCloud> v;
	return v;
}

std::vector<MVGMesh> MVGScene::getMeshes() const
{
	std::vector<MVGMesh> v;
	return v;
}
