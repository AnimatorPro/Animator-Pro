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


/******* Device Level Functions *********/
static int ivmode;

static Errcode mcga_detect(Vdevice *vd)
{
int oldvm;
Errcode err;

	oldvm = pj_get_vmode();
	pj_set_vmode(0x13 | 0x80);
	if ((pj_get_vmode()&0x7f) == 0x13)
		err = Success;
	else
		err = Err_no_display;
	pj_set_vmode(oldvm | 0x80);
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
	0,				/* inner_type */
	1,				/* display pages */
	1,				/* store pages */
	320*200,		/* display bytes */
	320*200,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70, 			/* Vsync rate */
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
/* note that the old vmode is ANDed with 0xff7f.  DOS will have returned */
/* the vmode earlier with the 0x80 bit set if the prior modeset call included */
/* that bit, and in our case it always will, because our detect routine */
/* uses that bit.  but when closing the mcga device, we always want the */
/* screen-clear to occur, because we may be returning to a character mode. */
/* (for some reason this only affects the flilib code!) */
{
	if (ivmode > 0)
		pj_set_vmode(ivmode&0xff7f);
	ivmode = -1;
}

struct vdevice_lib pj_mcga_device_library = {
	mcga_detect, /* detect - Is our hardware attatched? */
	mcga_get_modes,
	mcga_mode_text, 		/* mode_text() */
	NOFUNC, /* set max width for height. Simple for us. Stays at 200 */
	pj_mcga_open_graphics,
	mcga_close_graphics,
	NOFUNC, /* open_cel */
	NOFUNC, /* show_rast */
};

static Vdevice mcga_driver = {
	{ REX_VDRIVER, VDEV_VERSION, NULL, NULL, NULL, NULL }, /* hdr */
	0,	  /* first_rtype */
	1,	  /* num_rtypes */
	1,	  /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&pj_mcga_device_library,			/* lib */
	NULL,			/* grclib */
	NUM_LIB_CALLS,	/* rast_lib_count */
};

/*****************************************************************************
 * The FLILIB code needs a local_vdevice pointer as a global variable.	In
 * PJ, we have a couple interface routines that serve a similar purpose.
 * The follow conditional sections handle the two situations.  The macro
 * FLILIB_CODE is defined within the MAKE.INC file that does the compiles
 * in the FLILIB environment.
 ****************************************************************************/

#ifdef FLILIB_CODE

static struct local_vdevice localdev = {NULL, &mcga_driver};
struct local_vdevice *pj_vdev_mcga = &localdev;

#else

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

#endif /* FLILIB_CODE */
