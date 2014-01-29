set(FONTLIB_SRC_FILES
	amifonts.c
	bit2alph.c
	cfont.c
	fontdev.c
	hpjet.c
	vfont.c
	stfont.c
	sail.c
	sixhi.c
	wwcountl.c
	wwcstart.c
	wwjustif.c
	wwline.c
	wwtext.c
	type1.c
	)

add_library(fontlib ${FONTLIB_SRC_FILES})
