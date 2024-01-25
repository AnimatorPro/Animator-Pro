/* init_dos.c */

#include <assert.h>
#include "aaconfig.h"
#include "input.h"
#include "resource.h"

extern char key_idriver_name[], mouse_idriver_name[], summa_idriver_name[];

static char *idr_names[] = {
	mouse_idriver_name, 
	summa_idriver_name,
	vconfg.idr_name
};

/* Function: init_input
 *
 *  Called on startup to initialize the mouse and other input.
 */
Errcode
init_input(void)
{
	Errcode err;
	char idr_path[PATH_SIZE];
	char *idr_name;

	idr_name = idr_names[vconfg.dev_type];
	make_resource_name(idr_name, idr_path);

	err = init_idriver(idr_path,vconfg.idr_modes, vconfg.comm_port);
	if (err < Success) {
		init_idriver(key_idriver_name, vconfg.idr_modes, 0);
	}

	/* setup input to current icb.input_screen loaded by open_wscreen() */

	reset_input();
	enable_textboxes();

	if (err < Success) {
		softerr(err,"!%s","nomouse", idr_name );
	}
	return Success;
}
