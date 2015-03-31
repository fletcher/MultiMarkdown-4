BUILD_DIR = build


$(BUILD_DIR): 
	-mkdir $(BUILD_DIR) 2>/dev/null
	cd $(BUILD_DIR); \
	rm -rf *

release: extras
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

debug: extras
	cd $(BUILD_DIR); \
	cmake ..

<<<<<<< HEAD
xcode: extras
	cd $(BUILD_DIR); \
	cmake -G Xcode  ..
=======
# OUR_GREG: the version of greg in a submodule
# GREG: the path to greg we want to use for parser.leg
#
# This way we can pass GREG=/usr/local/bin/greg in on
# the command line if we have greg installed already.

OUR_GREG=greg/greg
GREG?=$(OUR_GREG)
>>>>>>> master

# This is for cross-compiling for Windows via MinGW32
windows: extras
	cd $(BUILD_DIR); \
	cmake -DCMAKE_TOOLCHAIN_FILE=../tools/Toolchain-mingw32.cmake -DCMAKE_BUILD_TYPE=Release ..

clean: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	rm -rf *

<<<<<<< HEAD
extras: $(BUILD_DIR) $(BUILD_DIR)/README.html $(BUILD_DIR)/LICENSE.html $(BUILD_DIR)/enumMap.txt

$(BUILD_DIR)/README.html: README.md
	multimarkdown -o $(BUILD_DIR)/README.html README.md
=======
parser.c : parser.leg $(GREG) parser.h
	$(GREG) -o parser.c parser.leg

$(OUR_GREG): greg
	$(MAKE) -C greg
>>>>>>> master

$(BUILD_DIR)/LICENSE.html: LICENSE.txt
	multimarkdown -o $(BUILD_DIR)/LICENSE.html LICENSE.txt

map: $(BUILD_DIR)/enumMap.txt

$(BUILD_DIR)/enumMap.txt: src/libMultiMarkdown.h
	./tools/enumsToPerl.pl src/libMultiMarkdown.h $(BUILD_DIR)/enumMap.txt

# Use default cpack generator to build an installer (e.g. NSIS for Windows, Packagemaker for OS X, etc.)
installer: release
	cd $(BUILD_DIR); \
	cpack

# Use cpack to create a zipfile containing the relevant files (useful as a portable version)
portable: release
	cd $(BUILD_DIR); \
	cpack -G ZIP -D CPACK_INSTALL_PREFIX=""
