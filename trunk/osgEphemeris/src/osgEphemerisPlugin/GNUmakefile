TOPDIR = ../../
include $(DWMAKE)/makedefs

COMPILER_INCLUDE += -I$(TOPDIR)/include

CXXFILES = \
    IO_EphemerisModel.cpp\

CXXFLAGS    += -fPIC
LINKER_ARGS += -fPIC -L$(TOPDIR)/lib/$(OS)$(ARCH)/

LIBS = -losgEphemeris

LIB = osgdb_osgephemerismodel.so

include $(DWMAKE)/makerules
