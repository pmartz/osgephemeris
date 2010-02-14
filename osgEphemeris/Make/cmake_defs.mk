
CMAKE_BUILD_DIR = CMakeBuild.$(OSARCH).opt
CMAKE_DEBUG_BUILD_DIR = CMakeBuild.$(OSARCH).dbg
CMAKE_SUBDIRS  = $(shell [ -f CMakeLists.txt ] && ( grep -i add_subdirectory CMakeLists.txt | cut -f2 -d"("| cut -f1 -d")" ) )
CMAKE_CXXFILES = $(shell [ -f CMakeLists.txt ] && ( grep ".cpp" CMakeLists.txt ) )

