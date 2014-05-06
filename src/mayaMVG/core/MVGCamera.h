#pragma once

#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <vector>
#include <openMVG/cameras/PinholeCamera.hpp>

class MString;
class MFnCamera;

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
		MVGCamera(const std::string& name);
		MVGCamera(const MDagPath& dagPath);
		virtual ~MVGCamera();

	public:
		static MVGCamera create(const std::string& name);

	public:
		const MDagPath& dagPath() const;
		void setDagPath(const MDagPath& dagpath);
		void setName(const std::string&);
		const std::string name() const;
		void setImagePlane(const std::string&);
		const std::string& imagePlane() const;
		void loadImagePlane(const std::string&) const;
		void setPinholeCamera(const openMVG::PinholeCamera&);
		const openMVG::PinholeCamera& pinholeCamera() const;
		// double zoom() const;
		// void setZoom(double z);
		// void setPan(float x, float y);
		void add2DPoint(const MPoint&);
		void move2DPoint(const MPoint&);
	
	public:
		void select() const;

	private:
		std::string _imageName;
		openMVG::PinholeCamera _pinhole;
		STEP _step;
		MDagPath _dagpath;
		MDagPath _dagpathImg;
		// std::vector<std::pair<MPoint, size_t> > _points;
		// list<MVGPoint2D> cachePointCloudProjection
		// list<MVGPoint2D> cacheMeshProjection

};

} // mayaMVG
