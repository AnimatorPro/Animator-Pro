add_definitions(-DINPUT_MACROS)

set(INPUTHI_SRC_FILES
	idrivcfg.c
	idriver.c
	inwaitup.c
	inwclick.c
	inwpenup.c
	inwtany.c
	keyidr.c
	mouseidr.c
	reponpdn.c
	waitask.c
	)

set(MACINPUT_SRC_FILES
	input.c
	)

## input library for pj with loadable input drivers and macro handling
add_library(macinput
	${INPUTHI_SRC_FILES}
	${MACINPUT_SRC_FILES})
