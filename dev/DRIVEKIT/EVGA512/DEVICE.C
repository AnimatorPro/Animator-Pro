/**********************************************************************
;
; DEVICE.C -- Device operating information (resolutions, etc.)
;
;
; Modified by Panacea Inc.
;
; Panacea Inc.
; 50 Nashua Road, Suite 305
; Londonderry, New Hampshire, 03053-3444
; (603) 437-5022
;
;
;Revision History:
;
;When     Who   What
;======== ===   =======================================================
;09/13/90 JBR   Start of development.
;09/18/90 JK    Updated device level glue to new PJ specs.  Commented a bit.
;
;**********************************************************************/

#include <stdio.h>
#include "evga.h"
#include "errcodes.h"

void *evga_get_rlib(void);


Errcode evga_detect(Vdevice *dev)
{
if (isit_evga())
	return(Success);
else
	return(Err_no_display);
}

static Vmode_info evga_infos[] =
{
	sizeof(Vmode_info),
	0,				/* mode index */
	"Everex EVGA",	/* name */
	8,				/* bits */
	1,				/* planes */
	{512,512,512,1},/* Width */
	{480,480,480,1},/* Height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1, 				/* fields_per_frame */
	1,			  	/* Display pages */
	1,			  	/* Store pages */
	512*480,	  	/* Display bytes */
	512*480,	  	/* Storage bytes */
	TRUE,		  	/* Palette updates only during vertical blank */
	FALSE,			/* Swap screen during vsync. (We can't swap screens even)*/
	60,			  	/* Vsync rate */
};

static int mode_to_mode[] = { 0x15};

static Errcode evga_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
/* Return info about a specific mode in *pvm */
{
	if (mode >= sizeof(evga_infos)/sizeof(evga_infos[0]))
		return(Err_no_such_mode);
	*pvm = evga_infos[mode];
	return(Success);
}

static char *mtext[] = {
	"Everex 512x480.  Should eventually work with all Everex VGAs"
	};

static char *evga_mode_text(Vdevice *driver, USHORT mode)
/* Return (ascii) text description of a specific mode */
{
	if (mode >= sizeof(evga_infos)/sizeof(evga_infos[0]))
		return(NULL);
	return(mtext[mode]);
}

#define NOMODE 0

static ivmode = NOMODE;

static Ytable evga_ytable[600+1];

Ytable *get_ytable(long bpr, Coor height)
/* Build up a table with one entry for each line of the screen.  
 * This table includes the starting address for the line, which
 * of the super-vga banks it's in, and (in most modes at least)
 * a flag to say whether this line is split between two banks.
 * This table is used by the low level drawing functions except
 * for put_dot and get_dot to help speed the bank switching. */
{
int y;
long apt;
long napt;

apt = 0;	
for (y=0; y<=height; y++)
	{
	napt = apt + bpr - 1;
	evga_ytable[y].address = (apt&0xffff)+0xa0000;
	evga_ytable[y].bank = apt>>16;

#ifdef SPLITTING_ENABLED
	if ((napt^apt)&0xf0000)	
		{
		evga_ytable[y].split_at = bpr-1-(napt&0xffff);
		}
	else
#endif

		evga_ytable[y].split_at = 0;
	apt += bpr;
	}
return(evga_ytable);
}

Errcode evga_open_graphics(Vdevice *dev,  EvgaRast *r,
	LONG width, LONG height, USHORT mode)
/* Put display adapter into graphics mode.  Fill in raster structure
   pointed to by r with info host program needs to manipulate screen. */
{
Errcode err;
int vmode;

if (mode >= sizeof(evga_infos)/sizeof(evga_infos[0]))
	return(Err_no_such_mode);
if (width != evga_infos[mode].width.actual ||
	height != evga_infos[mode].height.actual )
	return(Err_wrong_res);
if ((err = evga_detect(dev)) == Success)
	{
	vmode = mode_to_mode[mode];
	ivmode = get_vmode();
	set_xmode(vmode);
	if (get_vmode() != 0x70)    
		err = Err_no_vram;
	else
		err = Success;
	enable_ext();
	r->lib = evga_get_rlib();
	r->type = mode + dev->first_rtype;
	r->width = width;
	r->height = height;
	r->pdepth = evga_infos[mode].bits;
	r->aspect_dx = 3;			/* is this the correct aspect ratio ?? */
	r->aspect_dy = 2;
	r->hw.em.bpr = r->width;
	r->hw.em.ytable = get_ytable(r->hw.em.bpr, r->height);
	}
return(err);
}

static Errcode evga_close_graphics(Vdevice *driver)
/* Get out of graphics mode and back to text mode.  Note that this
 * function is called by the PJ screen size menu without a corresponding
 * open_graphics call, so we need to check that we've made it into
 * graphics mode before proceeding */
{
if (ivmode != NOMODE)	/* Only do something if have done an open_graphics */
	{
	disable_ext();
	set_vmode(ivmode);
	}
return(Success);
}


struct vdevice_lib evga_device_library = {
	evga_detect, /* detect - Is our hardware attatched? */
	evga_get_modes,	/* get modes */
	evga_mode_text,	/* get extra info */
	NOFUNC,	/* set max width for height.  Since we're always 480... */
	evga_open_graphics,	/* open_graphics */
	evga_close_graphics,	/* close_graphics */
	NOFUNC,	/* open_cel */
	NOFUNC, /* show_rast */
};

#ifdef DEVELOPMENT
Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_stdiolib, AA_SYSLIB, AA_SYSLIB_VERSION };
#else
Hostlib _a_a_syslib = { NULL, AA_SYSLIB, AA_SYSLIB_VERSION };
#endif /* DEVELOPMENT */


Vdevice rexlib_header = {
	{ REX_VDRIVER, VDEV_VERSION, NOFUNC,NOFUNC,&_a_a_syslib},
	0, 									  /* first_rtype */
	sizeof(evga_infos)/sizeof(evga_infos[0]), /* num_rtypes */
	sizeof(evga_infos)/sizeof(evga_infos[0]), /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&evga_device_library, 			/* lib */
	NULL, 			/* grclib */
	NUM_LIB_CALLS, 	/* rast_lib_count */
};

