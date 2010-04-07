set( HEADER_PATH ${CMAKE_SOURCE_DIR}/include/osgEphemeris )
set( osgEphemeris_LIBS )

find_package(OpenSceneGraph 2.0.0 REQUIRED osgDB osgUtil osgText)


include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${OPENSCENEGRAPH_INCLUDE_DIRS}
    )

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

SET(PUBLIC_HEADERS
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

SET(PRIVATE_HEADERS
		star_data.h
	)

SOURCE_GROUP(
        "Private Header Files"
        ${PRIVATE_HEADERS}
    )

SET(TARGET_NAME osgEphemeris)
ADD_LIBRARY(${TARGET_NAME} SHARED ${TARGET_SRC} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${osgEphemeris_LIBS} ${OPENSCENEGRAPH_LIBRARIES})


SET(INSTALL_INCDIR include)
SET(INSTALL_BINDIR bin)
IF(WIN32)
    SET(INSTALL_LIBDIR bin)
    SET(INSTALL_ARCHIVEDIR lib)
ELSE()
    SET(INSTALL_LIBDIR lib${LIB_POSTFIX})
    SET(INSTALL_ARCHIVEDIR lib${LIB_POSTFIX})
ENDIF()


INSTALL(
        TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${INSTALL_BINDIR}
        LIBRARY DESTINATION ${INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR}
    )

INSTALL(
        FILES       ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCDIR}/${TARGET_NAME}
    )
