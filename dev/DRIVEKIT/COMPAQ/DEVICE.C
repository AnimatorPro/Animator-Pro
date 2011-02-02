/*  DEVICE.C
*/


/******************************************************************************
*									      *
*		   Copyright (C) 1991 by Autodesk, Inc. 		      *
*									      *
*	Permission to use, copy, modify, and distribute this software and     *
*	its documentation for the purpose of creating applications for	      *
*	AutoCAD, is hereby granted in accordance with the terms of the	      *
*	License Agreement accompanying this product.			      *
*									      *
*	Autodesk makes no warrantees, express or implied, as to the	      *
*	correctness of this code or any derivative works which incorporate    *
*	it.  Autodesk provides the code on an ''as-is'' basis and             *
*	explicitly disclaims any liability, express or implied, for	      *
*	errors, omissions, and other problems in the code, including	      *
*	consequential and incidental damages.				      *
*									      *
*	Use, duplication, or disclosure by the U.S.  Government is	      *
*	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
*	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
*	applicable.							      *
*									      *
******************************************************************************/

/*
    4/18/91 - jdb - put into ADI stream (created from \vesa driver)
*/


#include "stdio.h"
#include "syslib.h"
#include "errcodes.h"
#include "compaq.h"


/*  EXTERNS and FORWARD REFERENCES */

extern void set_xmode(int);
extern short get_xmode(void);
extern Errcode Init_VInfo();
extern Errcode null_detect(Vdevice *dev);
extern Errcode null_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm);
extern char *null_mode_text(Vdevice *driver, USHORT mode);
extern Errcode null_open_graphics(Vdevice *dev, VRaster *r,
	LONG width, LONG height, USHORT mode);
extern Errcode null_close_graphics(Vdevice *driver);
extern struct rastlib *get_rlib(Vdevice *dev, int mode, Raster *r);
extern void get_mode_info(int mode);
extern int is_it_compaq(void);


/*  GLOBALS */

struct vdevice_lib null_device_library = {
	null_detect, /* detect - Is our hardware attached? */
	null_get_modes, /* get modes */
	null_mode_text, /* get extra info */
	NOFUNC, /* set max width for height.  Since we're always 400... */
	null_open_graphics,	/* open_graphics */
	null_close_graphics,	/* close_graphics */
	NOFUNC, /* open_cel */
	NOFUNC, /* show_rast */
};

Hostlib _a_a_stdiolib = { NULL,AA_STDIOLIB,(AA_STDIOLIB_VERSION|HLIB_OPTIONAL)};
Hostlib _a_a_syslib = { &_a_a_stdiolib, AA_SYSLIB, AA_SYSLIB_VERSION };

Vdevice rexlib_header = {

	{ REX_VDRIVER, VDEV_VERSION, Init_VInfo, NOFUNC, &_a_a_syslib},
	0,									  /* first_rtype */
	1, /* num_rtypes */
	1, /* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()), /* dev_lib_count */
	&null_device_library,			/* lib */
	NULL,			/* grclib */
	NUM_LIB_CALLS,	/* rast_lib_count */
};


/*  locals */


void get_Vi(int mode);
void build_VInfo(Vdevice *dev);

static int  is_VInfo_built = FALSE;
static int numVESAModes = 0;
static short packed_mode_count = 0;
static short packed_modes[MAXVESAMODES];
static char *null_msg = "";

#define NOMODE 0
static int ivmode = NOMODE;

static struct vmode_info Vi = {
	sizeof(Vmode_info),
	0,			/* mode index */
	"Compaq not detected",
	8,			/* bits */
	1,			/* planes */
	{640,640,640,1},	/* fixed width */
	{400,400,400,1},	/* fixed height */
	TRUE, TRUE,		/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	0,			/* inner_type */
	1,			/* display pages */
	1,			/* store pages */
	640*400,		/* display bytes */
	640*400,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,			/* Swap screen during vsync. (We can't swap screens even)*/
	1000000U/70,		/* field rate */
	0			/* vertical blanking period */
};


Errcode null_detect(Vdevice *dev)
{
    int     retval;


    retval = is_it_compaq();

    if (retval == Success) {
	if (!is_VInfo_built) {
	    build_VInfo(dev);
	}
	return Success;
    } else {
	return Err_no_display;
    }
}


Errcode Init_VInfo()
{
    return null_detect(&rexlib_header);
}



