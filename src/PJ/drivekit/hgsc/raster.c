/*****************************************************************************
 * RASTER.C - Raster-layer function for the PJ HGSC SVGA driver.
 ****************************************************************************/

#include "hgsc.h"

typedef struct {		/* parms passed to mask1line and mask2line calls */
	UBYTE	*mbytes;
	Pixel	*lbuf;
	int 	count;
	UBYTE	bit1;
	UBYTE	oncolor;
	UBYTE	offcolor;
	} Mlparms;

extern void mask1line(Mlparms *pparms); /* these routines live in the */
extern void mask2line(Mlparms *pparms); /* maskblit.asm file */

static void hgs_mask1blit(UBYTE *mbytes, int mbpr, int mx, int my,
						  Hrast *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
						  Pixel oncolor)
/*****************************************************************************
 * drive the mask1blit process...get_hseg, process the line, put_hseg.
 ****************************************************************************/
{
	UBYTE	linebuf[1024];
	Mlparms parms;

	parms.mbytes  = mbytes + (mx >> 3) + (my * mbpr);
	parms.bit1	  = 0x0080 >> (mx & 7);
	parms.lbuf	  = linebuf;
	parms.count   = rw;
	parms.oncolor = oncolor;

	while (rh--)
		{
		hgs_get_hseg(r, &linebuf, rx, ry, rw);
		mask1line(&parms);
		hgs_put_hseg(r, &linebuf, rx, ry, rw);
		++ry;
		parms.mbytes += mbpr;
		}
}

static void hgs_mask2blit(UBYTE *mbytes, int mbpr, int mx, int my,
						  Hrast *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
						  Pixel oncolor, Pixel offcolor)
/*****************************************************************************
 * drive the mask2blit process...get_hseg, process the line, put_hseg.
 ****************************************************************************/
{
	UBYTE	linebuf[1024];
	Mlparms parms;

	parms.mbytes   = mbytes + (mx >> 3) + (my * mbpr);
	parms.bit1	   = 0x0080 >> (mx & 7);
	parms.lbuf	   = linebuf;
	parms.count    = rw;
	parms.oncolor  = oncolor;
	parms.offcolor = offcolor;

	while (rh--)
		{
		hgs_get_hseg(r, &linebuf, rx, ry, rw);
		mask2line(&parms);
		hgs_put_hseg(r, &linebuf, rx, ry, rw);
		++ry;
		parms.mbytes += mbpr;
		}
}


Rastlib *hgsc_get_rlib(Vdevice *dev, int mode, Hrast *r)
/*****************************************************************************
 * fill in the static raster library structure, return a pointer to it.
 ****************************************************************************/
{
	static UBYTE   got_lib = FALSE;
	static Rastlib hgsc_raster_library;

	if (!got_lib)
		{
		hgsc_raster_library.set_colors	 = (rl_type_set_colors)hgs_setpalette;
		hgsc_raster_library.wait_vsync	 = (rl_type_wait_vsync)hgs_wait_vsync;
		hgsc_raster_library.put_dot 	 = (rl_type_put_dot)hgs_putdot;
		hgsc_raster_library.cput_dot	 = (rl_type_cput_dot)hgs_cputdot;
		hgsc_raster_library.get_dot 	 = (rl_type_get_dot)hgs_getdot;
		hgsc_raster_library.cget_dot	 = (rl_type_cget_dot)hgs_cgetdot;
		hgsc_raster_library.get_hseg	 = (rl_type_get_hseg)hgs_get_hseg;
		hgsc_raster_library.put_hseg	 = (rl_type_put_hseg)hgs_put_hseg;
		hgsc_raster_library.put_vseg	 = (rl_type_put_vseg)hgs_put_vseg;
		hgsc_raster_library.get_vseg	 = (rl_type_get_vseg)hgs_get_vseg;
		hgsc_raster_library.set_hline	 = (rl_type_set_hline)hgs_set_hline;
		hgsc_raster_library.set_vline	 = (rl_type_set_vline)hgs_set_vline;
		hgsc_raster_library.get_rectpix  = (rl_type_get_rectpix)hgs_get_rectpix;
		hgsc_raster_library.put_rectpix  = (rl_type_put_rectpix)hgs_put_rectpix;
		hgsc_raster_library.set_rect	 = (rl_type_set_rect)hgs_setrect;
//		hgsc_raster_library.xor_rect	 = (rl_type_xor_rect)hgs_xorrect;
		hgsc_raster_library.set_rast	 = (rl_type_set_rast)hgs_setrast;
		hgsc_raster_library.mask1blit	 = (rl_type_mask1blit)hgs_mask1blit;
		hgsc_raster_library.mask2blit	 = (rl_type_mask2blit)hgs_mask2blit;
		hgsc_raster_library.unss2_rect	 = (rl_type_unss2_rect)hgs_unss2;

		got_lib = TRUE;
		}
	return(&hgsc_raster_library);
}

