cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(ani C)

if(APPLE)
	enable_language(OBJC)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

include_directories("inc")

if (UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wmissing-prototypes")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wold-style-definition")
endif()

add_subdirectory(fileio)
add_subdirectory(fli)
add_subdirectory(font)
add_subdirectory(gfx)
add_subdirectory(input)
add_subdirectory(menus)
add_subdirectory(picdrive)
add_subdirectory(pjhigh)
add_subdirectory(pj_sdl)
add_subdirectory(rastlib)
add_subdirectory(rexlib)
add_subdirectory(softmenu)
add_subdirectory(util)
add_subdirectory(wndo)

if (WITH_POCO)
	add_subdirectory(poco)
	set(PJ_POCO_FILES
		pocodraw.c
		pocotur.c
		qpoco.c
		qpocoed.c
	)
else()
	## files still needed for softmenu parsing
	set(PJ_POCO_FILES
		poco/token.c
		poco/bcutil.c
	)
endif()

if (UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-sign")
endif()

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
	status.c
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

if (WITH_ANI)
	add_executable(
		${PROJECT_NAME}
			${PJMAIN_SRC_FILES}
			${PJ_POCO_FILES}
	)
	
	target_link_libraries(
		${PROJECT_NAME}
			trdfile
			flilib
			fontlib
			gfxlib
			macinput
			menulib
			picdrive
			pjhigh
			pj_sdl
			raster
			minirex
			softmenu
			trdutil
			wndolib
			SDL2-static
			SDL2main
			nfd
			m
	)
	
	if (WITH_POCO)
		target_link_libraries(${PROJECT_NAME} libpoco)
	endif()
	
	if (CMAKE_GENERATOR STREQUAL "Xcode")
		set_target_properties(
			${PROJECT_NAME}
			PROPERTIES
				MACOSX_BUNDLE YES
				XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
				XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.vpaint.animator-pro"
		)
	
		if (PLATFORM STREQUAL "OS64")
			set_target_properties(
				${PROJECT_NAME}
				PROPERTIES
					XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" ## iPhone / iPad
			)
		endif()
	
			## pre-build required or signing will fail
		add_custom_command(
			TARGET ${PROJECT_NAME}
			PRE_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_CURRENT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources"
		)
	else()
		INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ./)
		INSTALL(DIRECTORY resource/ DESTINATION ./resource)
	endif()
	
endif()

