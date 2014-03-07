#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include <vector>

namespace mayaMVG {

class MVGScene {

	public:
		MVGScene();
		virtual ~MVGScene();

	private:
		MVGScene(const MVGScene&);
		
	public:
		static bool load(const std::string&);
		static bool loadCameras(const std::string&);
		static bool loadPointCloud(const std::string&);
		std::vector<MVGCamera> getCameras() const;
		std::vector<MVGPointCloud> getPointClouds() const;
		std::vector<MVGMesh> getMeshes() const;

	private:
		static std::vector<MVGCamera> _cameras;
};

} // mayaMVG
