#pragma once
#include <maya/MPoint.h>
#include <openMVG/numeric/numeric.h>
#include <openMVG/robust_estimation/robust_estimator_LMeds.hpp>

namespace mayaMVG
{

// Should be part of OpenMVG
struct PlaneKernel
{
    typedef openMVG::Vec4 Model;
    enum
    {
        MINIMUM_SAMPLES = 3
    };
    PlaneKernel(const openMVG::Mat& pt)
        : _pt(pt)
    {
    }
    size_t NumSamples() const { return _pt.cols(); }
    void Fit(const std::vector<size_t>& samples, std::vector<Model>* equation) const
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
        // Compute the plane coefficients from the 3 given points in a
        // straightforward manner
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
    double Error(size_t sample, const Model& model) const
    {
        // Calculate the distance from the point to the plane normal as the dot
        // product
        // D = (P-A).N/|N|
        openMVG::Vec3 pt3 = _pt.col(sample);
        openMVG::Vec4 pt4(pt3(0), pt3(1), pt3(2), 1.0);
        return fabs(model.dot(pt4));
    }
    const openMVG::Mat& _pt;
};

// FIXME check return value
static bool plane_line_intersect(const PlaneKernel::Model& model, const MPoint& P1,
                                 const MPoint& P2, MPoint& P)
{
    double u = (model(0) * P1.x + model(1) * P1.y + model(2) * P1.z + model(3)) /
               (model(0) * (P1.x - P2.x) + model(1) * (P1.y - P2.y) + model(2) * (P1.z - P2.z));
    P = P1 + u * (P2 - P1);
    return (0 < u && u < 1);
}

} // namespace
