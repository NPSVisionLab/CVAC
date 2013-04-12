# ===================================================================================
#  The OpenCV CMake configuration file
#  Modified from the auto-generated OpenCVConfig.cmake file for CVAC
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    FIND_PACKAGE(OpenCV REQUIRED)
#    TARGET_LINK_LIBRARIES(MY_TARGET_NAME ${OpenCV_LIBS})
#
#    Or you can search for specific OpenCV modules:
#
#    FIND_PACKAGE(OpenCV REQUIRED core highgui)
#
#    If the module is found then OPENCV_<MODULE>_FOUND is set to TRUE.
#
#    This file will define the following variables:
#      - OpenCV_LIBS                     : The list of libraries to links against.
#      - OpenCV_LIB_DIR                  : The directory(es) where lib files are. Calling LINK_DIRECTORIES
#                                          with this path is NOT needed.
#      - OpenCV_INCLUDE_DIRS             : The OpenCV include directories.
#      - OpenCV_COMPUTE_CAPABILITIES     : The version of compute capability
#      - OpenCV_ANDROID_NATIVE_API_LEVEL : Minimum required level of Android API
#      - OpenCV_VERSION                  : The version of this OpenCV build. Example: "2.4.2"
#      - OpenCV_VERSION_MAJOR            : Major version part of OpenCV_VERSION. Example: "2"
#      - OpenCV_VERSION_MINOR            : Minor version part of OpenCV_VERSION. Example: "4"
#      - OpenCV_VERSION_PATCH            : Patch version part of OpenCV_VERSION. Example: "2"
#
#    Advanced variables:
#      - OpenCV_SHARED
#      - OpenCV_INSTALL_PATH  (not set on Windows)
#      - OpenCV_LIB_COMPONENTS
#      - OpenCV_USE_MANGLED_PATHS
#
# ===================================================================================

# Version Compute Capability from which OpenCV has been compiled is remembered
set(OpenCV_COMPUTE_CAPABILITIES "")

# Android API level from which OpenCV has been compiled is remembered
set(OpenCV_ANDROID_NATIVE_API_LEVEL 0)

# Some additional settings are required if OpenCV is built as static libs
set(OpenCV_SHARED ON)

# Enables mangled install paths, that help with side by side installs
set(OpenCV_USE_MANGLED_PATHS FALSE)

# ======================================================
# Root OpenCV directory
# ======================================================
FIND_PATH (OpenCV_ROOT_DIR  include/opencv2
           HINTS
           ${CMAKE_SOURCE_DIR}/3rdparty/OpenCV-2.4.2 
           DOC "The OpenCV root folder"
           )

# ======================================================
# Include directories to add to the user project:
# ======================================================

# Provide the include directories to the caller
set(OpenCV_INCLUDE_DIRS "${OpenCV_ROOT_DIR}/include" "${OpenCV_ROOT_DIR}/include/opencv")
include_directories(${OpenCV_INCLUDE_DIRS})

# ======================================================
# Link directories to add to the user project:
# ======================================================

# Provide the libs directories to the caller
set(LIBPATH lib)
IF (MSVC90)
    set (LIBPATH "x86/vc9/lib")
ENDIF (MSVC90)
IF (MSVC10)
    set (LIBPATH "x86/vc10/lib")
ENDIF (MSVC10)
set(OpenCV_LIB_DIR_OPT "${OpenCV_ROOT_DIR}/${LIBPATH}" CACHE PATH "Path where release OpenCV libraries are located")
set(OpenCV_LIB_DIR_DBG "${OpenCV_ROOT_DIR}/${LIBPATH}" CACHE PATH "Path where debug OpenCV libraries are located")
set(OpenCV_3RDPARTY_LIB_DIR_OPT "${OpenCV_ROOT_DIR}/lib/3rdparty/${LIBPATH}" CACHE PATH "Path where release 3rdpaty OpenCV dependencies are located")
set(OpenCV_3RDPARTY_LIB_DIR_DBG "${OpenCV_ROOT_DIR}/lib/3rdparty/${LIBPATH}" CACHE PATH "Path where debug 3rdpaty OpenCV dependencies are located")
mark_as_advanced(FORCE OpenCV_LIB_DIR_OPT OpenCV_LIB_DIR_DBG OpenCV_3RDPARTY_LIB_DIR_OPT OpenCV_3RDPARTY_LIB_DIR_DBG OpenCV_CONFIG_PATH)

