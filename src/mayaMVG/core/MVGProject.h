#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include <vector>

namespace mayaMVG {

class MVGProject {
	
	public:
		MVGProject();
		virtual ~MVGProject();	
	
		bool load();
		bool loadCameras();
		bool loadPointCloud();
		
		void setLeftView(const MVGCamera& camera) const;
		void setRightView(const MVGCamera& camera) const;
	
	public:
		std::string projectDirectory();
		void setProjectDirectory(const std::string&);
		std::string cameraFile();
		std::string cameraBinary(const std::string&);
		std::string cameraDirectory();
		std::string imageFile(const std::string&);
		std::string imageDirectory();
		std::string pointCloudFile();
		std::vector<MVGCamera> cameras();
	
	public:
		// openMVG node names
		static std::string _CLOUD;
		static std::string _MESH;

};

} // mayaMVG
