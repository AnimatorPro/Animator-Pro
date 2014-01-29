
#include "jimk.h"
#include "errcodes.h"
#include "fontdev.h"
#include "resource.h"
#include "softmenu.h"

Errcode init_pj_resources(void)
{
Errcode err;

	if((err = change_dir(resource_dir)) < Success)
		goto error;
	if((err = init_menu_parts()) < Success)
		goto error;
	init_font_dev();	/* set up font handling */
	change_dir(vb.init_drawer);
error:
	return(softerr(err,"resource_init"));
}
cleanup_resources()
{
	cleanup_menu_parts();
}
