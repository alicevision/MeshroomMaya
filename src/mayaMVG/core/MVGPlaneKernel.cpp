#include "MVGPlaneKernel.hpp"

#include <aliceVision/robustEstimation/leastMedianOfSquares.hpp>


namespace mayaMVG
{

void PlaneKernel::Fit(const std::vector<size_t>& samples, std::vector<Model>* equation) const
{
    assert(samples.size() >= MINIMUM_SAMPLES);
    equation->clear();
    aliceVision::Mat sampled_xs = aliceVision::ExtractColumns(_pt, samples);
    aliceVision::Vec3 p0 = sampled_xs.col(0);
    aliceVision::Vec3 p1 = sampled_xs.col(1);
    aliceVision::Vec3 p2 = sampled_xs.col(2);
    // Compute the segment values (in 3d) between p1 and p0
    aliceVision::Vec3 p1p0 = p1 - p0;
    // Compute the segment values (in 3d) between p2 and p0
    aliceVision::Vec3 p2p0 = p2 - p0;
    // Avoid some crashes by checking for collinearity here
    aliceVision::Vec3 dy1dy2 = p1p0.array() / p2p0.array();
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


} // namespace
