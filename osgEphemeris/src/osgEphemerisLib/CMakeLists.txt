set( HEADER_PATH ${CMAKE_SOURCE_DIR}/include/osgEphemeris )
set( osgEphemeris_LIBS )
foreach( L OpenThreads osgDB osgUtil osgText osg )
    find_package( ${L} )
    set( osgEphemeris_LIBS ${osgEphemeris_LIBS} ${L} )
endforeach( L OpenThreads osgDB osgUtil osgText osg )

string( REGEX REPLACE [/][^/]*$ ""  OSG_LIB_DIR ${OSG_LIBRARY} )

include_directories( ${CMAKE_SOURCE_DIR}/include ${OPENTHREADS_INCLUDE_DIR} ${OSGDB_INCLUDE_DIR} ${OSGUTIL_INCLUDE_DIR} ${OSGTEXT_INCLUDE_DIR} ${OSG_INCLUDE_DIR} )

IF(WIN32)
ADD_DEFINITIONS(-DOSGEPHEMERIS_LIBRARY)
ENDIF (WIN32)

SET(TARGET_SRC 
        CelestialBodies.cpp
		DateTime.cpp
		EphemerisData.cpp
		EphemerisEngine.cpp
		EphemerisModel.cpp
		EphemerisUpdateCallback.cpp
		GroundPlane.cpp
		MoonModel.cpp
		moon_images.cpp
		Planets.cpp
		Shmem.cpp
		SkyDome.cpp
		Sphere.cpp
		StarField.cpp
		sun_image.cpp
	)


SET(TARGET_H
		${HEADER_PATH}/CelestialBodies.h
		${HEADER_PATH}/DateTime.h
		${HEADER_PATH}/EphemerisData.h
		${HEADER_PATH}/EphemerisEngine.h
		${HEADER_PATH}/EphemerisModel.h
		${HEADER_PATH}/EphemerisUpdateCallback.h
		${HEADER_PATH}/Export.h
		${HEADER_PATH}/GroundPlane.h
		${HEADER_PATH}/MoonModel.h
		${HEADER_PATH}/Planets.h
		${HEADER_PATH}/Shmem.h
		${HEADER_PATH}/SkyDome.h
		${HEADER_PATH}/Sphere.h
		${HEADER_PATH}/StarField.h
	)

LINK_DIRECTORIES( ${OSG_LIB_DIR} )
ADD_LIBRARY(osgEphemeris SHARED ${TARGET_SRC} ${TARGET_H})
TARGET_LINK_LIBRARIES(osgEphemeris ${osgEphemeris_LIBS} )


