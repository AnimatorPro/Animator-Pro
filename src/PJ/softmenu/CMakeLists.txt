set(SOFTMENU_SRC_FILES
	dstrgets.c
	dstring.c
	keyequiv.c
	loadpull.c
	loadstri.c
	namestri.c
	pullfmt.c
	serrtext.c
	snamestr.c
	sofpanel.c
	softmenu.c
	softok.c
	)

add_library(softmenu ${SOFTMENU_SRC_FILES})

target_link_libraries(softmenu poco)
