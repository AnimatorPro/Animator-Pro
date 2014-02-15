set(MENU_SRC_FILES
	choices.c
	feelme.c
	icb_boxf.c
	icbcontb.c
	icbvbool.c
	icbvcont.c
	icbvwait.c
	menus.c
	pull.c
	pullid.c
	pullsee.c
	qnumber.c
	qscroll.c
	qstring.c
	scroller.c
	seeme.c
	seepath.c
	slider.c
	stringq.c
	titlebar.c
	wndoio.c
	)

add_library(menulib ${MENU_SRC_FILES})
