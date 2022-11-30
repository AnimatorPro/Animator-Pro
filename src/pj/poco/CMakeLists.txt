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

#set(LIBPOCO_SRC_FILES
#		bcutil.c
#		pocoface.c
#		token.c
#
#		poco.h
#		)


add_library(libpoco ${LIBPOCO_SRC_FILES})
target_include_directories(libpoco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../inc")
set_target_properties(libpoco PROPERTIES PREFIX "")

set(POCO_MAIN_SRC_FILES main.c)
if (APPLE OR UNIX)
	list(APPEND POCO_MAIN_SRC_FILES poco_unix.c)
endif()

add_executable(poco ${POCO_MAIN_SRC_FILES})
target_link_libraries(poco libpoco trdfile)