# ======================================================
#  Version variables and library prefix and suffix
# ======================================================
SET(OpenCV_VERSION_MAJOR  2)
SET(OpenCV_VERSION_MINOR  4)
SET(OpenCV_VERSION_PATCH  2)
SET(OpenCV_VERSION ${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH})
if (WIN32)
  MESSAGE(STATUS "Configuring OpenCV library naming for Windows platforms")
  set(OpenCV_VERSION_STRING "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}")
  set(OpenCV_LIB_OPT_SUFFIX "${OpenCV_VERSION_STRING}.lib")
  set(OpenCV_LIB_DBG_SUFFIX "${OpenCV_VERSION_STRING}d.lib")
elseif (APPLE)
  MESSAGE(STATUS "Configuring OpenCV library naming for Apple platforms")
  set(OpenCV_LIB_PREFIX "lib")
  set(OpenCV_VERSION_STRING "${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")
  set(OpenCV_LIB_OPT_SUFFIX ".${OpenCV_VERSION_STRING}.dylib")
  set(OpenCV_LIB_DBG_SUFFIX ".${OpenCV_VERSION_STRING}.dylib")
else (UNIX && CMAKE_SYSTEM_NAME MATCHES "Linux")
  MESSAGE(STATUS "Configuring OpenCV library naming for Linux platforms")
  set(OpenCV_LIB_PREFIX "lib")
  set(OpenCV_LIB_PREFIX "lib")
  set(OpenCV_VERSION_STRING ".${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")
  if(OpenCV_USE_MANGLED_PATHS)
    set(OpenCV_LIB_OPT_SUFFIX ".so.${OpenCV_VERSION_STRING}")
    set(OpenCV_LIB_DBG_SUFFIX ".so.${OpenCV_VERSION_STRING}")
  else ()
    set(OpenCV_LIB_OPT_SUFFIX ".so")
    set(OpenCV_LIB_DBG_SUFFIX ".so")
  endif(OpenCV_USE_MANGLED_PATHS)
else ()
  MESSAGE(WARNING "unknown OS in FindOpenCV.cmake")
endif (WIN32)


# ====================================================================
# Link libraries: e.g.   libopencv_core.so, opencv_imgproc220d.lib, etc...
# ====================================================================

SET(OpenCV_LIB_COMPONENTS opencv_videostab;opencv_video;opencv_ts;opencv_stitching;opencv_photo;opencv_objdetect;opencv_nonfree;opencv_ml;opencv_legacy;opencv_imgproc;opencv_highgui;opencv_gpu;opencv_flann;opencv_features2d;opencv_core;opencv_contrib;opencv_calib3d)

