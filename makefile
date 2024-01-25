.PHONY: all release debug run poco clean

all: debug run

debug:
	rm -rf build && mkdir -p build_debug && pushd build_debug && cmake -G "Ninja" -DWITH_POCO=ON .. && cmake --build . --config debug --target install && popd

release:
	rm -rf build && mkdir -p build && pushd build && cmake -G "Ninja" -DWITH_POCO=ON .. && cmake --build . --config RelWithDebInfo --target install && popd

run:
	build_debug/bin/ani

poco:
	build_debug/bin/poco

clean:
	rm -rf build build_debug && echo Project Cleaned

