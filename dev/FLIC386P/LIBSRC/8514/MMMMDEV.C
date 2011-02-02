/*****************************************************************************
 * MMMMDEV.C - The device layer interface for the 8514 driver.
 *
 *
 *	 NOTE:	The following symbols in the driver are unreferenced.  These are
 *			functions that have been coded but never tested.  Don't add any
 *			of these functions to the raster library unless you're prepared
 *			to do extensive testing of the functions!
 *
 *			   pj_8514_blitrect 			 (mmmmblin.asm)
 *			   pj_8514_d_hline				 (mmmmdhli.asm)
 *			   pj_8514_get_dot				 (mmmmdot.asm)
 *			   pj_8514_put_dot				 (mmmmdot.asm)
 *			   _pj_8514_mask2blit			 (mmmmmsk2.asm)
 *			   pj_8514_set_rect 			 (mmmmrect.asm)
 *
 * MAINTENANCE:
 *	10/02/91	Ian
 *				Ported into fliclib.  This involved mainly running SUBS to
 *				changed all the old mmmm_whatever globals to pj_8514_whatever.
 *				Also, a couple of conditional sections were added to the code
 *				in this module to cope with the slight difference between the
 *				the rex-loadable evironment and the fliclib environment.
 ****************************************************************************/

#include "a8514.h"

/*----------------------------------------------------------------------------
 * the following block does some code tweaks depending on whether we're
 * building the REX-loadable driver or the FLILIB version of the driver.
 *
 * When building the FLILIB version, the FLILIB_CODE symbol is defined via
 * a -D compiler switch in the makefile.  (The switch is actually defined
 * in the MAKE.INC file, so that it's automatically used when the compile
 * is being done in the FLILIB subdirs.
 *--------------------------------------------------------------------------*/

#ifndef FLILIB_CODE
	#define pj_malloc	malloc				// rex code uses malloc() and
	#define pj_free 	free				// free() from hostlib interface.
	#define a8514dev	rexlib_header		// rex code needs Vdevice named
	extern Vdevice		rexlib_header;		// rexlib_header for loader.
#else
	extern void 		*pj_malloc(size_t); // flilib code must go through
	extern void 		pj_free(void *);	// library memory management.
	#define a8514dev	a8514_vdevice		// flilib code Vdevice name must
	extern Vdevice		a8514_vdevice;		// be a static name, not global.
#endif

#define XPIX 640
#define YPIX 480
#define HIGHX 1024
#define HIGHY 768
#define CEL_COUNT 2

void *pj_8514_get_rlib(void);

/* ----------------- device level functions --------------------------- */
static Boolean crt8514, color_mon;
static SHORT graphics_mode = -1;
static SHORT ivmode = 0;
static mwidth, mheight;

static Errcode detect()
{
int i,crt;

/* Make sure error port is there... */
pj_8514_i86_outw(ERR_TERM, 0x5555);
if (pj_8514_i86_inw(ERR_TERM) != 0x5555)
	{
	return(Err_no_8514);
	}
if (!(pj_8514_i86_inw(SUBSYS_STAT) & 0x80))
	return(Err_no_vram);
for (i=0; i<20; i++)	/* poll subsystem status register until stable */
	{
	crt = pj_8514_i86_inw(SUBSYS_STAT);
	if (crt & NO8514)
		{
		crt8514 = FALSE;
		}
	else
		{
		crt8514 = TRUE;
		}
	}
if (crt&MONOCHROME)
	{
	color_mon = FALSE;
	}
else
	{
	color_mon = TRUE;
	}
return(Success);
}

static Errcode detect_8514(Vdevice *vd)
{
return detect();
}

static Errcode detect_display()
/*
 * Check for adapter.  Return error code that'll keep us off the driver list,
 * but otherwise not squawk inside Animator Pro.
 */
{
if (detect() < Success)
	return Err_no_display;
else
	return Success;
}


