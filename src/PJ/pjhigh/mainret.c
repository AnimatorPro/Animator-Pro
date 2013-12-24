#include "pjbasics.h"

void return_to_main(Errcode code)

/* closes all menus and sets code returned by lowest level menu or requestor
 * loop */
{
	close_all_menus(vb.screen,code);
}

