build := build
cmake := cmake
make := make

all:
	mkdir -p $(build)
	$(cmake) -B$(build) -H. -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE
	$(make) -C $(build)

check: all
	set -e; for test in `find $(build) -name "*Test" -type f -perm +111`; do echo; echo "$$test"; ./$$test; done

bindings: bindings/swift

bindings/swift:
	cd $@ && xcodebuild -scheme Arbiter

docs:
	doxygen Doxyfile

clean:
	rm -rf $(build)

