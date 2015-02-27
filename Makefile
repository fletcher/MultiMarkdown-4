BUILD_DIR = build


$(BUILD_DIR): 
	-mkdir $(BUILD_DIR) 2>/dev/null
	cd $(BUILD_DIR); \
	rm -rf *

release: $(BUILD_DIR) $(BUILD_DIR)/README.html
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

debug: $(BUILD_DIR)  $(BUILD_DIR)/README.html
	cd $(BUILD_DIR); \
	cmake ..

xcode: $(BUILD_DIR)  $(BUILD_DIR)/README.html
	cd $(BUILD_DIR); \
	cmake -G Xcode ..

clean: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	rm -rf *

$(BUILD_DIR)/README.html: README.md
	multimarkdown -o $(BUILD_DIR)/README.html README.md
