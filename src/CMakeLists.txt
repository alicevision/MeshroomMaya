
cmake_minimum_required(VERSION 2.6.2)
project(openMVG-UI)

#
# Project Search Paths
#
set(MAINFOLDER "${PROJECT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH "${MAINFOLDER}/cmake")

#
# Project Output Paths
#
set(LIBRARY_OUTPUT_PATH "${MAINFOLDER}/lib")


include_directories(openMVG/src/
openMVG/src/third_party/eigen)

add_subdirectory(openMVG/src/)

#
# Add Build Targets
#
add_subdirectory(mayaMVG)