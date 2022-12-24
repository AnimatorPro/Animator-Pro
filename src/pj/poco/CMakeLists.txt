
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(poco C)

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

#set(LIBPOCO_SRC_FILES
#		bcutil.c
#		pocoface.c
#		token.c
#
#		poco.h
#		)


add_library(libpoco ${LIBPOCO_SRC_FILES})
target_include_directories(libpoco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../inc")
target_link_libraries(libpoco PUBLIC ffi)
set_target_properties(libpoco PROPERTIES PREFIX "")

set(POCO_MAIN_SRC_FILES main.c)
if (APPLE OR UNIX)
	list(APPEND POCO_MAIN_SRC_FILES poco_unix.c)
endif()

add_executable(poco ${POCO_MAIN_SRC_FILES})
target_link_libraries(poco libpoco trdfile)
INSTALL(TARGETS poco DESTINATION ./)

## cscope
#set(ALL_POCO_FILES ${LIBPOCO_SRC_FILES} ${POCO_MAIN_SRC_FILES})
#string(REPLACE ";" "\n" ALL_POCO_FILES "${ALL_POCO_FILES}")
#file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/cscope.files "${ALL_POCO_FILES}")

