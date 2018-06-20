#pragma once
// /!\ Include this file instead of <aliceVision/numeric/numeric.hpp>
// /!\ or before any include of aliceVision or Eigen

// X11 defines CamelCase macros that clash with some of Eigen's enums.
// To avoid include-order based issues, we undefine those problematic names
// see : http://eigen.tuxfamily.org/bz/show_bug.cgi?id=253
#if defined(LINUX)

#if defined(Success)
#undef Success
#endif

#endif //LINUX

// aliceVision includes Eigen
#include <aliceVision/numeric/numeric.hpp>
