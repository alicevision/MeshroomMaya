#include "mayaMVG/core/MVGScene.h"

using namespace mayaMVG;

MVGScene::MVGScene()
{
}

MVGScene::~MVGScene()
{
}

bool MVGScene::load(const MString& directory)
{
	return false;
}

bool MVGScene::loadCameras(const MString& filename)
{
	return false;
}

bool MVGScene::loadPointCloud(const MString& filename)
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