set(OpenCV_opencv_videostab_LIBNAME_OPT "opencv_videostab")
set(OpenCV_opencv_videostab_DEPS_OPT )
set(OpenCV_opencv_videostab_EXTRA_DEPS_OPT )
set(OpenCV_opencv_video_LIBNAME_OPT "opencv_video")
set(OpenCV_opencv_video_DEPS_OPT )
set(OpenCV_opencv_video_EXTRA_DEPS_OPT )
set(OpenCV_opencv_ts_LIBNAME_OPT "opencv_ts")
set(OpenCV_opencv_ts_DEPS_OPT )
set(OpenCV_opencv_ts_EXTRA_DEPS_OPT )
set(OpenCV_opencv_stitching_LIBNAME_OPT "opencv_stitching")
set(OpenCV_opencv_stitching_DEPS_OPT )
set(OpenCV_opencv_stitching_EXTRA_DEPS_OPT )
set(OpenCV_opencv_photo_LIBNAME_OPT "opencv_photo")
set(OpenCV_opencv_photo_DEPS_OPT )
set(OpenCV_opencv_photo_EXTRA_DEPS_OPT )
set(OpenCV_opencv_objdetect_LIBNAME_OPT "opencv_objdetect")
set(OpenCV_opencv_objdetect_DEPS_OPT )
set(OpenCV_opencv_objdetect_EXTRA_DEPS_OPT )
set(OpenCV_opencv_nonfree_LIBNAME_OPT "opencv_nonfree")
set(OpenCV_opencv_nonfree_DEPS_OPT )
set(OpenCV_opencv_nonfree_EXTRA_DEPS_OPT )
set(OpenCV_opencv_ml_LIBNAME_OPT "opencv_ml")
set(OpenCV_opencv_ml_DEPS_OPT )
set(OpenCV_opencv_ml_EXTRA_DEPS_OPT )
set(OpenCV_opencv_legacy_LIBNAME_OPT "opencv_legacy")
set(OpenCV_opencv_legacy_DEPS_OPT )
set(OpenCV_opencv_legacy_EXTRA_DEPS_OPT )
set(OpenCV_opencv_imgproc_LIBNAME_OPT "opencv_imgproc")
set(OpenCV_opencv_imgproc_DEPS_OPT )
set(OpenCV_opencv_imgproc_EXTRA_DEPS_OPT )
set(OpenCV_opencv_highgui_LIBNAME_OPT "opencv_highgui")
set(OpenCV_opencv_highgui_DEPS_OPT )
set(OpenCV_opencv_highgui_EXTRA_DEPS_OPT )
set(OpenCV_opencv_gpu_LIBNAME_OPT "opencv_gpu")
set(OpenCV_opencv_gpu_DEPS_OPT )
set(OpenCV_opencv_gpu_EXTRA_DEPS_OPT )
set(OpenCV_opencv_flann_LIBNAME_OPT "opencv_flann")
set(OpenCV_opencv_flann_DEPS_OPT )
set(OpenCV_opencv_flann_EXTRA_DEPS_OPT )
set(OpenCV_opencv_features2d_LIBNAME_OPT "opencv_features2d")
set(OpenCV_opencv_features2d_DEPS_OPT )
set(OpenCV_opencv_features2d_EXTRA_DEPS_OPT )
set(OpenCV_opencv_core_LIBNAME_OPT "opencv_core")
set(OpenCV_opencv_core_DEPS_OPT )
set(OpenCV_opencv_core_EXTRA_DEPS_OPT )
set(OpenCV_opencv_contrib_LIBNAME_OPT "opencv_contrib")
set(OpenCV_opencv_contrib_DEPS_OPT )
set(OpenCV_opencv_contrib_EXTRA_DEPS_OPT )
set(OpenCV_opencv_calib3d_LIBNAME_OPT "opencv_calib3d")
set(OpenCV_opencv_calib3d_DEPS_OPT )
set(OpenCV_opencv_calib3d_EXTRA_DEPS_OPT )


set(OpenCV_opencv_videostab_LIBNAME_DBG "opencv_videostab")
set(OpenCV_opencv_videostab_DEPS_DBG )
set(OpenCV_opencv_videostab_EXTRA_DEPS_DBG )
set(OpenCV_opencv_video_LIBNAME_DBG "opencv_video")
set(OpenCV_opencv_video_DEPS_DBG )
set(OpenCV_opencv_video_EXTRA_DEPS_DBG )
set(OpenCV_opencv_ts_LIBNAME_DBG "opencv_ts")
set(OpenCV_opencv_ts_DEPS_DBG )
set(OpenCV_opencv_ts_EXTRA_DEPS_DBG )
set(OpenCV_opencv_stitching_LIBNAME_DBG "opencv_stitching")
set(OpenCV_opencv_stitching_DEPS_DBG )
set(OpenCV_opencv_stitching_EXTRA_DEPS_DBG )
set(OpenCV_opencv_photo_LIBNAME_DBG "opencv_photo")
set(OpenCV_opencv_photo_DEPS_DBG )
set(OpenCV_opencv_photo_EXTRA_DEPS_DBG )
set(OpenCV_opencv_objdetect_LIBNAME_DBG "opencv_objdetect")
set(OpenCV_opencv_objdetect_DEPS_DBG )
set(OpenCV_opencv_objdetect_EXTRA_DEPS_DBG )
set(OpenCV_opencv_nonfree_LIBNAME_DBG "opencv_nonfree")
set(OpenCV_opencv_nonfree_DEPS_DBG )
set(OpenCV_opencv_nonfree_EXTRA_DEPS_DBG )
set(OpenCV_opencv_ml_LIBNAME_DBG "opencv_ml")
set(OpenCV_opencv_ml_DEPS_DBG )
set(OpenCV_opencv_ml_EXTRA_DEPS_DBG )
set(OpenCV_opencv_legacy_LIBNAME_DBG "opencv_legacy")
set(OpenCV_opencv_legacy_DEPS_DBG )
set(OpenCV_opencv_legacy_EXTRA_DEPS_DBG )
set(OpenCV_opencv_imgproc_LIBNAME_DBG "opencv_imgproc")
set(OpenCV_opencv_imgproc_DEPS_DBG )
set(OpenCV_opencv_imgproc_EXTRA_DEPS_DBG )
set(OpenCV_opencv_highgui_LIBNAME_DBG "opencv_highgui")
set(OpenCV_opencv_highgui_DEPS_DBG )
set(OpenCV_opencv_highgui_EXTRA_DEPS_DBG )
set(OpenCV_opencv_gpu_LIBNAME_DBG "opencv_gpu")
set(OpenCV_opencv_gpu_DEPS_DBG )
set(OpenCV_opencv_gpu_EXTRA_DEPS_DBG )
set(OpenCV_opencv_flann_LIBNAME_DBG "opencv_flann")
set(OpenCV_opencv_flann_DEPS_DBG )
set(OpenCV_opencv_flann_EXTRA_DEPS_DBG )
set(OpenCV_opencv_features2d_LIBNAME_DBG "opencv_features2d")
set(OpenCV_opencv_features2d_DEPS_DBG )
set(OpenCV_opencv_features2d_EXTRA_DEPS_DBG )
set(OpenCV_opencv_core_LIBNAME_DBG "opencv_core")
set(OpenCV_opencv_core_DEPS_DBG )
set(OpenCV_opencv_core_EXTRA_DEPS_DBG )
set(OpenCV_opencv_contrib_LIBNAME_DBG "opencv_contrib")
set(OpenCV_opencv_contrib_DEPS_DBG )
set(OpenCV_opencv_contrib_EXTRA_DEPS_DBG )
set(OpenCV_opencv_calib3d_LIBNAME_DBG "opencv_calib3d")
set(OpenCV_opencv_calib3d_DEPS_DBG )
set(OpenCV_opencv_calib3d_EXTRA_DEPS_DBG )


