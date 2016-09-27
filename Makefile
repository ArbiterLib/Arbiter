BUILD := build
CMAKE := cmake

all: cmake
	$(CMAKE) --build $(BUILD)

cmake:
	mkdir -p $(BUILD)
	$(CMAKE) -B$(BUILD) -H. -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE

check: all
	set -e; for test in `find $(BUILD) -name "*Test" -type f -perm +111`; do echo; echo "$$test"; ./$$test; done

bindings: bindings/swift

bindings/swift:
	cd $@ && xcodebuild -scheme Arbiter

docs:
	doxygen Doxyfile

examples: cmake
	$(CMAKE) --build $(BUILD) --target library_folders

clean:
	rm -rf $(BUILD)

