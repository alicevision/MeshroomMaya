cmake_minimum_required(VERSION 3.3)

project(pluginMayaMVG)

#
# Plugin version
#

set(PLUGIN_VERSION_MAJOR 1)
set(PLUGIN_VERSION_MINOR 0)

#
# Compiler settings
#

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_C_COMPILER gcc)
endif()

#
# Project Search Paths
#

set(MAINFOLDER "${PROJECT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH "${MAINFOLDER}/cmake")
include_directories(${PROJECT_SOURCE_DIR})

#
# Dependencies
#

# Maya dependency
find_package(Maya REQUIRED)

# Qt dependency
find_package(Qt5 ${MAYA_QT_VERSION_SHORT} COMPONENTS Core Widgets Quick QuickWidgets REQUIRED)
set(CMAKE_AUTOMOC ON) # Instruct CMake to run moc automatically when needed.

# AliceVision dependency
find_package(AliceVision REQUIRED)

# Boost dependency
find_package(Boost REQUIRED)

# Ceres dependency
find_package(Ceres REQUIRED)

# OpenGL dependency
find_package(OpenGL REQUIRED)

#
# Add sources
#

add_subdirectory(mayaMVG)