# ==============================================================
#  Extra include directories, needed by OpenCV 2 new structure
# ==============================================================
# set to "modules/" if referencing an OpenCV in its source tree
# rather than the installed location (which doesn't have "modules" in the path)
SET(UNINSTALLED_INC "")
SET(OpenCV2_INCLUDE_DIRS ${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}core/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}imgproc/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}flann/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}highgui/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}features2d/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}calib3d/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}ml/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}video/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}objdetect/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}contrib/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}nonfree/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}gpu/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}legacy/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}photo/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}stitching/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}ts/include;${OpenCV_ROOT_DIR}/${UNINSTALLED_INC}videostab/include)
if(OpenCV2_INCLUDE_DIRS)
  include_directories(${OpenCV2_INCLUDE_DIRS})
  list(APPEND OpenCV_INCLUDE_DIRS ${OpenCV2_INCLUDE_DIRS})

  set(OpenCV_ADD_DEBUG_RELEASE FALSE)
  if(OpenCV_ADD_DEBUG_RELEASE)
    set(OpenCV_LIB_DIR_OPT "${OpenCV_LIB_DIR_OPT}/Release")
    set(OpenCV_LIB_DIR_DBG "${OpenCV_LIB_DIR_DBG}/Debug")
    set(OpenCV_3RDPARTY_LIB_DIR_OPT "${OpenCV_3RDPARTY_LIB_DIR_OPT}/Release")
    set(OpenCV_3RDPARTY_LIB_DIR_DBG "${OpenCV_3RDPARTY_LIB_DIR_DBG}/Debug")
  endif()
endif()

# ==============================================================
#  Form list of modules (components) to find
# ==============================================================
if(NOT OpenCV_FIND_COMPONENTS)
  set(OpenCV_FIND_COMPONENTS ${OpenCV_LIB_COMPONENTS})
  if(GTest_FOUND OR GTEST_FOUND)
    list(REMOVE_ITEM OpenCV_FIND_COMPONENTS opencv_ts)
  endif()
endif()

# expand short module names and see if requested components exist
set(OpenCV_FIND_COMPONENTS_ "")
foreach(__cvcomponent ${OpenCV_FIND_COMPONENTS})
  if(NOT __cvcomponent MATCHES "^opencv_")
    set(__cvcomponent opencv_${__cvcomponent})
  endif()
  list(FIND OpenCV_LIB_COMPONENTS ${__cvcomponent} __cvcomponentIdx)
  if(__cvcomponentIdx LESS 0)
    #requested component is not found...
    if(OpenCV_FIND_REQUIRED)
      message(FATAL_ERROR "${__cvcomponent} is required but was not found")
    elseif(NOT OpenCV_FIND_QUIETLY)
      message(WARNING "${__cvcomponent} is required but was not found")
    endif()
    #indicate that module is NOT found
    string(TOUPPER "${__cvcomponent}" __cvcomponent)
    set(${__cvcomponent}_FOUND "${__cvcomponent}_FOUND-NOTFOUND")
  else()
    list(APPEND OpenCV_FIND_COMPONENTS_ ${__cvcomponent})
    #indicate that module is found
    string(TOUPPER "${__cvcomponent}" __cvcomponent)
    set(${__cvcomponent}_FOUND 1)
  endif()
