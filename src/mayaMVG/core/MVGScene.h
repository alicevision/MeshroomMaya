#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include <vector>

namespace mayaMVG {

struct MVGScene {
	// import
	static bool load();
	static bool loadCameras();
	static bool loadPointCloud();
	// project directories & files
	static std::string projectDirectory();
	static void setProjectDirectory(const std::string&);
	static std::string cameraFile();
	static std::string cameraBinary(const std::string&);
	static std::string cameraDirectory();
	static std::string imageFile(const std::string&);
	static std::string imageDirectory();
	static std::string pointCloudFile();
	// openMVG node names
	static std::string _CLOUD;
	static std::string _MESH;
};

} // mayaMVG





