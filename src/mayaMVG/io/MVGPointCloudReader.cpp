#include "mayaMVG/io/MVGPointCloudReader.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include <software/SfMViewer/Ply.h>

using namespace mayaMVG;

bool MVGPointCloudReader::read()
{
	MVGPointCloud pointCloud(MVGScene::_CLOUD);
	if(!pointCloud.isValid()) {
		pointCloud = MVGPointCloud::create(MVGScene::_CLOUD);
		LOG_INFO("New MVG Point Cloud.")
	}

	std::string file = MVGScene::pointCloudFile();
	Ply ply;
	if(!ply.open(file)) {
		LOG_ERROR("Point cloud file not found (" << file << ")")
		ply.close();
		return false;
	}


	std::vector<MVGCamera> cameraList = MVGCamera::list();
	std::sort(cameraList.begin(), cameraList.end()); // sort by camera id

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
						if(cameraId < cameraList.size())
							cameraList[cameraId].addVisibleItem(item);							
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
	ply.close();
	return true;
}
