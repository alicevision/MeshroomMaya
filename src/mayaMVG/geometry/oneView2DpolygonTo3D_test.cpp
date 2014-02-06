#include "oneView2DpolygonTo3D.hpp"

#include <openMVG/cameras/PinholeCamera.hpp>
#include <testing/testing.h>

/**
 * Test the selection points inside a face
 */
TEST(Geometry, ComputeEquationPlan)
{
  // Create a camera
  openMVG::Mat34 matCam;
  matCam << 750.362775651,  -1.60282746822,  3082.76089044,   -7351.33249805, 
            948.31313828,   -2702.81737771,  -742.443254571,  -3083.56123142, 
            0.733406890622, 0.0643543829197, -0.676736910614, -2.46666989573;
  openMVG::PinholeCamera camera(matCam);
    
  
  // Create a face in front of the camera
  openMVG::Vec3 point1( 2.33066, 0.929558, 4.93463 );
  openMVG::Vec3 point2( 2.29588, -1.86543, 5.08264 ); 
  openMVG::Vec3 point3( 1.28318, -2.12238, 5.07092 ); 
  openMVG::Vec3 point4( 1.23894, 0.936656, 4.90687 ); 
  
  std::vector<openMVG::Vec3> vec_initialFacePoint3D;
  vec_initialFacePoint3D.push_back( point1 );
  vec_initialFacePoint3D.push_back( point2 );
  vec_initialFacePoint3D.push_back( point3 );
  vec_initialFacePoint3D.push_back( point4 );
  
  
  // Create 3D points on the plane of the face inside and outside of the face.
  openMVG::Vec3 vec1_2 = point2 - point1;
  openMVG::Vec3 vec1_4 = point4 - point1;
  
  std::vector<openMVG::Vec3> vec_point3D_all;
  for ( std::vector<openMVG::Vec3>::const_iterator iterPoint = vec_initialFacePoint3D.begin();
          iterPoint != vec_initialFacePoint3D.end();
          iterPoint++ )
  {
    vec_point3D_all.push_back( *iterPoint + vec1_2 / 4 + vec1_4 / 4 );
    vec_point3D_all.push_back( *iterPoint - vec1_2 / 4 + vec1_4 / 4 );
    vec_point3D_all.push_back( *iterPoint - vec1_2 / 4 - vec1_4 / 4 );
    vec_point3D_all.push_back( *iterPoint + vec1_2 / 4 - vec1_4 / 4 );
  }
  vec_point3D_all.push_back( point1 + vec1_2 / 2 + vec1_4 / 2 );  
  
  
  // Project the face and the 3D points on the image plane's camera
  std::vector<openMVG::Vec2> vec_facePoints2D;
  for ( std::vector<openMVG::Vec3>::const_iterator iterPoint = vec_initialFacePoint3D.begin();
          iterPoint != vec_initialFacePoint3D.end();
          iterPoint++ )
  {
    vec_facePoints2D.push_back( camera.Project( *iterPoint ) );
  }
  
  
  // Get 2D points inside the face
  std::vector<openMVG::Vec3> vec_insidePoints;
  geometry::getInsidePoints( vec_facePoints2D,
                         vec_point3D_all,
                         camera, 
                         vec_insidePoints );
  
  EXPECT_EQ( 5, vec_insidePoints.size() );
  
  
  // Compute the plane equation
  std::vector<openMVG::Vec3> vec_facePoints3D;
  geometry::computePlanEquation( vec_facePoints2D,
                             vec_insidePoints,
                             camera,
                             vec_facePoints3D );
  
  EXPECT_EQ( 4, vec_facePoints3D.size() );
  
  
  // Compare the finded 3D points with the initial 3D points  
  std::vector<openMVG::Vec3>::const_iterator iterPointInitial = vec_initialFacePoint3D.begin(); 
  for ( std::vector<openMVG::Vec3>::const_iterator iterPoint = vec_facePoints3D.begin();
          iterPoint != vec_facePoints3D.end();
          iterPoint++,
          iterPointInitial++
      )
  {
    openMVG::Vec3 initialPoint = *iterPointInitial;
    openMVG::Vec3 finalPoint = *iterPoint;     
    EXPECT_MATRIX_NEAR( initialPoint, finalPoint, 0.001  );
  }
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */ 
