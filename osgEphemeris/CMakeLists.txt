cmake_minimum_required(VERSION 2.6)

set( ProjectName osgEphemeris )
project( ${ProjectName} )

set (CMAKE_BUILD_TYPE Release)


list( APPEND CMAKE_MODULE_PATH "${osgEphemeris_SOURCE_DIR}/CMakeModules" )

# CMake 2.8 forward ships with scripts to find OSG. For older CMakes, uncomment this line:
# list( APPEND CMAKE_MODULE_PATH "${osgEphemeris_SOURCE_DIR}/CMakeModules/OSGSupport" )

SET(osgEphemeris_MAJOR_VERSION 0)
SET(osgEphemeris_MINOR_VERSION 1)
SET(osgEphemeris_SUB_VERSION 0)
SET(osgEphemeris_VERSION ${osgEphemeris_MAJOR_VERSION}.${osgEphemeris_MINOR_VERSION}.${osgEphemeris_SUB_VERSION})
set(_PACKAGE_VERSION ${osgEphemeris_VERSION})

IF(UNIX)
    # Some Unicies need explicit linkage to the Math library or the build fails.
    FIND_LIBRARY(MATH_LIBRARY m)
    ADD_DEFINITIONS(-Wall -Wextra)
ENDIF(UNIX)

################################################################################
# Platform specific definitions
IF(WIN32)
    IF(MSVC)
        # To enable /MP, parralel build on MSVC
        OPTION(WIN32_USE_MP "Set to OFF to diable /MP." ON)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # Other MSVC compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

    ENDIF(MSVC)
ENDIF (WIN32)

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually rd")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually s")

IF(UNIX AND NOT WIN32)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

IF(CYGWIN)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

include(BinAndLibDirs)

#IF(UNIX AND NOT WIN32 AND NOT APPLE)
#  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
#      SET(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
#      MARK_AS_ADVANCED(LIB_POSTFIX)
#  ENDIF()
#ENDIF()
#IF(NOT DEFINED LIB_POSTFIX)
#    SET(LIB_POSTFIX "")
#ENDIF()

################################################################################
# Create lib directory if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${_INSTALL_ARCHIVEDIR})
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

################################################################################

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/${_INSTALL_ARCHIVEDIR})
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})


add_subdirectory(src)

set( _EXPORT_LIBRARY_TARGETS osgEphemeris )

include(InstallAutoDiscoveryTools)


