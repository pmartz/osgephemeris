TOPDIR = ../../
include $(DWMAKE)/makedefs

CXXFILES = main.cpp \
           Compass.cpp\

COMPILER_INCLUDE += -I$(TOPDIR)/include
LINKER_ARGS      += -L$(TOPDIR)/lib/$(OSARCH)

LIBS     = -losgEphemeris -losgDB -losgUtil -losgGA -losg -losgProducer -lProducer
EXEC     = viewer1

include $(DWMAKE)/makerules
