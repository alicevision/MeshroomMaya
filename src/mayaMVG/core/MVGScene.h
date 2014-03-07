#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include <vector>

class MString;

namespace mayaMVG {

class MVGScene {

	public:
		MVGScene();
		virtual ~MVGScene();

	private:
		MVGScene(const MVGScene&);
		
	public:
		bool load(const MString&);
		bool loadCameras(const MString&);
		bool loadPointCloud(const MString&);
		std::vector<MVGCamera> getCameras() const;
		std::vector<MVGPointCloud> getPointClouds() const;
		std::vector<MVGMesh> getMeshes() const;

};

} // mayaMVG
