/* alt.c - A few functions for the alt screen */

#include "jimk.h"
#include "alt.h"
#include "errcodes.h"
#include "fli.h"
#include "pentools.h"

static void free_alt(void)
{
	pj_rcel_free(vl.alt_cel);
	vl.alt_cel = NULL;
	pj_delete(alt_name);
}

void qfree_alt(void)
{
	if(soft_yes_no_box("alt_delete"))
	{
		free_alt();
		set_trd_maxmem();
	}
}

void grab_alt(void)
{
	free_alt();
	if(NULL == (vl.alt_cel = clone_any_rcel(vb.pencel)))
		softerr(Err_no_memory,"alt_alloc");
}

void swap_alt(void)
{
	if (vl.alt_cel)
	{
		swap_pencels(vl.alt_cel, vb.pencel);
		see_cmap();
		zoom_it();
		dirties();
	}
}

