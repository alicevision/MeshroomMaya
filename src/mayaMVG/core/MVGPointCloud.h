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
		void getItemsFromProjection(std::vector<MVGPointCloudItem>& items, MVGCamera& camera, MVGFace2D& face2D) const;
		
	private:
		MDagPath _dagpath;
		std::vector<MVGPointCloudItem> _items;
		
};

} // mayaMVG
