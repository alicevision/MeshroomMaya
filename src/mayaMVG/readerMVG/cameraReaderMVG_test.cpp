#include "cameraReaderMVG.hpp"

//#include <maya/MObject.h>

#include <testing/testing.h>
#include <stdio.h>

/**
 * Write openMVG camera into binary file
 * Read binary file and convert to openMVG camera
 */
TEST(conversion, WriteAndReadCameraOpenMVG)
{
  openMVG::Mat34 matCam;
  matCam << 750.362775651,  -1.60282746822,  3082.76089044,   -7351.33249805, 
            948.31313828,   -2702.81737771,  -742.443254571,  -3083.56123142, 
            0.733406890622, 0.0643543829197, -0.676736910614, -2.46666989573;
  openMVG::PinholeCamera camera( matCam );
  
  const std::string sPathCameraBin = "bin.bin";
  // Write P matrix into a binary file
  EXPECT_TRUE( readerMVG::cameraOpenMVGtoBinaryFile( sPathCameraBin, camera ) );
  
  // Read P matrix from a binary file
  openMVG::PinholeCamera outCamera;
  EXPECT_TRUE( readerMVG::binaryFiletoCameraOpenMVG( sPathCameraBin, outCamera ) );
  
  EXPECT_MATRIX_NEAR( camera._P, outCamera._P, 0.001  );
  
  std::remove( sPathCameraBin.c_str() );
}



/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */ 
 
