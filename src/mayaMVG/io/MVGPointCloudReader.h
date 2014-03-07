#pragma once

#include <openMVG/numeric/numeric.h>

namespace mayaMVG {

struct MVGPointCloudReader {
	
	/**
	* Parse a .ply to read position color normal and visibility of the points
	* \param[in] sPathToPly path to file ply file
	* \param[out] vec_point3D vector containing all 3D points
	* \return false if the file doesn't exist
	*/
	static bool getPointCloudFromPly(const std::string& sPathToPly,
							std::vector<Point3D>& vec_point3D )
	{
		//-- Read the ply file:
		Ply ply;
		size_t num_vertices = 0;
		// Read PLY header
		if (ply.open( sPathToPly ) )
		{
			std::cout << "PLY file opened" << std::endl;
			// Iterate over elements
			for (Ply::ElementsIterator it = ply.elements_begin(); it != ply.elements_end(); ++it)
			{
				const Ply::Element& element = *it;
				if (element.name() != "vertex")
				{
					if (!ply.skip(element))
					{
						std::cerr << "Cannot skip element \"" << element.name() << '"' << std::endl;
						ply.close();
						return false;
					}
					continue;
				}
				num_vertices = element.count();
				//Reserve memory
				vec_point3D.reserve( num_vertices);
				for (size_t i = 0; i != num_vertices; ++i)
				{
					Point3D point;
					float weight;
					ply.read_begin(element);
					for (Ply::PropertiesIterator it2 = element.properties_begin(); it2 != element.properties_end(); ++it2)
					{
						const Ply::Property& property = *it2;
						if (property.name() == "x")
							ply.read(property, point.position[0]);
						else if (property.name() == "y")
							ply.read(property, point.position[1]);
						else if (property.name() == "z")
							ply.read(property, point.position[2]);
						//TODO define ply structure
						/*      else if (property.name() == "nx")
						else if (property.name() == "nx")
						ply.read(property, point.normal[0]);
						else if (property.name() == "ny")
						ply.read(property, point.normal[1]);
						else if (property.name() == "nz")
						ply.read(property, point.normal[2]);*/
						else if (property.name() == "red")
							ply.read(property, point.color[0]);
						else if (property.name() == "green")
							ply.read(property, point.color[1]);
						else if (property.name() == "blue")
							ply.read(property, point.color[2]);
						else if (property.name() == "weight" || property.name() == "confidence")
							ply.read(property, weight);
						else if (property.name() == "visibility")
						{
							size_t visibility_count;
							ply.read_count(property, visibility_count);
							point.visibility.reserve( visibility_count );
							while (visibility_count--)
							{
								int visibility_value;
								ply.read_value(property, visibility_value);
								point.visibility.push_back( visibility_value );              
							}
						}
						else if (!ply.skip(property))
						{
							std::cerr << "Cannot skip property \"" << property.name() << '"' << std::endl;
							ply.close();
							return false;
						}
					}
					vec_point3D.push_back( point );
					ply.read_end();
				}
			}
		}
		else
			std::cout << "File doesn't open" << std::endl;
		ply.close();
	}

};

} // mayaMVG
