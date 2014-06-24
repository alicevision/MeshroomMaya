#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include <vector>

namespace mayaMVG {

class MVGProject : public MVGNodeWrapper {
	
	public:
		MVGProject(const std::string& name = "mayaMVG");
		MVGProject(const MDagPath& dagPath);
		virtual ~MVGProject();	
		
	public:
		virtual bool isValid() const;
	
	public:
		static MVGProject create(const std::string& name);

	public:
		bool load();
		bool loadCameras();
		bool loadPointCloud();
	
	public:
		// views
		void setLeftView(const MVGCamera& camera) const;
		void setRightView(const MVGCamera& camera) const;
		// filesystem
		std::string moduleDirectory() const;
		std::string projectDirectory() const;
		void setProjectDirectory(const std::string&) const;
		std::string cameraFile() const;
		std::string cameraBinary(const std::string&) const;
		std::string cameraDirectory() const;
		std::string imageFile(const std::string&) const;
		std::string imageDirectory() const;
		std::string pointCloudFile() const;
		// nodes
		std::vector<MVGCamera> cameras() const;
	
	public:
		// openMVG node names
		static std::string _CLOUD;
		static std::string _MESH;
		static std::string _PREVIEW_MESH;
		static std::string _PROJECT;
		
		// dynamic attributes
		static MString _PROJECTPATH;
};

} // mayaMVG