struct VPARMS {
	WORD crtc_off;
	WORD advfunc;
	WORD v_total;
	WORD v_sync_strt;
	WORD v_displayed;
	WORD h_total;
	WORD h_sync_strt;
	WORD h_displayed;
	WORD h_sync_wid;
	WORD v_sync_wid;
	WORD crtc_on;
};
static struct VPARMS stuff[] = {
{ 0x43,0x03,0x418,0x3d2,0x3bb,0x63,0x52,0x4f,0x2c,0x22,0x023},
{ 0x53,0x07,0x660,0x600,0x5fb,0x9d,0x81,0x7f,0x16,0x08,0x33},
};
#define DISP_CNTL		0x22E8	/* Display Control Register 	*/

/* Wait out any pending commands */
static wait_out()
{
while (pj_8514_i86_inb(CMD_STATUS)&CMD_ACTIVE)
	;
}


#define WAIT_A14 0x4000

/* If possible make 8514 go to 256 color mode */
static void go_graphics(SHORT mode)
{

graphics_mode = mode;
wait_out();
/* Async Reset */
pj_8514_i86_outw(SUBSYS_CNTL, STOP_SEQ);
/* Enable 8514A */
pj_8514_i86_outw(SUBSYS_CNTL, START_SEQ);
/* Configure memory for 8 bits/pixel (I think?) */
pj_8514_i86_outw(MLTFUNC_CNTL, MEM_CNTL | 0x06);
/* Max pixels/rows - 1 */
pj_8514_i86_outw(MAJ_AXIS_PCNT,0);
/* Max pixel rows - 1 */
pj_8514_i86_outw(MLTFUNC_CNTL,MINOR_AXIS_PCNT);
pj_8514_i86_outw(WRT_MASK,0xFFFF);
/* Init clr compare funct. */
pj_8514_i86_outw(MLTFUNC_CNTL, PIX_CNTL | 0x000);
/* Background mix - dest. */
pj_8514_i86_outw(BKGD_MIX,MIX_DEST);

/* Set up clipping most likely ??? */
pj_8514_i86_outw(MLTFUNC_CNTL, SCISSOR_T | 0x000);
pj_8514_i86_outw(MLTFUNC_CNTL,SCISSOR_L | 0x000);
pj_8514_i86_outw(MLTFUNC_CNTL,SCISSOR_B | (0x3ff));
pj_8514_i86_outw(MLTFUNC_CNTL,SCISSOR_R | (0x3ff));

pj_8514_i86_outw(DISP_CNTL,stuff[mode].crtc_off);
pj_8514_i86_outw(ADVFUNC_CNTL,stuff[mode].advfunc);
pj_8514_i86_outw(V_TOTAL,stuff[mode].v_total);
pj_8514_i86_outw(V_SYNC_STRT,stuff[mode].v_sync_strt);
pj_8514_i86_outw(V_DISPLAYED,stuff[mode].v_displayed);
pj_8514_i86_outw(H_TOTAL,stuff[mode].h_total);
pj_8514_i86_outw(H_SYNC_STRT,stuff[mode].h_sync_strt);
pj_8514_i86_outw(H_DISPLAYED,stuff[mode].h_displayed);
pj_8514_i86_outw(H_SYNC_WID,stuff[mode].h_sync_wid);
pj_8514_i86_outw(V_SYNC_WID,stuff[mode].v_sync_wid);
pj_8514_i86_outw(DISP_CNTL,stuff[mode].crtc_on);
/* Set data transfer active bit in mix register */
pj_8514_i86_outw(FGRD_MIX,F_CLR_ACTIVE|MIX_SRC);
/* Clear screen */
pj_8514_i86_outw(FRGD_COLOR, 0);
pj_8514_i86_outw(CUR_X_POS, 0);
pj_8514_i86_outw(CUR_Y_POS, 0);
pj_8514_i86_outw(MAJ_AXIS_PCNT, 0x3FF);
pj_8514_i86_outw(MLTFUNC_CNTL, 0x3FF);
pj_8514_i86_outw(COMMAND, 0x40F3);
pj_8514_i86_outw(SUBSYS_CNTL, 4);
}


static void close_8514()
{
	if (graphics_mode >= 0)
	{
		pj_8514_i86_outb(DAC_MASK,0xff);	/* make all planes visible */
		pj_8514_i86_outw(ADVFUNC_CNTL,0x06);	/* enable VGA display */
	}
	graphics_mode = -1;
}

