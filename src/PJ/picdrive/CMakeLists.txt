set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")

set(HOST_SRC_FILES
	host/picdrive.c
	)

set(BMP_SRC_FILES
	bmp/bmp.c
	)

set(FLILO_SRC_FILES
	flilo/compbrun.c
	flilo/flicomp.c
	flilo/flilo.c
	flilo/jfwritos.c
	)

set(GIF_SRC_FILES
	gif/compress.c
	gif/decoder.c
	gif/gif.c
	)

set(PCX_SRC_FILES
	pcx/pcx.c
	)

set(RIF_SRC_FILES
	rif/convbits.c
	rif/rif.c
	rif/unrif.c
	)

set(TARGA_SRC_FILES
	targa/targa.c
	targa/tgareadf.c
	targa/tgawritf.c
	)

add_library(picdrive
	${HOST_SRC_FILES}
	${BMP_SRC_FILES}
	${FLILO_SRC_FILES}
	${GIF_SRC_FILES}
	${PCX_SRC_FILES}
	${RIF_SRC_FILES}
	${TARGA_SRC_FILES})
