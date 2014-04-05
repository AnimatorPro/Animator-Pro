add_definitions(-DINPUT_MACROS)

set(INPUTHI_SRC_FILES
	idrivcfg.c
	idriver.c
	inwaitup.c
	inwclick.c
	inwpenup.c
	inwtany.c
	reponpdn.c
	waitask.c
	)

set(MACINPUT_SRC_FILES
	input.c
	)

set(IDRIVER_SDL_SRC_FILES
	idr_sdl.c
	)

## input library for pj with loadable input drivers and macro handling
add_library(macinput
	${INPUTHI_SRC_FILES}
	${MACINPUT_SRC_FILES}
	${IDRIVER_SDL_SRC_FILES})
