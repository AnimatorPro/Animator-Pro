/* device4.c - 
 * 	device portion of a driver that divides VRAM 640x400 screen into
 *  4 320x200 sub-screens.  PJ will think the screen is just 320x200,
 *  and that there are 3 off-screen buffers */

#include <stdio.h>
#include "vram4.h"
#include "errcodes.h"

#define RASTER0   0
#define RASTER1	  1
#define RASTER2	  2
#define RASTER3   3

#define NOMODE 0

extern void *vram_get_rlib(void);


static short raster2_inuse = FALSE;

static ivmode = NOMODE;

static Ytable vram_ytable[400];

static struct rastinfo {
	USHORT  inuse;
	USHORT  xoff;
	USHORT	yoff;
	} raster_info[4] = {
	{0,   0,   0},
	{0, 320,   0},
	{0,   0, 200},
	{0, 320, 200},
	};


static Vmode_info vram_infos[] =
{
	{
	sizeof(Vmode_info),
	0,					/* mode ix */
	"V7 QuadScreen",    /* mode name */
	8,					/* bits */
	1,					/* planes */
	{320,320,320,1},	/* width */
	{200,200,200,1},	/* height */
	TRUE, TRUE, 		/* read and write */
	TRUE,				/* Has viewable screen suitable for menus etc */
	1, 					/* fields_per_frame */
	1,					/* display pages */
	4,					/* store pages */
	320*200,			/* display bytes */
	320*200*4,			/* storage bytes */
	TRUE,				/* Palette set's only allowed during vblank */
	0,					/* Swap screen during vsync. (We can't swap screens even)*/
	70,					/* Vsync rate */
	},
};

/* convert our mode to vram extended mode */
static int mode_to_mode[] = { 0x66, 0x67, 0x68, 0x69};

static char *mtext[] = {
	"V7 Quad - four 320x200 screens on one 640x400 screen",
	};

static Errcode vram_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
	if (mode >= sizeof(vram_infos)/sizeof(vram_infos[0]))
		return(Err_no_such_mode);
	*pvm = vram_infos[mode];
	return(Success);
}

static char *vram_mode_text(Vdevice *driver, USHORT mode)
{
	if (mode >= sizeof(vram_infos)/sizeof(vram_infos[0]))
		return(NULL);
	return(mtext[mode]);
}


Errcode vram_detect(Vdevice *dev)
{
if (isit_v7())
	return(Success);
else
	return(Err_no_display);
}

static void build_ytable(void)
{
int    y;
long   apt;
long   napt;
int    bpr = 640;
Ytable *ptab = vram_ytable;

apt = 0;	
for (y=0; y<400; y++, ptab++)
	{
	napt = apt + bpr - 1;
	ptab->address = (apt&0xffff)+0xa0000;
	ptab->bank = apt>>16;
	if ((napt^apt)&0xf0000)	/* if bank not same */
		{
		ptab->split_at = bpr-1-(napt&0xffff);
		}
	else
		ptab->split_at = 0;
	apt += bpr;
	}
}

static void open_raster(Vdevice *dev,
						USHORT 	mode,
						Raster 	*pjr,
						LONG 	width,
						LONG	height,
						USHORT	raster_number)
/*****************************************************************************
 *
 ****************************************************************************/
{
VramRast *r = (VramRast *)pjr;

r->lib 			= vram_get_rlib();
r->type 		= mode + dev->first_rtype;
r->width 		= width;
r->height 		= height;
r->pdepth 		= vram_infos[mode].bits;
r->aspect_dx 	= 6;
r->aspect_dy 	= 5;

r->hw.vm.bpr 	 = 640;
r->hw.vm.ytable  = vram_ytable;
r->hw.vm.rastnum = raster_number;
r->hw.vm.xoff 	 = raster_info[raster_number].xoff;
r->hw.vm.yoff 	 = raster_info[raster_number].yoff;
}

static USHORT vram_graphics_mode;

Errcode vram_open_graphics(Vdevice *dev, VramRast *r,
							LONG width, LONG height, USHORT mode)
{
Errcode err;
int vmode;

if (mode >= sizeof(vram_infos)/sizeof(vram_infos[0]))
	return(Err_no_such_mode);
if (width != vram_infos[mode].width.actual ||
	height != vram_infos[mode].height.actual )
	return(Err_wrong_res);
if ((err = vram_detect(dev)) == Success)
	{
	vmode = mode_to_mode[mode];
	ivmode = get_xmode();
	set_xmode(vmode);
	if (get_xmode() != vmode)
		err = Err_no_vram;
	else
		{
		build_ytable();
		enable_ext();
		open_raster(dev, mode, r, width, height, RASTER0);
		vram_graphics_mode = mode;
		}
	}
return(err);
}

static Errcode open_cel(Vdevice *dev, Raster *r,
						LONG width, LONG height, 
						USHORT pixel_depth, USHORT displayable)
/*****************************************************************************
 *
 ****************************************************************************/
{
SHORT rastnum;

	/* do a bunch of error checking first */
	if (pixel_depth != 8)	/* can only handle 8-bit-a-pixel */
		return Err_pdepth_not_avail;
	if (displayable)		/* can't handle extra displayable screens */
		return Err_no_vram;
	if (width > 320 || height > 200)	/* no bigger than 320x200 */
		return Err_too_big;

	for (rastnum = RASTER1; rastnum <= RASTER3; rastnum++)
		if (raster_info[rastnum].inuse == 0)
			{
			raster_info[rastnum].inuse = 1;
			open_raster(dev, vram_graphics_mode, r, width, height, rastnum);
			return Success;
			}
return Err_no_vram;
}

void vq_close_raster(VramRast *r)
/* Mark raster as free to use again */
{
raster_info[r->hw.vm.rastnum].inuse = FALSE;
}


static Errcode vram_close_graphics(Vdevice *driver)
{
if (ivmode != NOMODE)
	{
	disable_ext();
	set_xmode(ivmode);
	ivmode = NOMODE;
	}
return(Success);
}

struct vdevice_lib vram_device_library = {
	vram_detect, 			/* detect - Is our hardware attatched? */
	vram_get_modes,			/* get modes */
	vram_mode_text,			/* get extra info */
	NOFUNC,					/* set max width for height.  Since we're always 400... */
	vram_open_graphics,		/* open_graphics */
	vram_close_graphics,	/* close_graphics */
	open_cel, 				/* open_cel */
	NOFUNC, 				/* show_rast */
};

Hostlib _a_a_stdiolib = {NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION|HLIB_OPTIONAL};

Vdevice rexlib_header = {
	{ REX_VDRIVER, VDEV_VERSION, NOFUNC,NOFUNC,&_a_a_stdiolib},
	0, 									  /* first_rtype */
	sizeof(vram_infos)/sizeof(vram_infos[0]), /* num_rtypes */
	sizeof(vram_infos)/sizeof(vram_infos[0]), /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&vram_device_library, 			/* lib */
	NULL, 			/* grclib */
	NUM_LIB_CALLS, 	/* rast_lib_count */
};

