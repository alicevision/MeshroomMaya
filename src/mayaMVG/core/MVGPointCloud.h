#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"
#include "mayaMVG/core/MVGPointCloudItem.h"
#include <vector>

namespace mayaMVG {

class MVGCamera;

class MVGPointCloud : public MVGNodeWrapper  {

	public:
		MVGPointCloud(const std::string& name);
		MVGPointCloud(const MDagPath& dagPath);
		virtual ~MVGPointCloud();

	public:
		virtual bool isValid() const;
		
	public:
		static MVGPointCloud create(const std::string& name);

	public:
		void setItems(const std::vector<MVGPointCloudItem>& items);
		std::vector<MVGPointCloudItem> getItems() const;
		
	private:
		// dynamic attributes
		static MString _RGBPP;
};

} // mayaMVG
