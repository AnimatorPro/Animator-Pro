
#include <stdio.h>
#include "null.h"
#include "errcodes.h"

int nout;


struct rastlib *null_get_rlib(Vdevice *dev, int mode, NullRast *r);

Errcode null_detect(Vdevice *dev)
{
	printf("null_detect()\n");
	return(Success);
}

static Vmode_info null_infos[] =
{
	{
	sizeof(Vmode_info),
	0,	/* mode ix */
	"NULL driver",
	8,	/* bits */
	1,	/* planes */
	{640,640,640,1},	/* fixed width */
	{480,480,480,1},	/* fixed height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	640*480,		/* display bytes */
	640*480,		/* storage bytes */
	FALSE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70,				/* Vsync rate */
	},
	{
	sizeof(Vmode_info),
	1,	/* mode ix */
	"NULL driver",
	8,	/* bits */
	1,	/* planes */
	{100,320,320,1},	/* variable-width */
	{100,200,200,1},	/* variable-height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	0, 				/* inner_type */
	1,				/* display pages */
	1,				/* store pages */
	320*200,		/* display bytes */
	320*200,		/* storage bytes */
	FALSE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate */
	0,				/* vblank period */
	},
};

static Errcode null_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
	printf("null_get_modes(mode = %d)\n", mode);
	if (mode >= sizeof(null_infos)/sizeof(null_infos[0]))
		return(Err_no_such_mode);
	*pvm = null_infos[mode];
	return(Success);
}

static char *null_mode_text(Vdevice *driver, USHORT mode)
{
char *mtext[] = {
	"\"Hi res\" NULL driver.  A bit bucket of a display.  If you select this"
		" from Animator you'll have to delete aa.cfg to recover.  Meant for"
		" display driver developers as an example.",
	"\"Lo res\" NULL driver.  A bit bucket of a display.  If you select this"
		" from Animator you'll have to delete aa.cfg to recover.  Meant for"
		" display driver developers as an example.",
	};
	printf("null_get_text(mode = %d)\n", mode);
	if (mode >= sizeof(null_infos)/sizeof(null_infos[0]))
		return(NULL);

	return(mtext[mode]);
}

int null_set_max_height(Vdevice *vd, Vmode_info *vm)
{
int height;

printf("null_set_max_height()\n");
if ((height = vm->display_bytes/vm->width.actual) <= 0)
	return(Err_no_vram);
if (height > vm->height.max)
	height = vm->height.max;
height -= height%vm->height.grain;
if (height < vm->height.min)
	return(Err_too_big);
return(height);
}

Errcode null_open_graphics(Vdevice *dev, NullRast *r, 
						   LONG width, LONG height, USHORT mode)
{
Errcode err;

printf("null_open_graphics(width=%d, height=%d, mode=%d)\n",
	width, height, mode);
if (mode >= sizeof(null_infos)/sizeof(null_infos[0]))
	return(Err_no_such_mode);

if ((err = null_detect(dev)) == Success)
	{
	r->lib = null_get_rlib(dev, mode, r);
	r->type = mode + dev->first_rtype;
	if (width < null_infos[mode].width.min || 
		width > null_infos[mode].width.max ||
		height < null_infos[mode].height.min || 
		height > null_infos[mode].height.max)
		return(Err_wrong_res);
	r->width = width;
	r->height = height;
	r->pdepth = null_infos[mode].bits;
	r->aspect_dx = 3;
	r->aspect_dy = 2;
	r->hw.nm.dev = dev;
	r->hw.nm.puts = r->hw.nm.gets = r->hw.nm.colors = 0;
	}
return(err);
}

static Errcode null_close_graphics(Vdevice *driver)
{
printf("null_close_graphics()\n");
return(Success);
}

struct vdevice_lib null_device_library = {
	null_detect, /* detect - Is our hardware attatched? */
	null_get_modes,	/* get modes */
	null_mode_text,	/* get extra info */
	null_set_max_height,/* set max height for width.  */
	null_open_graphics,	/* open_graphics */
	null_close_graphics,	/* close_graphics */
	NOFUNC,	/* open_cel */
	NOFUNC, /* show_rast */
};

Hostlib _a_a_stdiolib = { NULL,AA_STDIOLIB,AA_STDIOLIB_VERSION};
Hostlib _a_a_syslib = { &_a_a_stdiolib, AA_SYSLIB, AA_SYSLIB_VERSION };


Vdevice rexlib_header = {
	{ REX_VDRIVER, 0, NOFUNC,NOFUNC,&_a_a_syslib},
	0, 									  /* first_rtype */
	sizeof(null_infos)/sizeof(null_infos[0]), /* num_rtypes */
	sizeof(null_infos)/sizeof(null_infos[0]), /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&null_device_library, 			/* lib */
	NULL, 			/* grclib */
	NUM_LIB_CALLS, 	/* rast_lib_count */
};
