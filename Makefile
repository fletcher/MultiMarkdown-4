BUILD_DIR = build


$(BUILD_DIR): 
	-mkdir $(BUILD_DIR) 2>/dev/null
	cd $(BUILD_DIR); \
	rm -rf *

release: $(BUILD_DIR) $(BUILD_DIR)/README.html $(BUILD_DIR)/LICENSE.html $(BUILD_DIR)/enumMap.txt
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

debug: $(BUILD_DIR) $(BUILD_DIR)/README.html $(BUILD_DIR)/LICENSE.html $(BUILD_DIR)/enumMap.txt
	cd $(BUILD_DIR); \
	cmake ..

xcode: $(BUILD_DIR) $(BUILD_DIR)/README.html $(BUILD_DIR)/LICENSE.html $(BUILD_DIR)/enumMap.txt
	cd $(BUILD_DIR); \
	cmake -G Xcode  ..

clean: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	rm -rf *

$(BUILD_DIR)/README.html: README.md
	multimarkdown -o $(BUILD_DIR)/README.html README.md

$(BUILD_DIR)/LICENSE.html: LICENSE.txt
	multimarkdown -o $(BUILD_DIR)/LICENSE.html LICENSE.txt

map: $(BUILD_DIR)/enumMap.txt

$(BUILD_DIR)/enumMap.txt: src/libMultiMarkdown.h
	./enumsToPerl.pl src/libMultiMarkdown.h $(BUILD_DIR)/enumMap.txt
