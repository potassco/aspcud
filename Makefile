all: release

debug:
	mkdir -p build/debug
	cd build/debug && \
		cmake ../.. \
		-DCMAKE_CXX_FLAGS=-Wall \
		-DCMAKE_BUILD_TYPE=debug && \
	$(MAKE)

release:
	mkdir -p build/release
	cd build/release && \
	cmake ../.. \
		-DCMAKE_BUILD_TYPE=release && \
	$(MAKE)

static:
	mkdir -p build/static
	cd build/static && \
	cmake ../.. \
		-DCMAKE_BUILD_TYPE=release -DUSE_STATIC_LIBS=true && \
	$(MAKE)

clean:
	rm -rf build