endforeach()
set(OpenCV_FIND_COMPONENTS ${OpenCV_FIND_COMPONENTS_})

# ==============================================================
#  Resolve dependencies
# ==============================================================

foreach(__opttype OPT DBG)
  SET(OpenCV_LIBS_${__opttype} "")
  SET(OpenCV_EXTRA_LIBS_${__opttype} "")
  foreach(__cvlib ${OpenCV_FIND_COMPONENTS})
    foreach(__cvdep ${OpenCV_${__cvlib}_DEPS_${__opttype}})
      if(__cvdep MATCHES "^opencv_")
        list(APPEND OpenCV_LIBS_${__opttype} "${OpenCV_LIB_DIR_${__opttype}}/${OpenCV_LIB_PREFIX}${OpenCV_${__cvdep}_LIBNAME_${__opttype}}${OpenCV_LIB_${__opttype}_SUFFIX}")
        #indicate that this module is also found
        string(TOUPPER "${__cvdep}" __cvdep)
        set(${__cvdep}_FOUND 1)
      else()
        list(APPEND OpenCV_LIBS_${__opttype} "${OpenCV_3RDPARTY_LIB_DIR_${__opttype}}/${OpenCV_${__cvdep}_LIBNAME_${__opttype}}")
      endif()
    endforeach()
    list(APPEND OpenCV_LIBS_${__opttype} "${OpenCV_LIB_DIR_${__opttype}}/${OpenCV_LIB_PREFIX}${OpenCV_${__cvlib}_LIBNAME_${__opttype}}${OpenCV_LIB_${__opttype}_SUFFIX}")
    list(APPEND OpenCV_EXTRA_LIBS_${__opttype} ${OpenCV_${__cvlib}_EXTRA_DEPS_${__opttype}})
  endforeach()

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 2.4)
    if(OpenCV_LIBS_${__opttype})
      list(REMOVE_DUPLICATES OpenCV_LIBS_${__opttype})
    endif()
    if(OpenCV_EXTRA_LIBS_${__opttype})
      list(REMOVE_DUPLICATES OpenCV_EXTRA_LIBS_${__opttype})
    endif()
  else()
    #TODO: duplicates are annoying but they should not be the problem
  endif()
endforeach()

if(OpenCV_LIBS_DBG)
  list(REVERSE OpenCV_LIBS_DBG)
endif()

if(OpenCV_LIBS_OPT)
  list(REVERSE OpenCV_LIBS_OPT)
endif()

# CMake>=2.6 supports the notation "debug XXd optimized XX"
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 2.4)
  # Modern CMake:
  SET(OpenCV_LIBS "")
  foreach(__cvlib ${OpenCV_LIBS_DBG} ${OpenCV_EXTRA_LIBS_DBG})
    list(APPEND OpenCV_LIBS debug "${__cvlib}")
  endforeach()
  foreach(__cvlib ${OpenCV_LIBS_OPT} ${OpenCV_EXTRA_LIBS_OPT})
    list(APPEND OpenCV_LIBS optimized "${__cvlib}")
  endforeach()
else()
  # Old CMake:
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(OpenCV_LIBS ${OpenCV_LIBS_DBG} ${OpenCV_EXTRA_LIBS_DBG})
  else()
    SET(OpenCV_LIBS ${OpenCV_LIBS_OPT} ${OpenCV_EXTRA_LIBS_OPT})
  endif()
endif()

# ==============================================================
# Compatibility stuff
# ==============================================================
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(OpenCV_LIB_DIR ${OpenCV_LIB_DIR_DBG} ${OpenCV_3RDPARTY_LIB_DIR_DBG})
else()
  SET(OpenCV_LIB_DIR ${OpenCV_LIB_DIR_OPT} ${OpenCV_3RDPARTY_LIB_DIR_OPT})
endif()
set(OpenCV_LIBRARIES ${OpenCV_LIBS})

# ==============================================================
# handle the QUIETLY and REQUIRED arguments and set OpenCV_FOUND to TRUE if 
# all listed variables are TRUE
# ==============================================================
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCV DEFAULT_MSG OpenCV_INCLUDE_DIRS
                                                  OpenCV_LIBRARIES
                                                  )
