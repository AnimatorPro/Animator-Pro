cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(animator-pro)

if (PLATFORM STREQUAL "OS64")
	## iOS-specific
	message("Building for platform iOS.")
endif()

if (CMAKE_GENERATOR STREQUAL "Xcode")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D IS_BUNDLE")
endif()

option(USE_ASAN "Use Clang Memory Address Sanitizer" OFF)


if (USE_ASAN)
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=address -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
endif()


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