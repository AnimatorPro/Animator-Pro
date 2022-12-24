
## Quick wrapper for libffi to integrate with CMake:
## build the library from the sub-folder, then install
## to the current folder.  This sets up an interface
## target called ffi_static for the compiled libs
## and the installed headers.


cmake_minimum_required(VERSION 3.16)
project(libffi)

include(ExternalProject)
ExternalProject_Add(
	libffi
		SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main
		CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/main/autogen.sh COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/main/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR} --disable-shared --disable-docs
		PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
		BUILD_COMMAND make
		BUILD_IN_SOURCE YES
)

add_library(ffi_static INTERFACE)
target_include_directories(ffi_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_directories(ffi_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib")

##!TODO: Windows version
target_link_libraries(ffi_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/libffi.a")
