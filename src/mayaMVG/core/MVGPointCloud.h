#pragma once

#include "mayaMVG/core/MVGPointCloudItem.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <vector>

namespace mayaMVG {

class MVGCamera;

class MVGPointCloud {

	public:
		MVGPointCloud(const std::string& name);
		MVGPointCloud(const MDagPath& dagPath);
		virtual ~MVGPointCloud();

	public:
		static MVGPointCloud create(const std::string& name);

	public:
		void setName(const std::string&);
		const std::string name() const;
		void setItems(const std::vector<MVGPointCloudItem>& items);
		std::vector<MVGPointCloudItem> getItems() const;
		
	private:
		MDagPath _dagpath;
	
};

} // mayaMVG
