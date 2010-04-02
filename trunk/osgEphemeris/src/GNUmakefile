TOPDIR = ../
include $(DWMAKE)/makedefs
include $(TOPDIR)/Make/cmake_defs.mk

SUBDIRS = $(CMAKE_SUBDIRS)

ifeq ($(SUBDIRS),)
SUBDIRS = \
    MakeMoonImages\
    MakeSunImage\
    osgEphemerisLib\
    osgEphemerisPlugin\
    Gui\
    CallbackExample\

#    Viewer\

endif


include $(DWMAKE)/makerules

