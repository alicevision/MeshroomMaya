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
		static bool load();
		static bool loadCameras();
		static bool loadPointCloud();
	
	public:
		static void setProjectDirectory(const std::string&);
		static void setCameraDirectoryName(const std::string&);
		static void setImageDirectoryName(const std::string&);
		static const std::string& projectDirectory();
		static std::string cameraDirectory();
		static const std::string& cameraDirectoryName();
		static std::string imageDirectory();
		static const std::string& imageDirectoryName();
		static std::string fullPath(const std::string&, const std::string&);
	
	public:
		static const std::vector<MVGCamera>& cameras();

	private:
		static std::string _projectDirectory;
		static std::string _cameraDirectoryName;
		static std::string _imageDirectoryName;
		static std::vector<MVGCamera> _cameras;
};

} // mayaMVG
