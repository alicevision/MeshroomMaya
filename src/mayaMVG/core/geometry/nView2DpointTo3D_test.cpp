#include "nView2DpointTo3D.hpp"

#include <openMVG/cameras/PinholeCamera.hpp>
#include <testing/testing.h>


/**
 * Test the triangulation of a point see in 3 images
 */
TEST(Geometry, TriangulationPosition)
{
  // Create 3 cameras turn on the same direction
  openMVG::Mat34 matCam1;
  matCam1 << 750.362775651,  -1.60282746822,  3082.76089044,   -7351.33249805, 
             948.31313828,   -2702.81737771,  -742.443254571,  -3083.56123142, 
             0.733406890622, 0.0643543829197, -0.676736910614, -2.46666989573;
  openMVG::PinholeCamera camera1(matCam1);
  
  openMVG::Mat34 matCam2;
  matCam2 << 1211.81300014,  -56.9839339726,  2930.29958579,   -6710.35158605, 
             822.243959723,  -2684.87191452,  -929.005517655,  -2268.47903427, 
             0.621326136337, 0.0786371632721, -0.779596067753, -1.76464751466;
  openMVG::PinholeCamera camera2(matCam2);
  
  openMVG::Mat34 matCam3;
  matCam3 << 1519.64289671,  38.1867006164,   2779.33139895,   -5950.5410189, 
             711.562921097,  -2715.93808501,  -917.656967798,  -1557.5363006, 
             0.533011702147, 0.0480334864519, -0.844743339455, -1.16768448874;
  openMVG::PinholeCamera camera3( matCam3 );
  
  
  // Create a 3D point 
  openMVG::Vec3 initial3Dpoint;
  initial3Dpoint << -3.19277, -0.869204, 4.84992;
  
  // Project the point on the image plane's cameras
  openMVG::Vec2 pointProjCam1 = camera1.Project(initial3Dpoint);
  openMVG::Vec2 pointProjCam2 = camera2.Project(initial3Dpoint);
  openMVG::Vec2 pointProjCam3 = camera3.Project(initial3Dpoint);
  
  // Triangulate the point from the 3 cameras
  std::vector<geometry::PositionByCam> vec_position2dByCamera;

  //map_position2dByCamera.insert( std::pair<openMVG::Vec2,openMVG::PinholeCamera>( pointProjCam1, camera1 ) );
  vec_position2dByCamera.push_back( geometry::PositionByCam( pointProjCam1, camera1 ) );
  vec_position2dByCamera.push_back( geometry::PositionByCam( pointProjCam2, camera2 ) );
  vec_position2dByCamera.push_back( geometry::PositionByCam( pointProjCam3, camera3 ) );
  
  openMVG::Vec3 outPoint3D;
  geometry::triangulatePosition( vec_position2dByCamera, outPoint3D );
  
  // Compare initial 3D point
  EXPECT_MATRIX_NEAR( initial3Dpoint, outPoint3D, 0.001 );
}


/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */ 
