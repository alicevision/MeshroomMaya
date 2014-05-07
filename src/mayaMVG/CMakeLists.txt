
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
include_directories(${MAYA_INCLUDE_DIR})
link_directories(${MAYA_Foundation_LIBRARY} ${MAYA_OpenMaya_LIBRARY} ${MAYA_OpenMayaUI_LIBRARY} ${MAYA_OpenMayaFX_LIBRARY})

#
# Qt dependency 
#
find_package(Qt4 ${MAYA_QT_VERSION_SHORT} REQUIRED)
set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTDECLARATIVE TRUE)
include(${QT_USE_FILE})

#
# OpenMVG dependency 
#
include_directories(
			../openMVG/src
			../openMVG/src/third_party/eigen)

#
# Plugin name
#
set(project_BIN	${PROJECT_NAME}-${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR})

#
# Plugin sources
#
file(GLOB_RECURSE PLUGIN_SRCS
	*.cpp *.cxx *.cc *.C *.c *.h *.hpp)

#
# Qt MOC
#
set(MOC_HDRS
	qt/MVGMainWidget.h
	qt/MVGCameraWrapper.h
	qt/MVGProjectWrapper.h
	qt/QmlInstantCoding.h
)
qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
# Maya plugin properties
#
add_library(${project_BIN} SHARED 
				${PLUGIN_SRCS}
				${MOC_SRCS}
				../openMVG/src/software/SfMViewer/Ply.cpp)
target_link_libraries(
				${project_BIN}
				${MAYA_Foundation_LIBRARY}
				${MAYA_OpenMaya_LIBRARY}
				${MAYA_OpenMayaUI_LIBRARY}
				${MAYA_OpenMayaFX_LIBRARY}
				stlplus
				openMVG_numeric
				openMVG_multiview
				${QT_LIBRARIES})
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
		\"+ ${PROJECT_NAME} ${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR} .\")")

### scripts
install(DIRECTORY "${PROJECT_SOURCE_DIR}/maya/scripts"
		DESTINATION "${CMAKE_INSTALL_PREFIX}")

### .qml files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/qt/qml/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/qml/mvg")

INSTALL(CODE "message(
		\"---\n QtQuick Desktop Components need to be copied in:\n '${CMAKE_INSTALL_PREFIX}/qml/QtDesktop'\n---\")")
