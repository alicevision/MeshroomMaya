#include "MVGLineConstrainedPlaneKernel.hpp"

#include <aliceVision/robustEstimation/leastMedianOfSquares.hpp>

namespace meshroomMaya
{

LineConstrainedPlaneKernel::LineConstrainedPlaneKernel(const aliceVision::Mat& pt, const aliceVision::Vec3& constraintP0,
                           const aliceVision::Vec3& constraintP1)
    : _pt(pt)
    , _constraintP0(constraintP0)
    , _constraintP1(constraintP1)
    // Compute the segment values (in 3d) between p1 and p0
    , _P1P0(_constraintP1 - _constraintP0)
{
}
void LineConstrainedPlaneKernel::Fit(const std::vector<size_t>& samples, std::vector<Model>* equation) const
{
    assert(samples.size() >= MINIMUM_SAMPLES);
    equation->clear();
    aliceVision::Mat sampled_xs = aliceVision::ExtractColumns(_pt, samples);
    aliceVision::Vec3 p2 = sampled_xs.col(0);
    // Compute the segment values (in 3d) between p2 and p0
    const aliceVision::Vec3 p2p0 = p2 - _constraintP0;
    // Avoid some crashes by checking for collinearity here
    aliceVision::Vec3 dy1dy2 = _P1P0.array() / p2p0.array();
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

} // namespace
