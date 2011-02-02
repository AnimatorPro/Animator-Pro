#ifndef AACONFIG_H
#define AACONFIG_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

#ifndef FILEPATH_H
	#include "filepath.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

typedef struct screen_mode {
	char drv_name[FILE_NAME_SIZE];
	SHORT mode;
	SHORT width;
	SHORT height;
} Screen_mode;

#define VCFG_VERSION 2

typedef struct aa_config {
	Fat_chunk id;			/* Verify it's right file type magic number 
							 * size is size of whole file */
	Screen_mode smode;      /* screen mode */

	UBYTE noint;			/* Should we install clock interrupt? */
	UBYTE dev_type;			/* 0 = mouse, 1 = tablet */
	SHORT unused;	
	SHORT comm_port;		/* which comm port for a serial tablet */
	SHORT font_type;		/* what type of font is current */
	Rgb3 mc_ideals[6];      /* menu colors to try for */
	UBYTE reserved[8];
 	char idr_name[FILE_NAME_SIZE];	/* loadable input device name */
	UBYTE idr_modes[4]; 				/* input device mode settings */
	char picsave_pdr[FILE_NAME_SIZE];   /* current picture saving driver */
	char flisave_pdr[FILE_NAME_SIZE];   /* current fli saving driver */
	char temp_path[PATH_SIZE*2];		/* semicolon separated path */
} AA_config;

extern AA_config vconfg;		/* in pjhigh.lib(config.c) */


#endif /* AACONFIG_H */
