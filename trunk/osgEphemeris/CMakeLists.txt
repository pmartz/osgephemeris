cmake_minimum_required(VERSION 2.6)

set(ProjectName osgEphemeris)

set (CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH "${osgEphemeris_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

PROJECT(${ProjectName})

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

SET(CMAKE_DEBUG_POSTFIX  "d")

################################################################################
# Create lib directory if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

################################################################################

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})


add_subdirectory(src)

