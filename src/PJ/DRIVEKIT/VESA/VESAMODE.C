/*****************************************************************************
 * VESAMODE.C - Translate VESA's modeinfo data into our SMInfo format.
 *				(Cloned from what used to be the DEVICE.C module for the
 *				standalone VESA driver.)
 *
 *	01/7/91  -	jdb
 *				put into ADI stream.
 *	01/16/91 -	jdb
 *				corrected getting the table of VESA modes; now limited to just
 *				pixel-packed modes.
 *	03/02/91 -	Ian
 *				Major rewrite of the whole driver.	In this module, mostly
 *				did minor performance tweaking and fixed a few buglets (such
 *				as not freeing the DOS memory buffer upon driver unload.)
 *	05/28/91 -	Ian
 *				Major tweak, to make it easier to clone new drivers off of
 *				this source code.  Eliminated RASTER.C module, moving the
 *				get_rlib function into this module, and putting the maskXblit
 *				routines into the BLTC module.	Also, at this time, the 'vesa'
 *				name prefix was changed to 'drv' throughout.  (EG, a routine
 *				named vesa_get_hseg() would now be named pj_vdrv_get_hseg().  A
 *				file named VESADOTS.ASM would now be named DRVDOTS.ASM).
 *	07/29/91 -	Ian
 *				Cloned from the original, tweaked to eliminate external names
 *				to whatever degree possible, and to make remaining externals
 *				start with pj_vesa_.  This is for use in the FLILIB project.
 *				Also, moved into the new drvcomn\ dir, since this module is
 *				now shared by the vesa and svga autodetect drivers.
 ****************************************************************************/

/******************************************************************************
*																			  *
*		   Copyright (C) 1991 by Autodesk, Inc. 							  *
*																			  *
*	Permission to use, copy, modify, and distribute this software and		  *
*	its documentation for the purpose of creating applications for			  *
*	AutoCAD, is hereby granted in accordance with the terms of the			  *
*	License Agreement accompanying this product.							  *
*																			  *
*	Autodesk makes no warrantees, express or implied, as to the 			  *
*	correctness of this code or any derivative works which incorporate		  *
*	it.  Autodesk provides the code on an ''as-is'' basis and                 *
*	explicitly disclaims any liability, express or implied, for 			  *
*	errors, omissions, and other problems in the code, including			  *
*	consequential and incidental damages.									  *
*																			  *
*	Use, duplication, or disclosure by the U.S.  Government is				  *
*	subject to restrictions set forth in FAR 52.227-19 (Commercial			  *
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)		  *
*	(1) (ii) (Rights in Technical Data and Computer Software, as			  *
*	applicable. 															  *
*																			  *
******************************************************************************/

#include "stdio.h"              // typical stuff...
#include "errcodes.h"
#include "syslib.h"

#include "vesamode.h"           // import VESA-defined thingies
#include "drvcomn.h"            // import definition of SMInfo structure

/*----------------------------------------------------------------------------
 * x/y dimensions table for standard 8-bit-per-pixel vesa modes...
 *--------------------------------------------------------------------------*/

static struct {
	USHORT xsize, ysize;
} stdmodes[] = {
	{640,	400  }, 	/* mode 0x0100 */
	{640,	480  }, 	/* mode 0x0101 */
	{0, 	0	 }, 	/* mode 0x0102 */
	{800,	600  }, 	/* mode 0x0103 */
	{0, 	0	 }, 	/* mode 0x0104 */
	{1024,	768  }, 	/* mode 0x0105 */
	{0, 	0	 }, 	/* mode 0x0106 */
	{1280,	1024 }, 	/* mode 0x0107 */
	};

