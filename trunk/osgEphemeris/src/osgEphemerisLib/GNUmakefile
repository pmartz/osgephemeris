TOPDIR = ../../
include $(DWMAKE)/makedefs
include $(TOPDIR)/Make/cmake_defs.mk

COMPILER_INCLUDE += -I$(TOPDIR)/include

CXXFILES = $(CMAKE_CXXFILES)

ifeq ($(CXXFILES),)

CXXFILES = \
           EphemerisModel.cpp\
           EphemerisData.cpp\
           DateTime.cpp\
           EphemerisEngine.cpp\
           CelestialBodies.cpp\
           Sphere.cpp\
           SkyDome.cpp\
           GroundPlane.cpp\
           MoonModel.cpp\
           Planets.cpp\
           StarField.cpp\
           Shmem.cpp\
           EphemerisUpdateCallback.cpp\
           moon_images.cpp\
           sun_image.cpp\

endif

LIBS =  -losgUtil -losgText -losg 

LIBNAME = osgEphemeris

include $(DWMAKE)/makerules

