#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "cmap.h"
#include "libdummy.h"
#include "rastlib.h"
#define VDEV_INTERNALS
#include "vdevice.h"

/* mcga driver name */
char pj_mcga_name[] = "=MCGA.DRV";  /* leading comma, a non-DOS name! */

static Errcode mcga_close_rast(McgaRast *r)
{
	return(Success);
}

extern void pj_mcga_put_dot(Bytemap *v, Pixel color, Coor x, Coor y);
extern void pj__mcga_put_dot(Bytemap *v, Pixel color, Coor x, Coor y);
extern void pj_mcga_get_dot(Bytemap *v, Pixel color, Coor x, Coor y);

void mcga_get_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);
void pj__mcga_get_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);
void mcga_put_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);
void pj__mcga_put_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);

void pj_mcga_set_colors(Bytemap *v, int startix, int count, Rgb3 *cmap);
void pj_mcga_uncc64(Bytemap *v, void *cbuf);
void pj_mcga_uncc256(Bytemap *v, void *cbuf);

static void mcga_wait_vsync(Raster *r)
{
	pj_wait_vsync();
}


/******* Device Level Functions *********/
static ivmode;

static Errcode mcga_detect(Vdevice *vd)
{
int oldvm;
Errcode err;

	oldvm = pj_get_vmode();
	pj_set_vmode(0x13 + 0x80);
	if ((pj_get_vmode()&0x7f) == 0x13)
		err = Success;
	else
		err = Err_no_display;
	pj_set_vmode(oldvm + 0x80);
	return(err);
}

static Vmode_info mcga_infos[] =
{
	{
	sizeof(Vmode_info),
	0,	/* mode ix */
	"Vanilla MCGA/VGA",
	8,	/* bits */
	1,	/* planes */
	{320,320,320,1},	/* width */
	{200,200,200,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	0, 				/* inner_type */
	1,				/* display pages */
	1,				/* store pages */
	320*200,		/* display bytes */
	320*200,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70,				/* Vsync rate */
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
char *s = "Built-in driver for MCGA or VGA displays 320x200 256 color mode.";
	return(s);
}

Errcode pj_mcga_open_graphics(Vdevice *vd, Raster *r,
						   LONG w, LONG h, USHORT mode)
{
	if (w != 320 || h != 200)
		return(Err_wrong_res);

	ivmode = pj_get_vmode();
	pj_set_vmode(0x13);
	if (pj_get_vmode() != 0x13)
		return(Err_no_display);

	pj_open_mcga_raster(r);
	r->type = vd->first_rtype;
	return(Success);
}

static void mcga_close_graphics(Vdevice *vd)
/* deallocates and resets resources used by mcga driver */
{
	if (ivmode > 0)
		pj_set_vmode(ivmode);
	ivmode = -1;
}

struct vdevice_lib pj_mcga_device_library = {
	mcga_detect, /* detect - Is our hardware attatched? */
	mcga_get_modes,
	mcga_mode_text,			/* mode_text() */
	pj_errdo_success,	/* set max width for height. Simple for us. Stays at 200 */
	pj_mcga_open_graphics,
	mcga_close_graphics,
	pj_errdo_unimpl, /* open_cel */
	pj_errdo_unimpl, /* show_rast */ 
};

static Vdevice mcga_driver = {
	{ REX_VDRIVER, VDEV_VERSION, NULL, NULL, NULL, NULL }, /* hdr */
	0, 	  /* first_rtype */
	1,	  /* num_rtypes */	
	1, 	  /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&pj_mcga_device_library, 			/* lib */
	NULL, 			/* grclib */
	NUM_LIB_CALLS, 	/* rast_lib_count */
};

static Errcode mcga_get_driver(Vdevice **pvd,char *name)
{
	mcga_driver.hdr.init = pj_errdo_success;
	*pvd = &mcga_driver;
	return(Success);
}
Errcode pj_open_mcga_vdriver(Vdevice **pvd)
{
	return(pj__vdr_initload_open(mcga_get_driver,pvd,pj_mcga_name));
}
