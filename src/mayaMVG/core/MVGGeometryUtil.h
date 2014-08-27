#pragma once

#include "openMVG/numeric/numeric.h"
#include <maya/MVector.h>

class MPoint;
class MPointArray;
class M3dView;

namespace mayaMVG {

#define AS_MPOINT(a)\
	MPoint(a(0), a(1), a(2))

#define AS_VEC3(a)\
	openMVG::Vec3(a.x, a.y, a.z)

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
    // 
	static MPoint worldToView(M3dView&, const MPoint&);
	static MPoint viewToWorld(M3dView&, const MPoint&);
	static void viewToCamera(M3dView& view, const short x, const short y, MPoint& point);
	static void worldToCamera(M3dView& view, const MPoint& worldPoint, MPoint& point);
	static void cameraToView(M3dView& view, const MPoint& point, MPoint& viewPoint);
	static void cameraToImage(const MVGCamera& camera, const MPoint& point, MPoint& image);

    //
	static bool projectFace2D(M3dView& view, MPointArray& face3DPoints, const MVGCamera& camera, const MPointArray& face2DPoints, MVector height = MVector(0, 0, 0));
	static bool computePlane(MPointArray& facePoints3D, PlaneKernel::Model& model);
	static void projectPointOnPlane(const MPoint& point, M3dView& view, PlaneKernel::Model& model, const MVGCamera&,  MPoint& projectedPoint);
	static void triangulatePoint(MPointArray& points2D, std::vector<MVGCamera>& cameras, MPoint& resultPoint3D);

    // math
    static double crossProduct2D(MVector& A, MVector& B);
	static double dotProduct2D(MVector& A, MVector& B);
	static bool edgesIntersection(MPoint A, MPoint B, MVector AD,  MVector BC);
};

} // namespace
