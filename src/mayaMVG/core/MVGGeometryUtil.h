#pragma once

#include <maya/MPoint.h>

namespace mayaMVG {
	
struct MVGFace2D {
	MPoint _a;
	MPoint _b;
	MPoint _c;
	MPoint _d;
};

struct MVGFace3D {
	MPoint _a;
	MPoint _b;
	MPoint _c;
	MPoint _d;
};

class MVGPointCloud;
class MVGCamera;

struct MVGGeometryUtil {
	static bool projectFace2D(MVGFace3D&, MVGPointCloud&, MVGCamera&, MVGFace2D&);
};

} // mayaMVG
