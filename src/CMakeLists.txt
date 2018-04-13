cmake_minimum_required(VERSION 2.6.2)
project(mayaMVG)

#
# Compiler settings
#
if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
endif()

#
# Project Search Paths
#
set(MAINFOLDER "${PROJECT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH "${MAINFOLDER}/cmake")

#
# Add Build Targets
#
add_subdirectory(mayaMVG)
