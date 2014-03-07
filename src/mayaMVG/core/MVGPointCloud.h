#pragma once

#include <maya/MDagPath.h>

namespace mayaMVG {

class MVGPointCloud {

	public:
		MVGPointCloud();
		virtual ~MVGPointCloud();

	private:
		MDagPath _dagpath;
		
};

} // mayaMVG
