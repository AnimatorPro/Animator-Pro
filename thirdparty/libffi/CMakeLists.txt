
## Quick wrapper for libffi to integrate with CMake:
## build the library from the sub-folder, then install
## to the current folder.  This sets up an interface
## target called ffi_static for the compiled libs
## and the installed headers.


cmake_minimum_required(VERSION 3.16)
project(libffi)

include(ExternalProject)

## add a dummy file until this library gets built or Cmake breaks
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/lib/libffi.a" "temporary")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/include/.placeholder" "temporary")


string(TOLOWER "${CMAKE_BUILD_TYPE}" FFI_BUILD_TYPE)
if (FFI_BUILD_TYPE STREQUAL "debug")
	set(LIBFFI_DEBUG_FLAGS "CFLAGS=\"-g -O0\" --enable-debug")
else()
	set(LIBFFI_DEBUG_FLAGS "")
endif()


set(FFI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
message("libffi source: ${FFI_SRC}")

## Cmake breaks the autotools configure stage by passing unnecessary parameters,
## so I'm making my own configure stage to ignore what it sends.
##!TODO: Windows version
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap_cmake.sh ${FFI_SRC}/bootstrap.sh)


ExternalProject_Add(
	libffi
		BUILD_IN_SOURCE YES
		SOURCE_DIR ${FFI_SRC}
		PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
		CONFIGURE_COMMAND sh ./bootstrap.sh
		BUILD_COMMAND make
		INSTALL_COMMAND make install
)


add_library(ffi_static STATIC IMPORTED GLOBAL)
target_include_directories(ffi_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/include")
message("libffi include: ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/include")
target_link_directories(ffi_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/lib")

add_dependencies(ffi_static libffi)

##!TODO: Windows version
set_target_properties(
	ffi_static
	PROPERTIES
		## can also specify IMPORTED_LOCATION_DEBUG and IMPORTED_LOCATION_RELEASE
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/lib/libffi.a"
)

