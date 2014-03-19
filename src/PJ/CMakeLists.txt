cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(pj C)

include_directories("inc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wold-style-definition")

add_subdirectory(fileio)
add_subdirectory(fli)
add_subdirectory(font)
add_subdirectory(gfx)
add_subdirectory(input)
add_subdirectory(menus)
add_subdirectory(picdrive)
add_subdirectory(pjhigh)
add_subdirectory(poco)
add_subdirectory(rastlib)
add_subdirectory(rexlib)
add_subdirectory(softmenu)
add_subdirectory(util)
add_subdirectory(wndo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-sign")

set(PJMAIN_SRC_FILES
	a3d.c
	a3ddat.c
	alt.c
	arctan.c
	auto.c
	autoseg.c
	blue.c
	broadcas.c
	browse.c
	brush.c
	celmenu.c
	celpaste.c
	celpull.c
	celtrans.c
	cfit.c
	cleanup.c
	cluster.c
	composit.c
	composmu.c
	csort.c
	cthread.c
	cursor.c
	cutcurs.c
	ellipse.c
	files.c
	findfree.c
	fli.c
	fli.c
	flicel.c
	flicelio.c
	flipath.c
	flood.c
	flxovlay.c
	freem.c
	gel.c
	geometry.c
	globals.c
	grid.c
	hitext.c
	hotkey.c
	inkcashe.c
	inkdot.c
	inkdot2.c
	inkopts.c
	macro.c
	main.c
	mainpull.c
	mask.c
	messages.c
	multimen.c
	muparts.c
	names.c
	onepal.c
	options.c
	overlay.c
	palet2.c
	pencel.c
	penopts.c
	pentools.c
	penwndo.c
	picfiles.c
	pjfont.c
	poly.c
	polyrub.c
	polytool.c
	pstamp.c
	qconfig.c
	qmacro.c
	qmenu.c
	quickdat.c
	random.c
	rcel.c
	redo.c
	render.c
	resource.c
	savefli.c
	saveseg.c
	selbrush.c
	sep.c
	sizemenu.c
	spiral.c
	spline.c
	tempflx.c
	tempname.c
	test.c
	textedit.c
	textfile.c
	thikline.c
	tileink.c
	time.c
	timemenu.c
	title.c
	truecolo.c
	tseg.c
	tween.c
	tweendat.c
	tweenhi.c
	tweenlo.c
	tweenmag.c
	vfeelme.c
	vmarqi.c
	vpaint.c
	vpsubs.c
	vsetfnam.c
	vsetting.c
	writeflx.c
	zoom.c
	)

add_executable(pj WIN32
	${PJMAIN_SRC_FILES})

target_link_libraries(pj
	trdfile
	flilib
	fontlib
	gfxlib
	macinput
	menulib
	picdrive
	pjhigh
	poco
	raster
	minirex
	softmenu
	trdutil
	wndolib)
