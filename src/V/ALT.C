
/* alt.c - A few functions for the alt screen */

#include "jimk.h"


free_alt()
{
free_screen(alt_form);
alt_form = NULL;
jdelete(alt_name);
}

grab_alt()
{
free_screen(alt_form);
alt_form = clone_screen(render_form);
}

swap_alt()
{
if (alt_form)
	{
	exchange_form(alt_form, render_form);
	see_cmap();
	zoom_it();
	dirties();
	}
}

