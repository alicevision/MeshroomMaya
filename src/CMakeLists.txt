
cmake_minimum_required(VERSION 2.6.2)
project(openMVG-UI)


#
# Compiler settings
#
if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

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
