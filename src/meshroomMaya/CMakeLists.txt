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

#
# Qml sources
#

file(GLOB_RECURSE QML_SRCS *.qml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
# Maya plugin properties
#

add_library(meshroomMaya SHARED
    ${PLUGIN_SRCS}
    ${MOC_HDRS}
    ${QML_SRCS}
)

target_include_directories(meshroomMaya PUBLIC
    ${MAYA_INCLUDE_DIR}
    ${ALICEVISION_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(meshroomMaya PUBLIC
    ${MAYA_Foundation_LIBRARY}
    ${MAYA_OpenMaya_LIBRARY}
    ${MAYA_OpenMayaUI_LIBRARY}
    ${MAYA_OpenMayaFX_LIBRARY}
    ${MAYA_OpenMayaRender_LIBRARY}
    aliceVision_system
    aliceVision_numeric
    aliceVision_multiview
    aliceVision_image
    ${OPENGL_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::Quick
    Qt5::QuickWidgets
)
maya_set_plugin_properties(meshroomMaya)

#
# Install settings
#

# Install path
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${MAINFOLDER}/dist/plugin CACHE STRING "Plugin install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Plugin file
install(TARGETS meshroomMaya
		DESTINATION "${CMAKE_INSTALL_PREFIX}/plug-ins")

# .mod file
install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.mod
\"+ meshroomMaya ${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR} .\nPYTHONPATH+:=scripts\")")

# Scripts
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/maya/scripts"
		DESTINATION "${CMAKE_INSTALL_PREFIX}")

# .qml files
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/qt/qml/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/qml/mvg")
