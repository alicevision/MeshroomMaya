#ifndef N_VIEW_2D_POINT_TO_3D_HPP
#define N_VIEW_2D_POINT_TO_3D_HPP

// openMVG includes
#include <openMVG/multiview/triangulation_nview.hpp>
#include <openMVG/cameras/PinholeCamera.hpp>
#include <openMVG/numeric/numeric.h>

// std includes
#include <vector>

namespace geometry
{
  
/**
  * Structure to store the relative camera to a 2D point projection
  */
struct PositionByCam
{
  openMVG::Vec2 point;
  openMVG::PinholeCamera camera;  
  
  PositionByCam( const openMVG::Vec2& point, const openMVG::PinholeCamera& camera )
  {
    this->point = point;
    this->camera = camera;
  }
};


/**
 * Triangulate position from 2D points
 * \param[in] map_position2dByCamera vector of 2D point and camrea model
 * \param[out] point 3D point computed
 * \return true if the 3D point is computed
 */
bool triangulatePosition( const std::vector< geometry::PositionByCam >& vec_position2dByCamera,
                          openMVG::Vec3 & point3D )
{  
  if ( vec_position2dByCamera.size() < 2 )
  {
    std::cout << "There is not enough camera to triangulate a position. You need at least 2 cameras." << std::endl;
    return false;
  }
  std::vector<openMVG::Mat34> matricesP(vec_position2dByCamera.size());
  openMVG::Mat2X points2D(2, vec_position2dByCamera.size());
  int index = 0;
  for ( std::vector< geometry::PositionByCam >::const_iterator iter_cam = vec_position2dByCamera.begin();
        iter_cam != vec_position2dByCamera.end();
        iter_cam++,
        index++
      )
  {
    matricesP[index] = iter_cam->camera._P;
    points2D.col(index) = iter_cam->point;
  }
  openMVG::Vec4 point4D;
  // For 2D version use TriangulateDLT contained openMVG/multiview/triangulation.hpp 
  openMVG::TriangulateNView( points2D, matricesP, &point4D);
  point4D /= point4D[3];
  point3D = openMVG::Vec3( point4D[0], point4D[1], point4D[2]);

  return true;
}

}// End namespace geometry

#endif // N_VIEW_2D_POINT_TO_3D_HPP
