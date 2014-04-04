#pragma once

#include "mayaMVG/core/MVGCamera.h"

namespace mayaMVG {

struct MVGCameraReader {

	static bool read(std::vector<MVGCamera>&);

};

} // mayaMVG
