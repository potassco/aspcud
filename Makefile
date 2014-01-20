# flags for the release build
CFLAGS=-O3 -DNDEBUG
CPPFLAGS=
CXXFLAGS=-O3 -DNDEBUG
LDFLAGS=
CMAKE_OPTS=

all: release

debug:
	mkdir -p build/debug
	cd build/debug && \
		cmake ../.. \
		$(CMAKE_OPTS) \
		-DCMAKE_CXX_FLAGS=-Wall \
		-DCMAKE_BUILD_TYPE=debug && \
	$(MAKE)

release:
	mkdir -p build/release
	cd build/release && \
	cmake ../.. \
		$(CMAKE_OPTS) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_FLAGS_RELEASE="$(CFLAGS) $(CPPFLAGS)" \
		-DCMAKE_CXX_FLAGS_RELEASE="$(CXXFLAGS) $(CPPFLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS_RELEASE="$(LDFLAGS)" && \
	$(MAKE)

static:
	mkdir -p build/static
	cd build/static && \
	cmake ../.. \
		$(CMAKE_OPTS) \
		-DCMAKE_BUILD_TYPE=release -DUSE_STATIC_LIBS=true && \
	$(MAKE)

clean:
	rm -rf build
