set(WNDOLIB_SRC_FILES
	marqi.c
	mucolors.c
	rast1lib.c
	textbox.c
	waitwndo.c
	wndo.c
	wndoclip.c
	wndolib.c
	wscreen.c
	)

add_library(wndolib ${WNDOLIB_SRC_FILES})
target_link_libraries(wndolib
	gfxlib
	raster
	trdfile)
