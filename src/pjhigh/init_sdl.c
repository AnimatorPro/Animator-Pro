/* init_sdl.c */

#define SCRNINIT_CODE
#define VDEV_INTERNALS
#include <SDL3/SDL.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef _MSC_VER
	#include <unistd.h>
#endif

#include "aaconfig.h"
#include "argparse.h"
#include "dpmiutil.h"
#include "input.h"
#include "jimk.h"
#include "msfile.h"
#include "pj_sdl.h"
#include "progids.h"
#include "rastcurs.h"
#include "rastlib.h"
#include "resource.h"
#include "vdevcall.h"

short pj_crit_errval = 1 - 19;

char pj_mcga_name[] = "=SDL.DRV";

Doserr pj_dget_err(void)
{
	return -1;
}

void new_config(void)
{
}

/*--------------------------------------------------------------*/
static int dir_exists(const char *const path)
{
	struct stat info;

	int statRC = stat(path, &info);
	if (statRC != 0) {
		if (errno == ENOENT) {
			return 0;
		}  // something along the path does not exist
		if (errno == ENOTDIR) {
			return 0;
		}  // something in path prefix is not a dir
		return -1;
	}

	return (info.st_mode & S_IFDIR) ? 1 : 0;
}

/*--------------------------------------------------------------*/

Errcode init_pj_startup(Argparse_list *more_args, Do_aparse do_others, int argc, char **argv,
						char *help_key, char *menufile_name)
{
	bool force_config;
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

	char resource_paths[3][PATH_MAX];
	int resource_paths_count = 2;
	getcwd(resource_paths[1], PATH_MAX);

	snprintf(resource_paths[0], PATH_MAX, pj_sdl_resources_path());
	snprintf(resource_paths[1], PATH_MAX, "%s/resource", resource_paths[1]);

	err = Failure;

	for (int i = 0; i < resource_paths_count; i++) {
		if (dir_exists(resource_paths[i])) {
			snprintf(resource_dir, PATH_MAX, resource_paths[i]);
			err = init_menu_resource(menufile_name);
			if (err == Success) {
				fprintf(stderr, "+ Resources folder: %s\n", resource_dir);
				break;
			}
		}
	}

	if (err < Success) {
		return err;
	}

	if (vb.config_name == NULL) {
		force_config = true;
		vb.config_name = get_default_config_name();
	} else {
		force_config = false;
	}

	err = init_config(force_config);
	if (err < Success) {
		return err;
	}

	return Success;
}

Errcode open_pj_startup_screen(Errcode (*init_with_screen)(void *data), void *data)
{
	Screen_mode *open_mode;
	Screen_mode *alt_mode;

	Uint32 flags = (SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
					SDL_INIT_GAMEPAD | SDL_INIT_EVENTS);

	SDL_Init(flags);

	open_mode = &vconfg.smode;
	alt_mode = NULL;

	return init_screen(open_mode, alt_mode, init_with_screen, data);
}

void cleanup_startup(void)
{
}