int pj_vesa_build_sminfo(SMInfo *sminf, SHORT *vesamodes)
/*****************************************************************************
 * one-time init routine...build the array of SMInfo structures for our modes.
 * the array of all available modes has been built by the pj_vesa_is_it_vesa()
 * routine, prior to entry to this routine.
 ****************************************************************************/
{
	Errcode 	err;
	VBEInfo    *vib;
	VModeInfo	*vmb;
	SMInfo		tmp;
	int 		i, j;
	int 		mode;
	int 		memsize;
	USHORT		xsize;
	USHORT		ysize;
	int 		packed_mode_count;
	int 		vram_available;

#ifdef DEBUG_SHOW_MODEINFO
	FILE *dbgfile;
	dbgfile = fopen("vesamode.inf","w");
#endif

	err = Success;
	packed_mode_count = 0;

	vib = pj_vesa_get_bios_info();
	vram_available = /* if vesa version is 1.0, we have to assume tons of memory */
	  (vib->VESAVersion == VESA_1_0) ? 0x7FFFFFFF:vib->TotalMemory*BYTES_IN_64K;

	/* extract the PACKED_PIXEL modes */

	while (packed_mode_count < MAX_SMODES) {

		if (-1 == (mode = *vesamodes++))
			goto NO_MORE_MODES;

		if (NULL == (vmb = pj_vesa_get_mode_info(mode)))
			goto SKIP_THIS_MODE;

#ifdef DEBUG_SHOW_MODEINFO

		if (NULL != dbgfile) {
			fprintf(dbgfile,"\nModeInfo for VESA mode %x (all values hex):\n"
				   "modeattr       = %x \n"
				   "winAattr       = %x \n"
				   "winBattr       = %x \n"
				   "wingran        = %x \n"
				   "winsize        = %x \n"
				   "winAseg        = %x \n"
				   "winBseg        = %x \n"
				   "winfuncptr     = %x \n"
				   "pitch          = %x \n"
				   "xsize          = %x \n"
				   "ysize          = %x \n"
				   "xcharsize      = %x \n"
				   "ycharsize      = %x \n"
				   "numplanes      = %x \n"
				   "bitsperpixel   = %x \n"
				   "numbanks       = %x \n"
				   "memorymodel    = %x \n"
				   "banksize       = %x \n"
				   "numimagespages = %x \n",
				   (int)mode,
				   (int)(vmb->modeattr),
				   (int)(vmb->winAattr),
				   (int)(vmb->winBattr),
				   (int)(vmb->wingran),
				   (int)(vmb->winsize),
				   (int)(vmb->winAseg),
				   (int)(vmb->winBseg),
				   (int)(vmb->winfuncptr),
				   (int)(vmb->pitch),
				   (int)(vmb->xsize),
				   (int)(vmb->ysize),
				   (int)(vmb->xcharsize),
				   (int)(vmb->ycharsize),
				   (int)(vmb->numplanes),
				   (int)(vmb->bitsperpixel),
				   (int)(vmb->numbanks),
				   (int)(vmb->memorymodel),
				   (int)(vmb->banksize),
				   (int)(vmb->numimagespages)
				  );
		}

#endif
		if (!(vmb->modeattr & MODE_SUPPORTED)	/* if mode not supported in */
		 || !(vmb->modeattr & GRAPHICS_MODE)	/* hardware, or isn't 8 bit */
		 || (vmb->memorymodel != PACKED_PIXEL)	/* packed pixel graphics	*/
		 || (vmb->bitsperpixel != 8)) { 		/* mode, then skip it.		*/
			goto SKIP_THIS_MODE;
		}

		if (vmb->modeattr & EXTENDED_INFO) {
			xsize = vmb->xsize;
			ysize = vmb->ysize;
		} else if (mode >= 0x100 && mode <= 0x107) {
			xsize = stdmodes[mode-0x100].xsize;
			ysize = stdmodes[mode-0x100].ysize;
		} else {
			ysize = 0;
		}

		memsize = ysize * vmb->pitch;

		/* if the board has enough memory, allow the mode */

		if (memsize > 0 && memsize <= vram_available) {
			sminf[packed_mode_count].hdwmode = mode;
			sminf[packed_mode_count].width	 = xsize;
			sminf[packed_mode_count].height  = ysize;
			++packed_mode_count;
		}
SKIP_THIS_MODE:

		continue;

	}

NO_MORE_MODES:
	/* if no packed pixel modes supported, return error */

	if (packed_mode_count == 0) {
		goto OUT;
	}

	/* move the highest numbers to the front of the packed pixel table */

	for (i = 0; i < packed_mode_count - 1; i++) {
		for (j = packed_mode_count - 1; j > i; j--) {
			if (sminf[j].hdwmode < sminf[j-1].hdwmode) {
				tmp = sminf[j];
				sminf[j] = sminf[j-1];
				sminf[j-1] = tmp;
			}
		}
	}

	/* remove any repeated entries */

	for (i = 0; i < packed_mode_count - 1; i++) {
		if (sminf[i].hdwmode == sminf[i+1].hdwmode) {
			for (j = i; j < packed_mode_count - 1; j++) {
				sminf[j] = sminf[j+1];
			}
			--packed_mode_count;
		}
	}

	/* make sure modes 0x100 or 0x101 are at the head of the list
	   because an unconfigured PJ powers up in a new driver at mode_ix 0 */

	if (sminf[0].hdwmode != 0x100 && sminf[0].hdwmode != 0x101) {
		for (i = 1; i < packed_mode_count; i++) {
			if (sminf[i].hdwmode == 0x100 || sminf[i].hdwmode == 0x101) {
				tmp = sminf[0];
				sminf[0] = sminf[i];
				sminf[i] = tmp;
				break;
			}
		}
	}
OUT:

#ifdef DEBUG_SHOW_MODEINFO
	if (dbgfile != NULL)
		fclose(dbgfile);
#endif

	return packed_mode_count;
}

