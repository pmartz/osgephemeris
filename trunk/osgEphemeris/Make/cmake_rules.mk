HELP += \
        "    cmake              - builds project using cmake in $(CMAKE_BUILD_DIR)\n"\
        "    cmake_debug  or cmake_dbg\n"\
        "                       - builds project using cmake with debugging info in $(CMAKE_DEBUG_BUILD_DIR)\n"

cmake:
	[ -d $(CMAKE_BUILD_DIR) ] || (mkdir -p $(CMAKE_BUILD_DIR); cd $(CMAKE_BUILD_DIR); cmake ../)
	cd $(CMAKE_BUILD_DIR); make
	[ -d lib/$(OSARCH) ] || mkdir -p lib/$(OSARCH)
	cd lib/$(OSARCH); for f in ../../$(CMAKE_BUILD_DIR)/lib/*.so; do ln -sf $$f .; done

cmake_debug cmake_dbg:
	[ -d $(CMAKE_DEBUG_BUILD_DIR) ] || (mkdir -p $(CMAKE_DEBUG_BUILD_DIR); cd $(CMAKE_DEBUG_BUILD_DIR); cmake -DBUILD_TYPE=Debug ../)
	cd $(CMAKE_DEBUG_BUILD_DIR); make
	[ -d lib/$(OSARCH) ] || mkdir -p lib/$(OSARCH)
	cd lib/$(OSARCH); for f in ../../$(CMAKE_DEBUG_BUILD_DIR)/lib/*.so; do ln -sf $$f .; done


