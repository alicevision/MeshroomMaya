#pragma once

#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/io/pointCloudIO.h"
#include "mayaMVG/core/MVGLog.h"
#include <software/SfMViewer/Ply.h>


namespace mayaMVG {
	
bool readPointCloud(std::string filePath)	
{
	MVGPointCloud pointCloud(MVGProject::_CLOUD);
	if(!pointCloud.isValid()) {
		pointCloud = MVGPointCloud::create(MVGProject::_CLOUD);
		LOG_INFO("New OpenMVG Point Cloud.")
	}

	Ply ply;
	if(!ply.open(filePath))
	{
		LOG_ERROR("Point cloud file not found (" << filePath << ")")
		ply.close();
		return false;
	}

	// camera list
	std::vector<MVGCamera> cameraList = MVGCamera::list();
	std::sort(cameraList.begin(), cameraList.end()); // sort by camera id
	
	// items per camera tmp list
	std::vector<std::vector<MVGPointCloudItem> > itemsPerCam;
	itemsPerCam.reserve(cameraList.size());
	for(size_t i=0; i<cameraList.size(); ++i)
		itemsPerCam.push_back(std::vector<MVGPointCloudItem>());

	// parse ply file
	std::vector<MVGPointCloudItem> items;
	for(Ply::ElementsIterator it = ply.elements_begin(); it != ply.elements_end(); ++it)
	{
		const Ply::Element& element = *it;
		if(element.name() != "vertex")
		{
			if(!ply.skip(element))
			{
				ply.close();
				return false;
			}
			continue;
		}
		items.reserve(element.count());
		for(size_t i = 0; i != element.count(); ++i)
		{
			MVGPointCloudItem item;
			ply.read_begin(element);
			for(Ply::PropertiesIterator it2 = element.properties_begin(); it2 != element.properties_end(); ++it2)
			{
				const Ply::Property& property = *it2;
				if (property.name() == "x")
					ply.read(property, item._position[0]);
				else if(property.name() == "y")
					ply.read(property, item._position[1]);
				else if(property.name() == "z")
					ply.read(property, item._position[2]);
				else if(property.name() == "red" || property.name() == "diffuse_red")
				{
					ply.read(property, item._color[0]);
					item._color[0] /= 255.;
				}
				else if(property.name() == "green" || property.name() == "diffuse_green")
				{
					ply.read(property, item._color[1]);
					item._color[1] /= 255.;
				}
				else if(property.name() == "blue" || property.name() == "diffuse_blue")
				{
					ply.read(property, item._color[2]);
					item._color[2] /= 255.;
				}
				else if(property.name() == "weight" || property.name() == "confidence")
					ply.read(property, item._weight);
				else if(property.name() == "visibility")
				{
					item._id = i;
					size_t cameraVisibilityCount;
					ply.read_count(property, cameraVisibilityCount);
					int cameraId;
					while(cameraVisibilityCount--) {
						ply.read_value(property, cameraId);
						if(cameraId > itemsPerCam.size())
							LOG_ERROR("Invalid camera id (" << cameraId << ")")
						else
							itemsPerCam[cameraId].push_back(item);
					}
				}
				else if(!ply.skip(property))
				{
					ply.close();
					return false;
				}
			}
			items.push_back(item);
			ply.read_end();
		}
	}

	// set items visibility
	for(size_t i=0; i<cameraList.size(); ++i)
		cameraList[i].setVisibleItems(itemsPerCam[i]);

	pointCloud.setItems(items);
	ply.close();
	return true;
}

} // mayaMVG
