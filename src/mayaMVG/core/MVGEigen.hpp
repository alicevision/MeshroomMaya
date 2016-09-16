#pragma once
// /!\ Include this file instead of <openMVG/numeric/numeric.h>
// /!\ or before any include of OpenMVG or Eigen

// X11 defines CamelCase macros that clash with some of Eigen's enums.
// To avoid include-order based issues, we undefine those problematic names
// see : http://eigen.tuxfamily.org/bz/show_bug.cgi?id=253
#if defined(LINUX)

#if defined(Success)
#undef Success
#endif

#endif //LINUX

// openMVG includes Eigen
#include <openMVG/numeric/numeric.h>