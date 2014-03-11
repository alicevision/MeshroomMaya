#pragma once

#include "mayaMVG/core/MVGCamera.h"

namespace mayaMVG {

struct MVGCameraReader {
	
	// static bool binaryFiletoCameraOpenMVG(const std::string&, openMVG::PinholeCamera&);
	// static bool cameraOpenMVGtoBinaryFile(const std::string&, const openMVG::PinholeCamera&);
	static bool read(std::vector<MVGCamera>&);

};

} // mayaMVG
