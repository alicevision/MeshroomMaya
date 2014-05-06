
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
# Plugin name
#
set(project_BIN ${PROJECT_NAME}-${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR})

#
# Plugin sources
#
file(GLOB_RECURSE PLUGIN_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
file(GLOB_RECURSE REMOVEFILESUNITTEST *_test.cpp)

#Remove the unit test files (not been used by the library)
list(REMOVE_ITEM PLUGIN_SRCS ${REMOVEFILESUNITTEST})

#
# Qt MOC
#
set(MOC_HDRS
#  qt/MVGMenu.h
#  qt/MVGMenuItem.h
  qt/MVGMainWidget.h
  qt/MVGCameraWrapper.h
  qt/MVGProjectWrapper.h
  qt/QmlInstantCoding.h
  qt/MVGMayaEvent.h
  qt/MVGMayaViewportEvent.h
)
qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})
qt4_wrap_ui(UISrcs 
	qt/ui/menu.ui
	qt/ui/menuItem.ui)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


#
# Maya plugin properties
#
add_library(${project_BIN} SHARED ${PLUGIN_SRCS} ${MOC_SRCS} ${UISrcs} ../openMVG/src/software/SfMViewer/Ply.cpp)
target_link_libraries(${project_BIN} ${MAYA_Foundation_LIBRARY} ${MAYA_OpenMaya_LIBRARY} ${MAYA_OpenMayaUI_LIBRARY} ${MAYA_OpenMayaFX_LIBRARY} stlplus openMVG_numeric openMVG_multiview ${QT_LIBRARIES})
maya_set_plugin_properties(${project_BIN})

