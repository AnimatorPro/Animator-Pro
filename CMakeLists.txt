cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(animator-pro LANGUAGES C CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	# will not take effect without FORCE
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Install top-level directory" FORCE)
endif()

add_compile_options(
	-Wno-deprecated-declarations
	-Wno-incompatible-pointer-types
	-Wno-address-of-packed-member
	-Wno-sometimes-uninitialized
	-Wno-format-security
	-Wno-null-pointer-subtraction
)

## thirdparty dependencies
add_subdirectory(thirdparty/sdl2)
include_directories(thirdparty/sdl2/include)

## main
add_subdirectory(src/PJ)

## tests
add_subdirectory(tests)