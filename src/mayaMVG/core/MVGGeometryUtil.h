#pragma once

#include "openMVG/numeric/numeric.h"
#include <maya/MPoint.h>
#include <maya/M3dView.h>
#include <vector>
#include <algorithm>

namespace mayaMVG {

#define AS_MPOINT(a)\
	MPoint(a(0), a(1), a(2))

#define AS_VEC3(a)\
	openMVG::Vec3(a.x, a.y, a.z)


struct MVGFace2D {
	MVGFace2D(){}
	MVGFace2D(MPoint a, MPoint b, MPoint c, MPoint d) {
		_p[0] = a; _p[1] = b; _p[2] = c; _p[3] = d;
	}
	MVGFace2D(std::vector<MPoint> p) {
		for(size_t i = 0; i<std::min((size_t)4, p.size()); ++i)
			_p[i] = p[i];
	}
	MPoint _p[4];
};

struct MVGFace3D {
	MVGFace3D(){}
	MVGFace3D(MPoint a, MPoint b, MPoint c, MPoint d) {
		_p[0] = a; _p[1] = b; _p[2] = c; _p[3] = d;
	}
	MVGFace3D(std::vector<MPoint> p) {
		for(size_t i = 0; i<std::min((size_t)4, p.size()); ++i)
			_p[i] = p[i];
	}
	MPoint _p[4];
};

// Should be part of OpenMVG
struct PlaneKernel
{
	typedef openMVG::Vec4 Model;
	enum { MINIMUM_SAMPLES = 3 };
	PlaneKernel(const openMVG::Mat & pt)
		: _pt(pt) {}
	size_t NumSamples() const {
		return _pt.cols();
	}
	/**
	 * 
     * @param samples
     * @param equation
     */
	void Fit(const std::vector<size_t> &samples, std::vector<Model> * equation) const
	{
		assert(samples.size() >= MINIMUM_SAMPLES);
		equation->clear();
		openMVG::Mat sampled_xs = openMVG::ExtractColumns(_pt, samples);
		openMVG::Vec3 p0 = sampled_xs.col(0);
		openMVG::Vec3 p1 = sampled_xs.col(1);
		openMVG::Vec3 p2 = sampled_xs.col(2);
		// Compute the segment values (in 3d) between p1 and p0
		openMVG::Vec3 p1p0 = p1 - p0;
		// Compute the segment values (in 3d) between p2 and p0
		openMVG::Vec3 p2p0 = p2 - p0;
		// Avoid some crashes by checking for collinearity here
		openMVG::Vec3 dy1dy2 = p1p0.array() / p2p0.array();
		if((dy1dy2[0] == dy1dy2[1]) && (dy1dy2[2] == dy1dy2[1])) // Check for collinearity
			return;
		// Compute the plane coefficients from the 3 given points in a straightforward manner
		// calculate the plane normal n = (p2-p1) x (p3-p1) = cross(p2-p1, p3-p1)
		Model m;
		m[0] = p1p0[1] * p2p0[2] - p1p0[2] * p2p0[1];
		m[1] = p1p0[2] * p2p0[0] - p1p0[0] * p2p0[2];
		m[2] = p1p0[0] * p2p0[1] - p1p0[1] * p2p0[0];
		m[3] = 0.0;
		m.normalize();
		m[3] = -1.0 * (m.head<3>().dot(p0));
		equation->push_back(m);
	}
	
	double Error(size_t sample, const Model & model) const
	{
		// Calculate the distance from the point to the plane normal as the dot product
		// D = (P-A).N/|N|
		openMVG::Vec3 pt3 = _pt.col(sample);
		openMVG::Vec4 pt4(pt3(0), pt3(1), pt3(2), 1.0);
		return fabs(model.dot(pt4));
	}
	const openMVG::Mat & _pt;
};

class MVGPointCloud;
class MVGCamera;

struct MVGGeometryUtil {
	static MPoint worldToView(M3dView&, const MPoint&);
	static MPoint viewToWorld(M3dView&, const MPoint&);
	static bool projectFace2D(MVGFace3D&, M3dView&, MVGCamera&, MVGFace2D&, bool compute = false, MVector height = MVector(0, 0, 0));
	static void computePlane(MVGFace3D& face3D, PlaneKernel::Model& model);
	static void projectPointOnPlane(MPoint& point, PlaneKernel::Model& model, MVGCamera&,  MPoint& projectedPoint);
};



} // mayaMVG
