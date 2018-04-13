#cmake_minimum_required(VERSION 3.3)

project(mayaMVG)

set(PLUGIN_VERSION_MAJOR 1)
set(PLUGIN_VERSION_MINOR 0)

#
# Compiler settings
#
if(UNIX)
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_COMPILER g++)
endif()

#
# Project Search Paths
#
include_directories(${PROJECT_SOURCE_DIR}/..)

#
# Maya dependency
#
find_package(Maya REQUIRED)

#
# Qt dependency
#
find_package(Qt5 ${MAYA_QT_VERSION_SHORT} COMPONENTS Core Widgets Quick QuickWidgets REQUIRED)

#
# AliceVision dependency
#
find_package(AliceVision REQUIRED)

find_package(Boost REQUIRED)
find_package(Ceres REQUIRED)

#
# OpenGL dependency
#
find_package(OpenGL REQUIRED)

#
# Plugin name
#
set(project_BIN	${PROJECT_NAME})

#
# Plugin sources
#
file(GLOB_RECURSE PLUGIN_SRCS
	*.cpp *.cxx *.cc *.C *.c *.h *.hpp)

#
# Qt MOC
#
set(MOC_HDRS
	qt/MVGMainWidget.hpp
	qt/MVGCameraWrapper.hpp
	qt/MVGMeshWrapper.hpp
	qt/MVGProjectWrapper.hpp
	qt/MVGPanelWrapper.hpp
	qt/MVGCameraSetWrapper.hpp
	qt/QmlInstantCoding.hpp
	qt/QObjectListModel.hpp
)
qt5_wrap_cpp(MOC_SRCS ${MOC_HDRS})

# Qml sources
#
file(GLOB_RECURSE QML_SRCS *.qml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
# Maya plugin properties
#
add_library(${project_BIN} SHARED
    ${PLUGIN_SRCS}
    ${MOC_SRCS}
    ${QML_SRCS}
)
target_include_directories(${project_BIN} PUBLIC
    ${MAYA_INCLUDE_DIR}
    ${ALICEVISION_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(${project_BIN} PUBLIC
    ${MAYA_Foundation_LIBRARY}
    ${MAYA_OpenMaya_LIBRARY}
    ${MAYA_OpenMayaUI_LIBRARY}
    ${MAYA_OpenMayaFX_LIBRARY}
    ${MAYA_OpenMayaRender_LIBRARY}
    ${ALICEVISION_LIBRARY}
    aliceVision_system
    aliceVision_numeric
    aliceVision_multiview
    aliceVision_image
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

qt5_use_modules(${project_BIN} Core Widgets Quick QuickWidgets)
maya_set_plugin_properties(${project_BIN})

#
# Install settings
#
### install path
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${MAINFOLDER}/dist/plugin CACHE STRING "Plugin install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

### plugin file
install(TARGETS ${project_BIN}
		DESTINATION "${CMAKE_INSTALL_PREFIX}/plug-ins")

### .mod file
install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.mod
\"+ ${PROJECT_NAME} ${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR} .\nPYTHONPATH+:=scripts\")")

### scripts
install(DIRECTORY "${PROJECT_SOURCE_DIR}/maya/scripts"
		DESTINATION "${CMAKE_INSTALL_PREFIX}")

### .qml files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/qt/qml/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/qml/mvg")

INSTALL(CODE "message(
		\"---\n QtQuick Desktop Components need to be copied in:\n '${CMAKE_INSTALL_PREFIX}/qml/QtDesktop'\n---\")")