void build_VInfo(Vdevice *dev)
{
    int i;
    int mode;


    packed_mode_count = 0;

    /*	extract the PACKED_PIXEL modes */
    for (i = 0; i < MAXVESAMODES; i++) {

	/* if end of modes table, break */
	if ((mode = VESAModes[i]) == -1) {
	    break;
	}

	get_mode_info(mode);

	packed_modes[packed_mode_count] = mode;
	packed_mode_count++;
    }

    rexlib_header.num_rtypes = packed_mode_count;
    rexlib_header.mode_count = packed_mode_count;
    is_VInfo_built = TRUE;
}


void get_Vi(int mode)
{
    int xsize, ysize, numplanes, bitsperpixel;


    get_mode_info(packed_modes[mode]);

    if (VESAModeBlock.modeattr & EXTENDED_INFO) {
	xsize = VESAModeBlock.xsize;
	ysize = VESAModeBlock.ysize;
	bitsperpixel = VESAModeBlock.bitsperpixel;
	numplanes = VESAModeBlock.numplanes;
    }

    Vi.struct_size = sizeof(Vmode_info);
    Vi.mode_ix = mode;

    sprintf(Vi.mode_name, "Compaq Advanced VGA ",
	xsize, ysize, packed_modes[mode]
    );

    Vi.bits = bitsperpixel;
    Vi.planes = numplanes;
    Vi.width.min = xsize;
    Vi.width.max = xsize;
    Vi.width.actual = xsize;
    Vi.width.grain = 1;
    Vi.height.min = ysize;
    Vi.height.max = ysize;
    Vi.height.actual = ysize;
    Vi.height.grain = 1;
    Vi.readable = TRUE;
    Vi.writeable = TRUE;
    Vi.displayable = TRUE;
    Vi.display_pages = 1;
    Vi.store_pages = 0;
    Vi.display_bytes = xsize * ysize;
    Vi.store_bytes = 0;
    Vi.palette_vblank_only = TRUE;
    Vi.screen_swap_vblank_only = FALSE;
    Vi.field_rate = 1000000U/70U;
}


static Errcode null_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
    if (!is_VInfo_built) {
	null_detect(driver);
    }

    if (mode >= packed_mode_count)
	return(Err_no_such_mode);

    get_Vi(mode);

    memcpy(pvm, &Vi, sizeof(Vmode_info));
    return(Success);
}


static char *null_mode_text(Vdevice *dev, USHORT mode)
{
	if (!is_VInfo_built) {
	    null_detect(dev);
	}
	if (mode >= packed_mode_count)
		return(NULL);

	return null_msg;
}


int null_set_max_height(Vdevice *dev, Vmode_info *vm)
{
    int height;

    if ((height = vm->display_bytes / vm->width.actual) <= 0)
	return(Err_no_vram);
    if (height > vm->height.max)
	height = vm->height.max;
    height -= height % vm->height.grain;
    if (height < vm->height.min)
	return(Err_too_big);
    return(height);
}


Errcode null_open_graphics(Vdevice *dev, VRaster *r,
	LONG width, LONG height, USHORT mode)
{
    Errcode err;
    int vmode;
    int x_aspect;


    if (!is_VInfo_built) {
	null_detect(dev);
    }

    if (mode >= packed_mode_count)
	return(Err_no_such_mode);

    get_Vi(mode);

    if (width != Vi.width.actual ||
	height != Vi.height.actual )
	return(Err_wrong_res);

    if ((err = null_detect(dev)) == Success) {

	vmode = packed_modes[mode];

	ivmode = 1;

	set_xmode(vmode);

	if (get_xmode() != vmode) {
		err = Err_no_vram;
	} else {
		err = Success;
	}
	err = Success;

	r->lib = get_rlib(dev, mode, (Raster *) r);
	r->type = mode + dev->first_rtype;
	r->width = width;
	r->height = height;
	r->pdepth = Vi.bits;
	x_aspect = (3 * width * 1000)/(4 * height);
	if (x_aspect == 1000) {
	    r->aspect_dx = 1;
	    r->aspect_dy = 1;
	} else {
	    r->aspect_dx = x_aspect;
	    r->aspect_dy = 1000;
	}
    }
    return(err);
}


static Errcode null_close_graphics(Vdevice *dev)
{
    if (ivmode > 0) {
	set_xmode(ivmode);
    }
    return(Success);
}

