#pragma once

#include <maya/MDagPath.h>
#include <vector>

class MPoint;
class MString;

namespace mayaMVG {

class MVGCamera {

	public:
		enum STEP {
			STEP_NEW = 0,
			STEP_WIP,
			STEP_DEF,
			STEP_DISABLED
		};

	public:
		MVGCamera(const MDagPath& path);
		virtual ~MVGCamera();

	public:
		void add2DPoint(const MPoint&);
		void move2DPoint(const MPoint&);
		void setImagePlane(const MString& name);
		void setZoom(float z);
		void setPan(float x, float y);

	private:
		MDagPath _dagpath;
		STEP _step;
		std::vector<std::pair<MPoint, size_t> > _points;
		// list<MVGPoint2D> cachePointCloudProjection
		// list<MVGPoint2D> cacheMeshProjection

};

} // mayaMVG
