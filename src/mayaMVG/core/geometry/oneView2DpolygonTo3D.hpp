#ifndef ONE_VIEW_2D_POLYGON_TO_3D_HPP
#define ONE_VIEW_2D_POLYGON_TO_3D_HPP

#include "plane_kernel.hpp"

// openMVG includes
#include <openMVG/robust_estimation/robust_estimator_LMeds.hpp>
#include <openMVG/cameras/PinholeCamera.hpp>
#include <openMVG/numeric/numeric.h>

// std includes
#include <vector>
#include <stdlib.h> 
#include <algorithm>

namespace geometry
{

static inline bool fuzzyCompare(float p1, float p2)
{
  return (abs(p1 - p2) * 100000.f <= std::min(abs(p1), abs(p2)));
}

/**
 * 
 */
static void polygonIntersectionLines( const openMVG::Vec2 &p1, 
                                   const openMVG::Vec2 &p2, 
                                   const openMVG::Vec2 &pos,
                                   int *winding)
{
  float x1 = p1[0];
  float y1 = p1[1];
  float x2 = p2[0];
  float y2 = p2[1];
  float y = pos[1];
  int dir = 1;
  if (fuzzyCompare(y1, y2)) 
  {
    // ignore horizontal lines according to scan conversion rule
    return;
  } 
  else if (y2 < y1) 
  {
    float x_tmp = x2; x2 = x1; x1 = x_tmp;
    float y_tmp = y2; y2 = y1; y1 = y_tmp;
    dir = -1;
  }
  if (y >= y1 && y < y2) 
  {
    float x = x1 + ((x2 - x1) / (y2 - y1)) * (y - y1);
    // count up the winding number if we're
    if (x<=pos.x()) 
    {   
      (*winding) += dir;
    }
  }
}

/**
 * Return true if the point is inside the Face
 * \param[in] vec_polygon points of the 2D face
 * \param[in] point 2D point
 * \return true if the point is inside
 */
bool isPointInsideQuad( const std::vector<openMVG::Vec2>& vec_polygon,
                        const openMVG::Vec2 &point)
{
  if (vec_polygon.size() == 0)
    return false;
  
  int winding_number = 0;
  openMVG::Vec2 last_pt = vec_polygon.at(0);
  openMVG::Vec2 last_start = vec_polygon.at(0);
  for (int i = 1; i < vec_polygon.size(); ++i) {
    const openMVG::Vec2 &e = vec_polygon.at(i);
    polygonIntersectionLines(last_pt, e, point, &winding_number);
    last_pt = e;
  }
  // implicitly close last subpath
  if (last_pt != last_start)
    polygonIntersectionLines(last_pt, last_start, point, &winding_number);
  
  return (winding_number != 0);
}

/**
 * Get all 3D points inside the quad.
 * \param[in] vec_facePoints2D points of the 2D face
 * \param[in] vec_allPoints all 3D points of the scene
 * \param[in] camera current camera
 * \param[out] vec_insidePoints 3D points inside 2D face
 * \return true if plane is computed
 */
void getQuadInsidePoints( const std::vector<openMVG::Vec2>& vec_facePoints2D,
                          const std::vector<openMVG::Vec3>& vec_allPoints,
                          const openMVG::PinholeCamera& camera, 
                          std::vector<openMVG::Vec3>& vec_insidePoints )
{ 
  for (std::vector<openMVG::Vec3>::const_iterator iterPoint3D = vec_allPoints.begin();
            iterPoint3D != vec_allPoints.end(); ++iterPoint3D)
  {
    openMVG::Vec2 point2D = camera.Project(*iterPoint3D);
    if ( isPointInsideQuad( vec_facePoints2D, point2D ) )
    {
      vec_insidePoints.push_back( *iterPoint3D );
    }
  }
}

/**
 * Get all 3D points inside the face.
 * \param[in] vec_facePoints2D points of the 2D face
 * \param[in] vec_allPoints all 3D points of the scene
 * \param[in] camera current camera
 * \param[out] vec_insidePoints 3D points inside 2D face
 * \return true if plane is computed
 */
void getInsidePoints( const std::vector<openMVG::Vec2>& vec_facePoints2D,
                      const std::vector<openMVG::Vec3>& vec_allPoints,
                      const openMVG::PinholeCamera& camera, 
                      std::vector<openMVG::Vec3>& vec_insidePoints )
{
  if ( vec_facePoints2D.size() == 4 )
  {
    getQuadInsidePoints( vec_facePoints2D, vec_allPoints, camera, vec_insidePoints );
  }
  else
  {
    std::cout << "Only quad are manage" << std::endl;
  }
}

/**
 * Compute Plan equation from 3D points inside the face
 * \param[in] vec_facePoints2D points of the 2D face
 * \param[in] vec_insidePoints 3D points inside the 2D face
 * \param[in] camera current camera
 * \param[out] vec_facePoints3D points of the 3D face
 * \return true if plane is computed
 */
bool computePlanEquation( const std::vector<openMVG::Vec2>& vec_facePoints2D,
                          const std::vector<openMVG::Vec3>& vec_insidePoints,
                          const openMVG::PinholeCamera& camera,
                          std::vector<openMVG::Vec3>& vec_facePoints3D )
{ 
  // To compute plan equation we need at leat 3 3D points.
  if ( vec_insidePoints.size() < 3 )
  {
    std::cout << "There is not enough points inside the face" << std::endl;
    return false;
  }
  
  openMVG::Mat points( 3, vec_insidePoints.size() );
  size_t cpt = 0;
  for (std::vector<openMVG::Vec3>::const_iterator iterI = vec_insidePoints.begin();
          iterI != vec_insidePoints.end();
          ++iterI, ++cpt)
  {
    points.col(cpt) = *iterI;
  }
  
  //-- RANSAC - 3D plane estimation
  PlaneKernel pk(points);
  std::vector<size_t> vec_inliers;

  openMVG::Vec4 planeLMeds;
  double dThresholdLMeds;
  openMVG::robust::LeastMedianOfSquares(pk, &planeLMeds, &dThresholdLMeds);
  std::cout << "estimatedPlaneLMeds" << planeLMeds.transpose() << std::endl;
  
  
  //Compute the polygon point over the estimated plane
  openMVG::Vec3 P1, P2, P;
  P1 = camera._C;


  for ( std::vector<openMVG::Vec2>::const_iterator iter_bound = vec_facePoints2D.begin();
          iter_bound != vec_facePoints2D.end();
          iter_bound++ )
  {
    P2 = camera._R.transpose() *
        ( camera._K.inverse() *
          openMVG::Vec3( (*iter_bound)(0), (*iter_bound)(1), 1.0 ) );
    P2 = P1 + P2;

    linePlaneIntersection( planeLMeds, P1, P2, P );
    vec_facePoints3D.push_back( P );
  }
  
  return true;
}

}// End namespace geometry

#endif // ONE_VIEW_2D_POLYGON_TO_3D_HPP
