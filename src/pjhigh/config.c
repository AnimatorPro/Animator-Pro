
/* config.c - These routines manipulate the file v.cfg and act on the
   data contained there.  Configuration - state stuff that is semi-permanent
   in nature.  What input device to use, where to put temp files, what
   directory your fonts are in, what comm port to use for the serial
   summa tablet, whether it's safe to speed up the clock interrupt, 
   user interface file path settings. interface menu colors */

#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "aaconfig.h"
#include "errcodes.h"
#include "reqlib.h"
#include "tfile.h"

#include "pj_sdl.h"


static char default_config_name[] = "aa.cfg";

AA_config vconfg = {  /* Ram image of v.cfg containing default values
					* for startup if file not found */
	{ sizeof(AA_config), VCFG_MAGIC, VCFG_VERSION }, /* file size and id */
	{ "aadisp.drv", 0,0,0 },  /* screen_mode drv_name, mode, width, height */

	false, 		/* noint */
	0, 			/* dev_type */
	0, 			/* unused */
	0, 			/* comm_port */
	0, 			/* font_type */

	/* mc_ideals */
	{
		{0, 0, 0},	/* menu black */
		{22*4, 22*4, 22*4}, /* menu grey */
		{38*4, 38*4, 38*4}, /* menu light */
		{52*4, 52*4, 52*4}, /* menu bright */
		{63*4, 0, 0},  /* menu red */
	},
	/* all else is zeros and set by program */
};					

/* re opens config file for readwrite with global name. The path is assumed
 * to be full or is relative to the startup drawer */
static Errcode
open_config(XFILE **pxf, bool create)
{
	const char *config_name = vb.config_name;

	if (!pj_assert(vb.config_name != NULL)) return Err_abort;

	return xffopen(config_name, pxf,
			create ? XREADWRITE_CLOBBER : XREADWRITE_OPEN);
}


Errcode default_temp_path(char *buf)
{
	//!TODO: handle the different cases for temp paths
#ifdef SDL_PLATFORM_APPLE
	snprintf(buf, PATH_MAX, "%s", mac_preferences_path());
#else
	buf[0] = '.';
	buf[1] = '\0';
#endif
	return Success;
}


const char* get_default_config_name() {
	static char config_name[PATH_MAX];
	static int initialized = 0;

	if (!initialized) {
		default_temp_path(config_name);
		sprintf(config_name, "%s%s%s",
				config_name, SEP, default_config_name);
		initialized = 1;
	}

	return config_name;
}


/* Attempt to read v.cfg file and check file data.  If can't find file or bad
   data and force_create is true then assume default values and create a
   new file.  If can't create a new file or find it, it is a fatal error.
   Returns Success if a new config was created, 1 if
   it was read in < Success if a fatal error occurred and a config file could
   not be read or created. This reports the fatal error via softerr(). */
Errcode init_config(bool force_create)
{
	Errcode goodret;
	Errcode err;
	XFILE* xf;
	AA_config incfg;

	goodret = 1; /* optimism */

	err = open_config(&xf, false);
	if (err < Success)
		goto bad_open;

	if (xffread(xf, &incfg, sizeof(incfg)) < Success)
		goto bad_config;

	/* check magic and size fields */

	if (incfg.id.type != VCFG_MAGIC || incfg.id.size != sizeof(AA_config) ||
		incfg.id.version != VCFG_VERSION) {
		goto bad_config;
	}

	vconfg = incfg;
	goto done;

bad_open:
	if (!force_create) {
		if (!soft_yes_no_box("!%s", "noconfig", vb.config_name))
			goto reported;
	} else
		goodret = 0;

	goto create_it;

bad_config: /* we do not have a valid file. Create a new one? */
	xffclose(&xf);
	if (!force_create) {
		if (!soft_yes_no_box("!%s", "badconfig", vb.config_name))
			goto reported;
	}

create_it:
	err = open_config(&xf, true);
	if (err < Success)
		goto fatal_error;

	/* prepare default initial record to write */
	err = default_temp_path(vconfg.temp_path);
	if (err < Success)
		goto fatal_error;

	incfg = vconfg; /* defaults or good one read in (won't hurt) */

	/* create and fill paths list with zeros */
	err = xffwrite(xf, &incfg, sizeof(incfg));
	if (err < Success)
		goto fatal_error;

done:
	xffclose(&xf);
	err = set_temp_path(vconfg.temp_path);
	if (err < Success) {
		err = default_temp_path(vconfg.temp_path);
		if (err < Success)
			goto fatal_error;

		err = set_temp_path(vconfg.temp_path);
		if (err < Success)
			goto fatal_error;
	}

	fprintf(stderr, "+ Working temp path: %s\n", vconfg.temp_path);

	return goodret;

reported:
	err = Err_reported;

fatal_error:
	xffclose(&xf);
	vb.config_name = NULL; /* disable write/open ability */
	return (softerr(err, "!%s", "fatal_create", vb.config_name));
}


Errcode rewrite_config(void)
/* Copy ram image of configuration file header to config file in startup
 * directory only rewrites header */
{
	Errcode err;
	XFILE *xf;

	err = open_config(&xf, false);
	if (err >= Success) {
		err = xffwrite(xf, &vconfg, sizeof(vconfg));
		xffclose(&xf);
	}

	return err;
}
void cleanup_config(void)
{
}
