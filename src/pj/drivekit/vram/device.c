#include <stdio.h>
#include "vram.h"
#include "errcodes.h"

void *vram_get_rlib(void);


Errcode detect()
{
if (isit_v7())
	return(Success);
else
	return(Err_no_display);
}

Errcode vram_detect(Vdevice *dev)
{
return detect();
}

static Vmode_info vram_infos[] =
{
	{
	sizeof(Vmode_info),
	0,	/* mode ix */
	"Video 7 VRAM VGA",
	8,	/* bits */
	1,	/* planes */
	{640,640,640,1},	/* width */
	{400,400,400,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	640*400,		/* display bytes */
	640*400,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70,				/* Vsync rate */
	},
	{
	sizeof(Vmode_info),
	1,	/* mode ix */
	"Video 7 VRAM VGA",
	8,	/* bits */
	1,	/* planes */
	{640,640,640,1},	/* width */
	{480,480,480,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	640*480,		/* display bytes */
	640*480,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate */
	0				/* vblank period */
	},
	{
	sizeof(Vmode_info),
	2,	/* mode ix */
	"Video 7 VRAM VGA",
	8,	/* bits */
	1,	/* planes */
	{720,720,720,1},	/* width */
	{540,540,540,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	720*540,		/* display bytes */
	720*540,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate */
	0				/* vblank period */
	},
	{
	sizeof(Vmode_info),
	3,	/* mode ix */
	"Video 7 VRAM VGA",
	8,	/* bits */
	1,	/* planes */
	{800,800,800,1},	/* width */
	{600,600,600,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,				/* display pages */
	1,				/* store pages */
	800*600,		/* display bytes */
	800*600,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate */
	0				/* vblank period */
	},
};
/* convert our mode to vram extended mode */
static int mode_to_mode[] = { 0x66, 0x67, 0x68, 0x69};
static int xaspects[] 	  = { 6, 1, 1, 1};
static int yaspects[]	  = { 5, 1, 1, 1};
static Errcode vram_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
	if (mode >= sizeof(vram_infos)/sizeof(vram_infos[0]))
		return(Err_no_such_mode);
	*pvm = vram_infos[mode];
	return(Success);
}
static char *mtext[] = {
	RL_KEYTEXT("V7VGA_0")"Video 7 640x400.",
	RL_KEYTEXT("V7VGA_1")"Video 7 640x480.",
	RL_KEYTEXT("V7VGA_2")"Video 7 720x540.",
	RL_KEYTEXT("V7VGA_3")"Video 7 800x600.",
	};
static char *vram_mode_text(Vdevice *driver, USHORT mode)
{
	if (mode >= sizeof(vram_infos)/sizeof(vram_infos[0]))
		return(NULL);
	return(mtext[mode]);
}

#define NOMODE 0

static ivmode = NOMODE;

static Ytable vram_ytable[600+1];

Ytable *get_ytable(long bpr, Coor height)
{
int y;
long apt;
long napt;

apt = 0;	
for (y=0; y<=height; y++)
	{
	napt = apt + bpr - 1;
	vram_ytable[y].address = (apt&0xffff)+0xa0000;
	vram_ytable[y].bank = apt>>16;
	if ((napt^apt)&0xf0000)	/* if bank not same */
		{
		vram_ytable[y].split_at = bpr-1-(napt&0xffff);
		}
	else
		vram_ytable[y].split_at = 0;
	apt += bpr;
	}
return(vram_ytable);
}


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
		err = Success;
	enable_ext();
	r->lib = vram_get_rlib();
	r->type = mode + dev->first_rtype;
	r->width = width;
	r->height = height;
	r->pdepth = vram_infos[mode].bits;
	r->aspect_dx = xaspects[mode];
	r->aspect_dy = yaspects[mode];
	r->hw.vm.bpr = r->width;
	r->hw.vm.ytable = get_ytable(r->hw.vm.bpr, r->height);
	}
return(err);
}

static Errcode vram_close_graphics(Vdevice *driver)
/* Get out of graphics mode and back to text mode.  Note that this
 * function is called by the PJ screen size menu without a corresponding
 * open_graphics call, so we need to check that we've made it into
 * graphics mode before proceeding */
{
if (ivmode != NOMODE)
	{
	disable_ext();
	set_xmode(ivmode);
	}
return(Success);
}

struct vdevice_lib vram_device_library = {
	vram_detect, /* detect - Is our hardware attatched? */
	vram_get_modes,	/* get modes */
	vram_mode_text,	/* get extra info */
	NOFUNC,	/* set max width for height.  Since we're always 400... */
	vram_open_graphics,	/* open_graphics */
	vram_close_graphics,	/* close_graphics */
	NOFUNC,	/* open_cel */
	NOFUNC, /* show_rast */
};

Vdevice rexlib_header = {
	{ REX_VDRIVER, VDEV_VERSION, detect,NOFUNC,NULL},
	0, 									  /* first_rtype */
	sizeof(vram_infos)/sizeof(vram_infos[0]), /* num_rtypes */
	sizeof(vram_infos)/sizeof(vram_infos[0]), /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&vram_device_library, 			/* lib */
	NULL, 			/* grclib */
	NUM_LIB_CALLS, 	/* rast_lib_count */
};

