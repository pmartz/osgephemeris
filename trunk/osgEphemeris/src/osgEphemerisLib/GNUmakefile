TOPDIR = ../../
include $(DWMAKE)/makedefs

COMPILER_INCLUDE += -I$(TOPDIR)/include

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

MOON_IMAGES = \
        $(THISDIR)/moon_1024x512.jpg\
        $(THISDIR)/moon_normal_1024x512.jpg\

SUN_IMAGE = \
            $(THISDIR)/sun.rgba

LIBS =  -losgUtil -losgText -losg 

LIBNAME = osgEphemeris

include $(DWMAKE)/makerules

moon_images.cpp: $(MOON_IMAGES) $(TOPDIR)/bin/$(OSARCH)/makeMoonImages
	$(TOPDIR)/bin/$(OSARCH)/makeMoonImages $(MOON_IMAGES) > $@ 


$(TOPDIR)/bin/$(OSARCH)/makeMoonImages:
	cd $(THISDIR)/../MakeMoonImages; make


sun_image.cpp: $(SUN_IMAGE) $(TOPDIR)/bin/$(OSARCH)/makeSunImage
	$(TOPDIR)/bin/$(OSARCH)/makeSunImage $(SUN_IMAGE) > $@ 

$(TOPDIR)/bin/$(OSARCH)/makeSunImage: 
	cd $(THISDIR)/../MakeSunImage; make
