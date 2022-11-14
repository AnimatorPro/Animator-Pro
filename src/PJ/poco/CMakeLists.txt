set(POCO_SRC_FILES
	bcutil.c
	pocoface.c
	token.c

	poco.h
)

add_library(libpoco ${POCO_SRC_FILES})
