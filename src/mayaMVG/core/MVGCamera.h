#pragma once

#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <vector>
#include <openMVG/cameras/PinholeCamera.hpp>

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
		MVGCamera(const std::string&);
		virtual ~MVGCamera();

	public:
		void instantiate();

		void setName(const std::string&);
		const std::string& name() const;
		void setImageName(const std::string&);
		const std::string& imageName() const;
		void setPinholeCamera(const openMVG::PinholeCamera&);
		const openMVG::PinholeCamera& pinholeCamera() const;

		void add2DPoint(const MPoint&);
		void move2DPoint(const MPoint&);
		void setZoom(float z);
		void setPan(float x, float y);

	private:
		std::string _name;
		std::string _imageName;
		openMVG::PinholeCamera _pinhole;
		STEP _step;
		MDagPath _dagpath;
		// std::vector<std::pair<MPoint, size_t> > _points;
		// list<MVGPoint2D> cachePointCloudProjection
		// list<MVGPoint2D> cacheMeshProjection

};

} // mayaMVG
