#pragma once
#include <maya/MPoint.h>
#include <openMVG/numeric/numeric.h>
#include <openMVG/robust_estimation/robust_estimator_LMeds.hpp>

namespace mayaMVG
{

// Should be part of OpenMVG
struct LineConstrainedPlaneKernel
{
    typedef openMVG::Vec4 Model;
    enum
    {
        MINIMUM_SAMPLES = 1
    };
    LineConstrainedPlaneKernel(const openMVG::Mat& pt, const openMVG::Vec3& constraintP0,
                               const openMVG::Vec3& constraintP1)
        : _pt(pt)
        , _constraintP0(constraintP0)
        , _constraintP1(constraintP1)
        // Compute the segment values (in 3d) between p1 and p0
        , _P1P0(_constraintP1 - _constraintP0)
    {
    }
    size_t NumSamples() const { return _pt.cols(); }
    void Fit(const std::vector<size_t>& samples, std::vector<Model>* equation) const
    {
        assert(samples.size() >= MINIMUM_SAMPLES);
        equation->clear();
        openMVG::Mat sampled_xs = openMVG::ExtractColumns(_pt, samples);
        openMVG::Vec3 p2 = sampled_xs.col(0);
        // Compute the segment values (in 3d) between p2 and p0
        const openMVG::Vec3 p2p0 = p2 - _constraintP0;
        // Avoid some crashes by checking for collinearity here
        openMVG::Vec3 dy1dy2 = _P1P0.array() / p2p0.array();
        if((dy1dy2[0] == dy1dy2[1]) && (dy1dy2[2] == dy1dy2[1])) // Check for collinearity
            return;
        // Compute the plane coefficients from the 3 given points in a
        // straightforward manner
        // calculate the plane normal n = (p2-p1) x (p3-p1) = cross(p2-p1, p3-p1)
        Model m;
        m[0] = _P1P0[1] * p2p0[2] - _P1P0[2] * p2p0[1];
        m[1] = _P1P0[2] * p2p0[0] - _P1P0[0] * p2p0[2];
        m[2] = _P1P0[0] * p2p0[1] - _P1P0[1] * p2p0[0];
        m[3] = 0.0;
        m.normalize();
        // [ n' d ] dot [ p0 ; 1 ])
        // m[3] = -d with d the distance from the plane m, whose unit normal is n, to p0
        //        m[3] = -1.0 * (m.head<3>().dot(p0));
        m[3] = -1.0 * (m.head<3>().dot(_constraintP0));
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
    const openMVG::Vec3& _constraintP0;
    const openMVG::Vec3& _constraintP1;
    const openMVG::Vec3 _P1P0;
};

} // namespace