static Errcode mmmm_close_graphics(Vdevice *driver)
/* Get out of graphics mode and back to text mode.	Note that this
 * function is called by the PJ screen size menu without a corresponding
 * open_graphics call, so we need to check that we've made it into
 * graphics mode before proceeding */
{
	if (ivmode != 0)
		{
		close_8514();
		pj_8514_set_vmode(ivmode);
		}
	return(Success);
}


static Vmode_info mmmm_infos[] =
{
	{
	sizeof(Vmode_info),
	0,	/* mode ix */
	"Panacea 8514/A Drvr",
	8,	/* bits */
	1,	/* planes */
	{XPIX,XPIX,XPIX,1}, /* width */
	{YPIX,YPIX,YPIX,1}, /* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	1,				/* fields_per_frame */
	1,				/* display pages */
	CEL_COUNT,				/* store pages */
	XPIX*YPIX,		/* display bytes */
	2*XPIX*YPIX,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate 1/70th sec */
	0,				/* vblank period */
	},
	{
	sizeof(Vmode_info),
	1,	/* mode ix */
	"Panacea 8514/A Drvr",
	8,	/* bits */
	1,	/* planes */
	{HIGHX,HIGHX,HIGHX,1},	/* width */
	{HIGHY,HIGHY,HIGHY,1},	/* height */
	TRUE, TRUE, 	/* read and write */
	TRUE,			/* Has viewable screen suitable for menus etc */
	2,				/* 2 fields_per_frame, interlaced */
	1,				/* display pages */
	CEL_COUNT,				/* store pages */
	HIGHX*HIGHY,		/* display bytes */
	1*HIGHX*HIGHY,		/* storage bytes */
	TRUE,			/* Palette set's only allowed during vblank */
	0,				/* Swap screen during vsync. (We can't swap screens even)*/
	70U/1000000U,	/* field rate 1/70th sec */
	0,				/* vblank period */
	},
};

static Errcode mmmm_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
{
	if (mode >= sizeof(mmmm_infos)/sizeof(mmmm_infos[0]))
		return(Err_no_such_mode);
	*pvm = mmmm_infos[mode];
	return(Success);
}

static char *mmmm_mode_text(Vdevice *driver, USHORT mode)
{
static char *mtext[] = {
 "Panacea's PanaMate-8514/A v1.0 - 640x480 Mode\n"
 "Copyright (c) 1989-1991 Panacea Inc.\n"
 "All Rights Reserved\nLicensed to Autodesk, Inc.",
 "Panacea's PanaMate-8514/A v1.0 - 1024x768 Mode\n"
 "Copyright (c) 1989-1991 Panacea Inc.\n"
 "All Rights Reserved\nLicensed to Autodesk, Inc.",
	};

if (mode >= sizeof(mmmm_infos)/sizeof(mmmm_infos[0]))
	return(NULL);
return(mtext[mode]);
}



static SHORT saymin[CEL_COUNT] = {0, 1*YPIX, };
static SHORT saxmin[CEL_COUNT] = {0, 0,};

static UBYTE mmmm_salloc[CEL_COUNT];


