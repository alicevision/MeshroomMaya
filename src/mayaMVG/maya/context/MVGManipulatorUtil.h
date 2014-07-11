#pragma once

#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/M3dView.h>
#include <map>

namespace mayaMVG {

namespace { // empty namespace
	double crossProduct2D(MVector& A, MVector& B) {
		return A.x*B.y - A.y*B.x;
	}
} // empty namespace
	
struct MVGManipulatorUtil {
	
	#define POINT_RADIUS 10

	enum IntersectionState {
        eIntersectionNone = 0
        , eIntersectionPoint
        , eIntersectionEdge
    };
	
	struct IntersectionData {
		int	pointIndex;
		MIntArray edgePointIndexes;
		MVector edgeHeight3D;
		MVector edgeHeight2D;
		double edgeRatio;
		
	};
			
	static bool intersectPoint(M3dView& view, DisplayData* displayData, IntersectionData& intersectionData, const short&x, const short& y);
	static bool intersectEdge(M3dView& view, DisplayData* displayData, IntersectionData& intersectionData, const short&x, const short& y);
	
	static void drawIntersections(M3dView& view, DisplayData* data, IntersectionData& intersectionData, IntersectionState intersectionState);
	static void drawCameraPoints(M3dView& view, DisplayData* data);
};

} // mayaMVG
