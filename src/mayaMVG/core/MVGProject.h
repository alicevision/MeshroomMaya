#ifndef MVGPROJECT_H
#define	MVGPROJECT_H

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
	
	public:
		void setProjectDirectory(const std::string&);
		void setCameraDirectoryName(const std::string&);
		void setImageDirectoryName(const std::string&);
		
		const std::string& projectDirectory() const;
		std::string cameraDirectory() const;
		const std::string& cameraDirectoryName() const;
		std::string imageDirectory() const;
		const std::string& imageDirectoryName() const;
		
		std::string fullPath(const std::string&, const std::string&) const;
	
	public:
		const std::vector<MVGCamera>& cameras();

	private:
		std::string _projectDirectory;
		std::string _cameraDirectoryName;
		std::string _imageDirectoryName;
		std::vector<MVGCamera> _cameras;
};

} // mayaMVG


#endif	/* MVGPROJECT_H */

