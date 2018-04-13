#pragma once

#include "MVGEigen.hpp"
#include <maya/MPoint.h>

namespace mayaMVG
{

// Should be part of aliceVision
struct LineConstrainedPlaneKernel
{
    typedef aliceVision::Vec4 Model;
    enum
    {
        MINIMUM_SAMPLES = 1
    };
    LineConstrainedPlaneKernel(const aliceVision::Mat& pt, const aliceVision::Vec3& constraintP0,
                               const aliceVision::Vec3& constraintP1);
    size_t NumSamples() const { return _pt.cols(); }
    void Fit(const std::vector<size_t>& samples, std::vector<Model>* equation) const;
    inline double Error(size_t sample, const Model& model) const
    {
        // Calculate the distance from the point to the plane normal as the dot
        // product
        // D = (P-A).N/|N|
        aliceVision::Vec3 pt3 = _pt.col(sample);
        aliceVision::Vec4 pt4(pt3(0), pt3(1), pt3(2), 1.0);
        return fabs(model.dot(pt4));
    }
    const aliceVision::Mat& _pt;
    const aliceVision::Vec3& _constraintP0;
    const aliceVision::Vec3& _constraintP1;
    const aliceVision::Vec3 _P1P0;
};

} // namespace
