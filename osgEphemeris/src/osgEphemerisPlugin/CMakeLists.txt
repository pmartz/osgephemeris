set( HEADER_PATH ${CMAKE_SOURCE_DIR}/include/osgEphemeris )

set( osgEphemerisPlugin_LIBS osgEphemeris )
foreach( L OpenThreads osgDB osgUtil osgText osg )
    find_package( ${L} )
    set( osgEphemeris_LIBS ${osgEphemerisPlugin_LIBS} ${L} )
endforeach( L OpenThreads osgDB osgUtil osgText osg )

string( REGEX REPLACE [/][^/]*$ ""  OSG_LIB_DIR ${OSG_LIBRARY} )

include_directories( 
        ${CMAKE_SOURCE_DIR}/include 
        ${OPENTHREADS_INCLUDE_DIR} 
        ${OSGDB_INCLUDE_DIR} 
        ${OSGUTIL_INCLUDE_DIR} 
        ${OSGTEXT_INCLUDE_DIR} 
        ${OSG_INCLUDE_DIR} )


SET(TARGET_SRC
    IO_EphemerisModel.cpp
	)


LINK_DIRECTORIES( ${OSG_LIB_DIR} )
ADD_LIBRARY(osgdb_osgephemerismodel SHARED ${TARGET_SRC} )
SET_TARGET_PROPERTIES( osgdb_osgephemerismodel PROPERTIES PREFIX "" )
TARGET_LINK_LIBRARIES(osgdb_osgephemerismodel ${osgEphemerisPlugin_LIBS} )
