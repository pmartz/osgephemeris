TOPDIR = ../../
include $(DWMAKE)/makedefs

CXXFILES = \
    main.cpp\


COMPILER_INCLUDE += -I$(TOPDIR)/include
LINKER_ARGS +=  -L$(TOPDIR)/lib/

LIBS = -lfltk -losgEphemeris -losgUtil -losgText -losg

COMPILER_INCLUDE += -I$(THISDIR)

EXEC = egui

include $(DWMAKE)/makerules
