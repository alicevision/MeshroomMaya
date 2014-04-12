#include "mayaMVG/io/MVGPointCloudReader.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include <software/SfMViewer/Ply.h>

using namespace mayaMVG;

bool MVGPointCloudReader::read(std::vector<MVGPointCloud>& pointClouds)
{
	std::string file = MVGScene::fullPath(MVGScene::fullPath(MVGScene::projectDirectory(), "clouds"), "calib.ply");

	Ply ply;
	if(!ply.open(file))
	{
		LOG_ERROR("File doesn't open")
		ply.close();
		return false;
	}

	MVGPointCloud pointCloud = MVGPointCloud::create("pointCloud");
	std::vector<MVGPointCloudItem> items;
	// iterate over elements
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
					size_t cameraVisibilityCount;
					ply.read_count(property, cameraVisibilityCount);
					item._visibility.reserve(cameraVisibilityCount);
					while(cameraVisibilityCount--)
					{
						int cameraId;
						ply.read_value(property, cameraId);
						item._visibility.push_back(cameraId);
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
	pointCloud.setItems(items);
	pointClouds.push_back(pointCloud);
	ply.close();
	return true;
}