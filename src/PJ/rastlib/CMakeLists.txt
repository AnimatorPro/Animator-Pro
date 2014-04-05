## low level raster internal calls needed for just about anything that has
## any raster in it
set(RASTLOW_SRC_FILES
	cmapshif.c
	getplane.c
	grc_comp.c
	grcdcomp.c
	grcfulib.c
	grclib.c
	grcommon.c
	grcset.c
	libdummy.c
	nullinit.c
	nullrast.c
	nullrlib.c
	)

## items needed for a bytemap raster
set(BYMRAST_SRC_FILES
	bymalloc.c
	bymrast.c
	)

## independent bytemap raster utilities
set(BYMUTIL_SRC_FILES
	temprast.c
	)

## things needed for a bitmap raster does not require bytemap code
set(BITRAST_SRC_FILES
	bitrast.c
	bitalloc.c
	)

## clipbox code is called with another raster but only needs low level code
set(CBOX_SRC_FILES
	clipbox.c
	)

## raster call and clipping glue. really needed for most things since a
## raster is pretty useless without it.
set(RASTCALL_SRC_FILES
	rastcall.c
	rc_blire.c
	rc_swapr.c
	rc_tblit.c
	rcblirec.c
	rcclose.c
	rcfree.c
	rcghseg.c
	rcgrpix.c
	rcgvseg.c
	rclclip2.c
	rclclipd.c
	rclip2r.c
	rcmsk1bl.c
	rcmsk2bl.c
	rcphseg.c
	rcprpix.c
	rcpvseg.c
	rcsetrec.c
	rcshline.c
	rcsvline.c
	rcswapr.c
	rctblitr.c
	rcxorast.c
	rcxorect.c
	rczoombl.c
	)

## code needed to load drivers this will have to be linked with a syslib
## menulib etc to be useable
set(VDRIVER_SRC_FILES
	vdrclose.c
	vdgetmod.c
	vdrvinfo.c
	vdverify.c
	vdrasopn.c
	vdscropn.c
	)

set(VDRIVER_SDL_SRC_FILES
	vdr_sdl.c
	)

add_library(raster
	${BITRAST_SRC_FILES}
	${BYMRAST_SRC_FILES}
	${BYMUTIL_SRC_FILES}
	${CBOX_SRC_FILES}
	${RASTCALL_SRC_FILES}
	${RASTLOW_SRC_FILES}
	${VDRIVER_SRC_FILES}
	${VDRIVER_SDL_SRC_FILES})
