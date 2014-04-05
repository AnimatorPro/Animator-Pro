/* init_sdl.c */

#define SCRNINIT_CODE
#define VDEV_INTERNALS
#include <SDL/SDL.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include "jimk.h"
#include "aaconfig.h"
#include "argparse.h"
#include "dpmiutil.h"
#include "input.h"
#include "msfile.h"
#include "progids.h"
#include "rastcurs.h"
#include "rastlib.h"
#include "resource.h"
#include "vdevcall.h"

short pj_crit_errval = 1 - 19;

static char default_config_name[] = "aa.cfg";
char pj_mcga_name[] = "=SDL.DRV";

Doserr
pj_dget_err(void)
{
	return -1;
}

void
new_config(void)
{
}

/*--------------------------------------------------------------*/

Errcode
init_pj_startup(Argparse_list *more_args, Do_aparse do_others,
		int argc, char **argv, char *help_key, char *menufile_name)
{
	Boolean force_config;
	Errcode err;
	(void)more_args;
	(void)do_others;
	(void)argc;
	(void)argv;
	(void)help_key;

	vb.ivmode = -1;

	init_stdfiles();
	init_scodes();
	init_mem(0);

	/* init_resource_path(char *path); */
	snprintf(resource_dir, sizeof(resource_dir), "resource/");

	if ((err = init_menu_resource(menufile_name)) < Success)
		return err;

	if (vb.config_name == NULL) {
		force_config = TRUE;
		vb.config_name = default_config_name;
	}
	else {
		force_config = FALSE;
	}

	if ((err = init_config(force_config)) < Success)
		return err;

	return Success;
}

Errcode
open_pj_startup_screen(Errcode (*init_with_screen)(void *data), void *data)
{
	Screen_mode *open_mode;
	Screen_mode *alt_mode;

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	open_mode = &vconfg.smode;
	alt_mode = NULL;

	return init_screen(open_mode, alt_mode, init_with_screen, data);
}

void
cleanup_startup(void)
{
}
