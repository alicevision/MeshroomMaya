file(
  GLOB_RECURSE
  geometry_files_hpp
  *.hpp
)
file(
  GLOB_RECURSE
  geometry_files_cpp
  *.cpp
)
file(GLOB_RECURSE REMOVEFILESUNITTEST *_test.cpp)

#Remove the unit test files (not been used by the library)
list(REMOVE_ITEM geometry_files_cpp ${REMOVEFILESUNITTEST})

# ==============================================================================
# OpenMP detection
# ==============================================================================
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  OPTION(USE_OPENMP "Use OpenMP for parallelization" ON)
  ADD_DEFINITIONS(-DUSE_OPENMP)
  IF (NOT MSVC)
    LIST(APPEND OPENMVG_LIBRARY_DEPENDENCIES gomp)
  ENDIF (NOT MSVC)
ELSE (OPENMP_FOUND)
  OPTION(USE_OPENMP "Use OpenMP for parallelization" OFF)
ENDIF (OPENMP_FOUND)

ADD_EXECUTABLE(openMVG_UI_test_oneView2DpolygonTo3D oneView2DpolygonTo3D_test.cpp)
TARGET_LINK_LIBRARIES(openMVG_UI_test_oneView2DpolygonTo3D CppUnitLite openMVG_numeric openMVG_multiview )
ADD_TEST(openMVG_UI_test_oneView2DpolygonTo3D openMVG_UI_test_oneView2DpolygonTo3D)
