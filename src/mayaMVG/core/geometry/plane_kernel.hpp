#ifndef PLANE_KERNEL_H
#define PLANE_KERNEL_H

#include <openMVG/numeric/numeric.h>

#include <iostream>

// The model coefficients are defined as:
// a : the X coordinate of the plane's normal (normalized)
// b : the Y coordinate of the plane's normal (normalized)
// c : the Z coordinate of the plane's normal (normalized)
// d : the fourth Hessian component of the plane's equation
// Kernel implementation with code based from PCL library
//http://docs.pointclouds.org/trunk/classpcl_1_1_sample_consensus_model_plane.html
//http://docs.pointclouds.org/trunk/sac__model__plane_8hpp_source.html
struct PlaneKernel
{
  PlaneKernel(const openMVG::Mat & pt) : _pt(pt) {}

  typedef openMVG::Vec4 Model;  // a, b, c, d;

  enum { MINIMUM_SAMPLES = 3 };

  size_t NumSamples() const {return _pt.cols();}
  void Fit(const std::vector<size_t> &samples, std::vector<Model> * equation) const  {
    equation->clear();
    // Need 3 samples
    if (samples.size () < 3)
    {
      std::cerr<<"\n[PlaneKernel::Fit] Invalid set of samples given" << std::endl;
      return ;
    }

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
    if ( (dy1dy2[0] == dy1dy2[1]) && (dy1dy2[2] == dy1dy2[1]) )          // Check for collinearity
      return ;

    // Compute the plane coefficients from the 3 given points in a straightforward manner
    // calculate the plane normal n = (p2-p1) x (p3-p1) = cross (p2-p1, p3-p1)
    openMVG::Vec4 model_coefficients;
    model_coefficients[0] = p1p0[1] * p2p0[2] - p1p0[2] * p2p0[1];
    model_coefficients[1] = p1p0[2] * p2p0[0] - p1p0[0] * p2p0[2];
    model_coefficients[2] = p1p0[0] * p2p0[1] - p1p0[1] * p2p0[0];
    model_coefficients[3] = 0.0;
    // Normalize
    model_coefficients.normalize();
    // ... + d = 0
    model_coefficients[3] = -1.0 * (model_coefficients.head<3>().dot(p0));
    equation->push_back(model_coefficients);
  }

  double Error(size_t sample, const Model & planeEq) const {
    // Calculate the distance from the point to the plane normal as the dot product
    // D = (P-A).N/|N|
    openMVG::Vec3 pt3 = _pt.col(sample);
    openMVG::Vec4 pt4(pt3(0), pt3(1), pt3(2), 1.0);
    return fabs(planeEq.dot(pt4));
  }

  const openMVG::Mat & _pt;
};

static bool linePlaneIntersection(const openMVG::Vec4 & plane, const openMVG::Vec3 & P1, const openMVG::Vec3 & P2, openMVG::Vec3 & P)
{
  double A,B,C,D;
  A = plane(0);
  B = plane(1);
  C = plane(2);
  D = plane(3);
  double x1,y1,z1,x2,y2,z2;
  x1 = P1(0);
  y1 = P1(1);
  z1 = P1(2);

  x2 = P2(0);
  y2 = P2(1);
  z2 = P2(2);

  double u = (A * x1 + B *y1 + C * z1 + D) / (A*(x1-x2) + B*(y1-y2) + C*(z1-z2));

  P = P1 + u * (P2-P1);

  if (0<u && u<1)
    return true;
  else
    return false;
}

#endif // PLANE_KERNEL_H
