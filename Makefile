BUILD_DIR = build


$(BUILD_DIR):
	-mkdir $(BUILD_DIR) 2>/dev/null
	cd $(BUILD_DIR); \
	rm -rf *

release: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -DCMAKE_BUILD_TYPE=Release ..

debug: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake ..

xcode: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	cmake -G Xcode ..

clean: $(BUILD_DIR)
	cd $(BUILD_DIR); \
	rm -rf *
