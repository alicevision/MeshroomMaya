#
# Variables that will be defined:
# OPENMVG_FOUND             Defined if an OpenMVG installation has been detected
# OPENMVG_<lib>_FOUND       Defined if <lib> has been found
# OPENMVG_<lib>_LIBRARY     Path to <lib> library
# OPENMVG_LIBRARY_DIR       Path to the library directory
# OPENMVG_INCLUDE_DIR       Path to the include directories
#
# Macros provided:
# OPENMVG_SET_PLUGIN_PROPERTIES  passed the target name, this sets up typical plugin
#                             properties like macro defines, prefixes, and suffixes
#
# Naming convention:
#  Local variables of the form _maya_foo
#  Input variables from CMake of the form Maya_FOO
#  Output variables of the form OPENMVG_FOO
#

#=============================================================================
# Macros
#=============================================================================

FIND_PATH(OPENMVG_INCLUDE_DIR openMVG/multiview/triangulation.hpp
  PATH_SUFFIXES
    include
  DOC "OPENMVG's include path")

FIND_LIBRARY(OPENMVG_IMAGE_LIB NAMES openMVG_image)
FIND_LIBRARY(OPENMVG_COMPUTERVISION_LIB NAMES openMVG_lInftyComputerVision)
FIND_LIBRARY(OPENMVG_KVLD_LIB NAMES openMVG_kvld)
FIND_LIBRARY(OPENMVG_MULTIVIEW_LIB NAMES openMVG_multiview)
FIND_LIBRARY(OPENMVG_NUMERIC_LIB NAMES openMVG_numeric)
FIND_LIBRARY(OPENMVG_SFM_GLOBAL_LIB NAMES openMVG_SfM_Global)
FIND_LIBRARY(OPENMVG_SFM_SEQUENTIAL_LIB NAMES openMVG_SfM_Sequential)
FIND_LIBRARY(STLPLUS_LIB NAMES stlplus)

# handle the QUIETLY and REQUIRED arguments and set OPENMVG_FOUND to TRUE if 
# all listed variables are TRUE
# INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
# FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENMVG DEFAULT_MSG OPENMVG_LIBRARY OPENMVG_INCLUDE_DIR)

# IF(OPENMVG_FOUND)
SET(OPENMVG_LIBRARIES ${OPENMVG_IMAGE_LIB}
                      ${OPENMVG_COMPUTERVISION_LIB}
                      ${OPENMVG_KVLD_LIB}
                      ${OPENMVG_MULTIVIEW_LIB}
                      ${OPENMVG_SFM_GLOBAL_LIB}
                      ${OPENMVG_SFM_SEQUENTIAL_LIB}
                      ${STLPLUS_LIB})

SET(OPENMVG_INCLUDE_DIR ${OPENMVG_INCLUDE_DIR})
# ENDIF(OPENMVG_FOUND)

# Deprecated declarations.
#SET (NATIVE_OPENMVG_INCLUDE_PATH ${OPENMVG_INCLUDE_DIR} )
#IF(OPENMVG_LIBRARY)
#  GET_FILENAME_COMPONENT (NATIVE_OPENMVG_LIB_PATH ${OPENMVG_LIBRARY} PATH)
#ENDIF(OPENMVG_LIBRARY)

MARK_AS_ADVANCED(OPENMVG_LIBRARY OPENMVG_INCLUDE_DIR )
