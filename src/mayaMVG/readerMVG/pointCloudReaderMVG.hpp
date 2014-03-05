#ifndef POINTCLOUD_CONVERSION_HPP
#define POINTCLOUD_CONVERSION_HPP

#include <openMVG/numeric/numeric.h>
#include "software/SfMViewer/Ply.h"

#include <string>

namespace readerMVG
{
/**
 * 3D point create from ply file
 */
struct PointCloudItem
{
  openMVG::Vec3 position;
  openMVG::Vec3 color;
  openMVG::Vec3 normal;
  std::vector<size_t> visibility; // Index of camera which can see this point
};

/**
 * Parse a .ply to read position color normal and visibility of the points
 * \param[in] sPathToPly path to file ply file
 * \param[out] vec_point3D vector containing all 3D points
 * \return false if the file doesn't exist
 */
bool getPointCloudFromPly( const std::string& sPathToPly,
                           std::vector<PointCloudItem>& vec_point3D )
{
  //-- Read the ply file:
  Ply ply;
  size_t num_vertices = 0;
  // Read PLY header
  if( ! ply.open( sPathToPly ) )
  {
    std::cout << "File doesn't open" << std::endl;
    ply.close();
    return false;
  }
  
  std::cout << "PLY file opened" << std::endl;

  Ply::ElementsIterator it = ply.elements_begin();
  // Iterate over elements
  for (Ply::ElementsIterator it = ply.elements_begin();
    it != ply.elements_end(); ++it)
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
    break;
  }
  if( it == ply.elements_end() )
    return false;
  
  const Ply::Element& vertex_element = *it;

  //Reserve memory
  vec_point3D.reserve( num_vertices);

  for (size_t i = 0; i != num_vertices; ++i)
  {
    PointCloudItem point;
    float weight;
    ply.read_begin(vertex_element);
    for (Ply::PropertiesIterator it2 = vertex_element.properties_begin();
      it2 != vertex_element.properties_end(); ++it2)
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
      else if (property.name() == "red" || property.name() == "diffuse_red" )
      {
        ply.read(property, point.color[0]);
        point.color[0] /= 255.;
      }
      else if (property.name() == "green" || property.name() == "diffuse_green" )
      {
        ply.read(property, point.color[1]);
        point.color[1] /= 255.;
      }
      else if (property.name() == "blue" || property.name() == "diffuse_blue" )
      {
        ply.read(property, point.color[2]);
        point.color[2] /= 255.;
      }
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
  ply.close();
  return true;
}

}

#endif // POINTCLOUD_CONVERSION_HPP