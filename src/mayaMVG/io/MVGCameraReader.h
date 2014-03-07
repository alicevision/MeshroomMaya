#pragma once

#include <openMVG/cameras/PinholeCamera.hpp>
// #include <openMVG/numeric/numeric.h>
// #include <third_party/stlplus3/filesystemSimplified/file_system.hpp>
#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

struct MVGCameraReader {
	
	// static bool binaryFiletoCameraOpenMVG(const std::string&, openMVG::PinholeCamera&);
	// static bool cameraOpenMVGtoBinaryFile(const std::string&, const openMVG::PinholeCamera&);
	static bool read(const std::string&, std::vector<CameraOpenMVG>&);

};

} // mayaMVG