static Errcode mmmm_open_cel_sub(Vdevice *driver, Rast8514 *r,
								 LONG width, LONG height,
								 USHORT pixel_depth, SHORT mode)
{
int i;
int celmax;

if(mode < 0)
	mode = 0;
if(pixel_depth > mmmm_infos[mode].bits)
	return(Err_pdepth_not_avail);
if (mode > 0)
	celmax = 1;
else
	celmax = CEL_COUNT;
if (mmmm_infos[mode].width.max >= width &&
	mmmm_infos[mode].height.max >= height)
	{
	for (i=0; i<celmax; i++)
		{
		if (!mmmm_salloc[i])
			{
			mmmm_salloc[i] = TRUE;
			r->type = mode + driver->first_rtype;
			r->width = width;
			r->height = height;
			r->hw.am.screen_ix = i;
			r->hw.am.xcard = saxmin[i];
			r->hw.am.ycard = saymin[i];
			r->hw.am.dev = driver;
			r->hw.am.flags = 0;
			r->pdepth = pixel_depth;
			r->lib = pj_8514_get_rlib();
			r->aspect_dx = 1;
			r->aspect_dy = 1;
			return(Success);
			}
		}
	return(Err_no_vram);
	}
else
	{
	return(Err_overflow);
	}
}
static Errcode mmmm_open_cel(Vdevice *driver, Rast8514 *r,
							 LONG width, LONG height,
							 USHORT pixel_depth, UBYTE displayable)
{
	if (displayable)
		return(Err_unimpl);

	return(mmmm_open_cel_sub(driver,r,width,height,
							 pixel_depth,graphics_mode));
}
static Errcode mmmm_open_graphics(Vdevice *driver, Rast8514 *r,
								  LONG width, LONG height, USHORT mode)
{
Errcode err;

	if (mode >= sizeof(mmmm_infos)/sizeof(mmmm_infos[0]))
		return(Err_no_such_mode);

	if ((err = detect_8514(driver)) < Success)
		return(err);

	ivmode = pj_8514_get_vmode();
	if((err = mmmm_open_cel_sub(driver, r, width, height,
								 mmmm_infos[mode].bits, mode)) < Success)
	{
		return(err);
	}
	r->hw.am.flags |= IS_DISPLAYED;
	go_graphics(mode);
	return(Success);

}
Errcode pj_8514_close_rast(Rast8514 *rast)
{
	mmmm_salloc[rast->hw.am.screen_ix] = FALSE;
	return(Success);
}

/*****************************************************************************
 * Rex and Vdriver interface stuff...
 ****************************************************************************/

static struct vdevice_lib device_library = {
	detect_8514,			/* detect - Is our hardware attatched? */
	mmmm_get_modes,
	mmmm_mode_text, 		/* get extra info */
	NOFUNC, 				/* set max width for height.  Stays fixed for us. */
	mmmm_open_graphics,
	mmmm_close_graphics,
	mmmm_open_cel,			/* open_cel */
	NOFUNC, 				/* show_rast */
	};

/*----------------------------------------------------------------------------
 * if we're compiling the REX-loadable driver, we need to request the host
 * library support we need, and fill in a rexlib_header structure.
 *--------------------------------------------------------------------------*/

#ifndef FLILIB_CODE

	#define HLIB_TYPE_1 AA_SYSLIB		// we use malloc/free from this lib
	#define HLIB_TYPE_2 AA_STDIOLIB
	#include "hliblist.h"               // auto-build hostlib linked list

	Vdevice rexlib_header = {
		{ REX_VDRIVER, VDEV_VERSION, detect_display, NOFUNC, HLIB_LIST},
		0,											/* first_rtype */
		sizeof(mmmm_infos)/sizeof(mmmm_infos[0]),	/* num_rtypes */
		sizeof(mmmm_infos)/sizeof(mmmm_infos[0]),	/* mode_count */
		sizeof(Vdevice_lib)/sizeof(int(*)()),		/* dev_lib_count */
		&device_library,							/* lib */
		NULL,										/* grclib filled in by host */
		NUM_LIB_CALLS,								/* rast_lib_count */
		};

/*----------------------------------------------------------------------------
 * if we're building the FLILIB driver, we don't need host libraries, and
 * our Vdevice structure can't be named rexlib_header, it has to have its
 * own name.  we also need a local_vdevice structure that points to the
 * Vdevice structure.
 *--------------------------------------------------------------------------*/

#else

	static Vdevice a8514_vdevice = {
		{ 0, 0, detect_display, NOFUNC, NULL},
		0,											/* first_rtype */
		sizeof(mmmm_infos)/sizeof(mmmm_infos[0]),	/* num_rtypes */
		sizeof(mmmm_infos)/sizeof(mmmm_infos[0]),	/* mode_count */
		sizeof(Vdevice_lib)/sizeof(int(*)()),		/* dev_lib_count */
		&device_library,							/* lib */
		NULL,										/* grclib filled in by host */
		NUM_LIB_CALLS,								/* rast_lib_count */
		};

	static struct local_vdevice localdev = {NULL, &a8514_vdevice};

	struct local_vdevice *pj_vdev_8514 = &localdev;

#endif	/* FLILIB_CODE */
