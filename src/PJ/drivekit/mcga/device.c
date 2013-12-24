
#include "mcga.h"

struct rastlib *mcga_get_rlib(Vdevice *dev, int mode, Raster *r);
void set_vmode(int);
short get_vmode(void);

mcga_detect(Vdevice *vd)
{
int oldvm;
Errcode err;

oldvm = get_vmode();
set_vmode(0x13 | 0x80);
if ((get_vmode()&0x7f) == 0x13)
	err = Success;
else
	err = Err_no_display;
set_vmode(oldvm | 0x80);
return(err);
}

static int ivmode;

static Vmode_info mcga_infos[] =
{
	{
	sizeof(Vmode_info),
	0,	/* mode ix */
	"Vanilla MCGA/VGA",
	8,	/* bits */
	1,	/* planes */
	{320,320,320,1},
	{200,200,200,1},
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1,				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	320*200,		/* display bytes */
	320*200,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	1000000U/70U,	/* field rate 1/70th sec */
	0,				/* vblank period */
	},
};

static Errcode mcga_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
	if (mode >= sizeof(mcga_infos)/sizeof(mcga_infos[0]))
		return(Err_no_such_mode);
	*pvm = mcga_infos[mode];
	return(Success);
}

static char *mcga_mode_text(Vdevice *driver, USHORT mode)
{
char *s = "Vanilla VGA/MCGA 256 color 320x200 driver.";
return(s);
}

static Errcode mcga_open_graphics(Vdevice *driver, McgaRast *r,
	LONG width, LONG height, USHORT mode)
{
if (width != 320 || height != 200)
	return(Err_wrong_res);
r->pdepth = 8;
r->type = mode + driver->first_rtype;
r->lib = mcga_get_rlib(driver,mode,r);
r->aspect_dx = 6;
r->aspect_dy = 5;
r->width = width;
r->height = height;
r->hw.bm.segment = VGA_SEG;
r->hw.bm.num_planes = 1;
r->hw.bm.bpr = 320;
r->hw.bm.psize = 320*200;
r->hw.bm.bp[0] = VGA_SCREEN;
ivmode = get_vmode()&0x7f;
set_vmode(0x13);
if (get_vmode() != 0x13)
	return(Err_no_display);
return(Success);
}

static Errcode mcga_close_graphics(Vdevice *driver)
/* Get out of graphics mode and back to text mode.	Note that this
 * function is called by the PJ screen size menu without a corresponding
 * open_graphics call, so we need to check that we've made it into
 * graphics mode before proceeding */
{
	if (ivmode > 0)
		set_vmode(ivmode&0xff7f);
	ivmode = 0;
	return(Success);
}

struct vdevice_lib mcga_device_library = {
	mcga_detect,	/* detect - Is our hardware attatched? */
	mcga_get_modes, /* get modes */
	mcga_mode_text, /* get extra info */
	NOFUNC, /* set max height for width.  */
	mcga_open_graphics, 	/* open_graphics */
	mcga_close_graphics,	/* close_graphics */
	NOFUNC, /* open_cel */
	NOFUNC, 	/* show_rast */
};

Vdevice rexlib_header = {
	{ REX_VDRIVER, VDEV_VERSION, NOFUNC,NOFUNC,NULL },
	0,									  /* first_rtype set by host */
	sizeof(mcga_infos)/sizeof(mcga_infos[0]), /* num_rtypes */
	sizeof(mcga_infos)/sizeof(mcga_infos[0]), /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&mcga_device_library,			/* lib */
	NULL,			/* grclib set by host */
	NUM_LIB_CALLS,	/* rast_lib_count */
};
