cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(pj C)

include_directories("inc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wold-style-definition")

add_subdirectory(fli)
add_subdirectory(font)
add_subdirectory(gfx)
add_subdirectory(picdrive)
add_subdirectory(poco)
add_subdirectory(rastlib)
add_subdirectory(rexlib)
add_subdirectory(util)

set(PJMAIN_SRC_FILES
	test.c
	)

add_executable(pj WIN32
	${PJMAIN_SRC_FILES})

target_link_libraries(pj
	flilib
	fontlib
	gfxlib
	picdrive
	poco
	raster
	minirex
	trdutil)
