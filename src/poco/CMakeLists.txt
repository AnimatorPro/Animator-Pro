
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(poco LANGUAGES C)

set (CMAKE_C_STANDARD 99)

if(CMAKE_BUILD_TYPE STREQUAL "debug")
	add_compile_definitions(-DDEVELOPMENT=1 -DDEBUG=1)
endif()


set(LIBPOCO_SRC_FILES
		bcrcall.c
		bcutil.c
		bop.c
		chopper.c
		code.c
		declare.c
		dummylib.c
		fold.c
		funccall.c
		linklist.c
		mathlib.c
		pocmemry.c
		poco.c
		pocodis.c
		pocoface.c
		pocolib.c
		pocoop.c
		pocotype.c
		postring.c
		poco_ffi.c
		pp.c
		ppeval.c
		runops.c
		safefile.c
		statemen.c
		strlib.c
		struct.c
		token.c
		trace.c
		varinit.c

		lostdio.h
		poco.h
		pocoop.h
		pp.h
		pplists.h
		statemen.h
		struct.h
		)

list(TRANSFORM
		LIBPOCO_SRC_FILES
	PREPEND
		"${CMAKE_CURRENT_SOURCE_DIR}/"
)


add_library(libpoco ${LIBPOCO_SRC_FILES})
set_target_properties(libpoco PROPERTIES PREFIX "")
target_include_directories(libpoco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../inc")
target_link_libraries(libpoco PUBLIC ffi_static hashmap)

set(POCO_MAIN_SRC_FILES main.c)
if (APPLE OR UNIX)
	list(APPEND POCO_MAIN_SRC_FILES poco_unix.c)
endif()

add_executable(poco ${POCO_MAIN_SRC_FILES})
target_link_libraries(poco libpoco trdfile ffi_static hashmap)
INSTALL(TARGETS poco DESTINATION ./)

add_subdirectory(poco_gui)

