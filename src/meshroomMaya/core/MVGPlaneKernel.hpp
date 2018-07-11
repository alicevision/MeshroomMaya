#pragma once

#include "MVGEigen.hpp"
#include <maya/MPoint.h>

namespace meshroomMaya {

// Should be part of aliceVision
struct PlaneKernel
{
    typedef aliceVision::Vec4 Model;
    enum
    {
        MINIMUM_SAMPLES = 3
    };
    PlaneKernel(const aliceVision::Mat& pt)
        : _pt(pt)
    {
    }
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
};

// FIXME check return value
inline bool plane_line_intersect(const PlaneKernel::Model& model, const MPoint& P1,
                                 const MPoint& P2, MPoint& P)
{
    double u = (model(0) * P1.x + model(1) * P1.y + model(2) * P1.z + model(3)) /
               (model(0) * (P1.x - P2.x) + model(1) * (P1.y - P2.y) + model(2) * (P1.z - P2.z));
    P = P1 + u * (P2 - P1);
    return (0 < u && u < 1);
}

} // namespace
