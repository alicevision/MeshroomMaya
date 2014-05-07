
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
# Add Build Targets
#
add_subdirectory(openMVG/src EXCLUDE_FROM_ALL)
add_subdirectory(mayaMVG)
