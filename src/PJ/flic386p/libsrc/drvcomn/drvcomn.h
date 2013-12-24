/*****************************************************************************
 * DRVCOMN.H - Header file for common driver routines.
 ****************************************************************************/

#ifndef DRVCOMN_H
#define DRVCOMN_H

/******************************************************************************
*
*		   Copyright (C) 1991 by Autodesk, Inc.
*
*	Permission to use, copy, modify, and distribute this software and
*	its documentation for the purpose of creating applications for
*	AutoCAD, is hereby granted in accordance with the terms of the
*	License Agreement accompanying this product.
*
*	Autodesk makes no warrantees, express or implied, as to the
*	correctness of this code or any derivative works which incorporate
*	it.  Autodesk provides the code on an ''as-is'' basis and
*	explicitly disclaims any liability, express or implied, for
*	errors, omissions, and other problems in the code, including
*	consequential and incidental damages.
*
*	Use, duplication, or disclosure by the U.S.  Government is
*	subject to restrictions set forth in FAR 52.227-19 (Commercial
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)
*	(1) (ii) (Rights in Technical Data and Computer Software, as
*	applicable.
*
******************************************************************************/

/*
	01/07/91  - jdb - put into ADI stream.
	03/27/91  - ian - major rewrite of entire driver.
	04/17/91  - jdb - def'd Raster to VesaRast before rastlib.h inclusion
				made necessary protos below match rastlib.h
				cast all 'Bytemap *' to 'Raster *' to match rastlib.h
				(above was necessary for High C strict type-casting)
	05/28/91  - Split VESA.H file into DRVCOMN.H and VESA.H, makes it easier
				to clone a new driver off of this code.
	07/30/91  - Added SMInfo structure.
*/

#include "stdtypes.h"
#include "raster.h"
#include "rastlib.h"

/*----------------------------------------------------------------------------
 * our internal 'window control' structure...
 *	 see DRVCOMN.I for a detailed description.
 *
 * NOTE:  If this is changed, corresponding changes must be made in DRVCOMN.I!
 *--------------------------------------------------------------------------*/

typedef struct {
	int 	offsmask;
	int 	rdcurbank;
	int 	wrcurbank;
	void	*rdaddr;
	void	*wraddr;
	int 	pitch;
	int 	rdwhich;
	int 	wrwhich;
	int 	width;
	int 	height;
	int 	wincount;
	int 	windwords;
	int 	winlbytes;
	void	*srbvector; 	// pointer to set_read_bank routine
	void	*swbvector; 	// pointer to set_write_bank routine
	long	drvdat1;		// driver-specific data field for svga driver
	UBYTE	bankshift;
	UBYTE	granshift;
	UBYTE	samerw;
	UBYTE	align1; 	/* realign struct to dword boundry */
	UBYTE	*localbuf;	/* our local buffer, alloc'd at device-open time */
	UBYTE	*mbytes;	/* these items used by maskXblit... */
	int 	rx;
	int 	ry;
	int 	count;
	UBYTE	bit1;
	UBYTE	oncolor;
	UBYTE	offcolor;
	} Wcontrol;

/*----------------------------------------------------------------------------
 * SMInfo struct...
 *	 herein we keep the hardware mode number and x/y sizes for each mode the
 *	 detected graphics card supports.  A static array of these exists
 *	 in drvcomn.asm; the values are filled in by the detect routines.
 *
 *	 if this struct or the MAX constant change, drvcomn.i must change too!
 *--------------------------------------------------------------------------*/

typedef struct svga_modeinfo {
	USHORT	hdwmode;
	USHORT	width;
	USHORT	height;
	} SMInfo;

#define MAX_SMODES 16

/*----------------------------------------------------------------------------
 * NOTE!  if the LCLBUF_SIZE value is changed, the corresponding constant in
 *		  the DRVCOMN.I assembler header MUST be changed too!!!
 *
 *		  The buffer size MUST be a multiple of 4 (some asm code assumes the
 *		  buffer size is a dword multiple).
 *--------------------------------------------------------------------------*/

#define LCLBUF_SIZE 16384	/* allocate 16k buffer for swap_in_card, et. al. */

/*----------------------------------------------------------------------------
 * prototypes for the functions written in assembler...
 *--------------------------------------------------------------------------*/

/* in drvcomn.asm */

extern Rastlib	 pj_vdrv_raster_library;
extern Boolean	 pj_vdrv_has_8bitdac;
extern Wcontrol  pj_vdrv_wcontrol;
extern SMInfo	 pj_vdrv_modeinfo[];

extern void  pj_vdrv_wait_vblank(Raster *r);
extern void  pj_vdrv_set_colors(Raster *r, LONG start, LONG count, void *color_table);
extern void  pj_vdrv_uncc256(Raster *r, void *ucbuf);
extern void  pj_vdrv_uncc64(Raster *r, void *ucbuf);
extern void  pj_vdrv_8bit_set_colors(Raster *r, LONG start, LONG count, void *color_table);
extern void  pj_vdrv_8bit_uncc256(Raster *r, void *ucbuf);
extern void  pj_vdrv_8bit_uncc64(Raster *r, void *ucbuf);

/* in drvdots.asm */

extern void  pj_vdrv_put_dot(Raster *r, Pixel color, Coor x, Coor y);
extern void  pj_vdrv_cput_dot(Raster *r, Pixel color, Coor x, Coor y);
extern Pixel pj_vdrv_get_dot(Raster *r, Coor x, Coor y);
extern Pixel pj_vdrv_cget_dot(Raster *r, Coor x, Coor y);

/* in drvsegs.asm */

extern void  pj_vdrv_put_hseg(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
extern void  pj_vdrv_get_hseg(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
extern void  pj_vdrv_put_rectpix(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor w, Ucoor h);
extern void  pj_vdrv_get_rectpix(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor w, Ucoor h);

/* in drvlines.asm */

extern void  pj_vdrv_set_hline(Raster *r, Pixel color, Coor x, Coor y, Ucoor w);
extern void  pj_vdrv_set_rect(Raster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);

/* in drvvert.asm */

extern void  pj_vdrv_set_vline(Raster *r, Pixel color, Coor x, Coor y, Ucoor h);
extern void  pj_vdrv_put_vseg(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);
extern void  pj_vdrv_get_vseg(Raster *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);

/* in drvxorr.asm */

extern void  pj_vdrv_xor_rect(Raster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);

/* in drvunss2.asm */

extern void  pj_vdrv_unss2();

/* in drvunbrn.asm */

extern void pj_vdrv_unbrun_rect(Raster *r, void *ucbuf, LONG pixsize,
							 Coor x, Coor y, Ucoor width, Ucoor height);

/* in drvunlcc.asm */

extern void pj_vdrv_unlccomp_rect(Raster *r, void *ucbuf, LONG pixsize,
							   Coor x, Coor y, Ucoor width, Ucoor height);
/* in drvrast.asm */

extern void pj_vdrv_set_rast(Raster *r, Pixel color);
extern void pj_vdrv_xor_to_ram(Raster *sr, Raster *dr);
extern void pj_vdrv_xor_from_ram(Raster *sr, Raster *dr);

/* in drvblitc.c */

extern void pj_vdrv_blit_in_card(Raster *source, Coor src_x, Coor src_y,
					   Raster *dest, Coor dest_x, Coor dest_y,
					   Coor width, Coor height);

extern void pj_vdrv_swap_in_card(Raster *source, Coor src_x, Coor src_y,
					   Raster *dest, Coor dest_x, Coor dest_y,
					   Coor width, Coor height);

extern void pj_vdrv_swap_to_ram(Raster	*source, Coor src_x, Coor src_y,
					  Raster *dest, Coor dest_x, Coor dest_y,
					  Coor width, Coor height);

extern void pj_vdrv_swap_from_ram(Raster *source, Coor src_x, Coor src_y,
						Raster	*dest, Coor dest_x, Coor dest_y,
						Coor width, Coor height);


extern void pj_vdrv_tblit_in_card(Raster *source, Coor src_x, Coor src_y,
						Raster *dest, Coor dest_x, Coor dest_y,
						Coor width, Coor height, Pixel tcolor);

extern void pj_vdrv_tblit_to_ram(Raster  *source, Coor src_x, Coor src_y,
					   Raster *dest, Coor dest_x, Coor dest_y,
					   Coor width, Coor height, Pixel tcolor);

extern void pj_vdrv_tblit_from_ram(Raster *source, Coor src_x, Coor src_y,
						 Raster  *dest, Coor dest_x, Coor dest_y,
						 Coor width, Coor height, Pixel tcolor);

extern void pj_vdrv_zoom_in_card(Raster  *source, Coor src_x, Coor src_y,
					   Raster  *dest, Coor dest_x, Coor dest_y,
					   Ucoor width, Ucoor height,
					   LONG zoom_x, LONG zoom_y);

extern void pj_vdrv_zoom_to_ram(Raster	*source, Coor src_x, Coor src_y,
					  Raster *dest, Coor dest_x, Coor dest_y,
					  Ucoor width, Ucoor height,
					  LONG zoom_x, LONG zoom_y);

extern void pj_vdrv_zoom_from_ram(Raster *source, Coor src_x, Coor src_y,
						Raster	*dest, Coor dest_x, Coor dest_y,
						Ucoor width, Ucoor height,
						LONG zoom_x, LONG zoom_y);

extern void pj_vdrv_mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
						  Raster *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
						  Pixel oncolor);


extern void pj_vdrv_mask2blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
						  Raster *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
						  Pixel oncolor, Pixel offcolor);

/* in drvblita.asm */

extern void pj_vdrv_zoom_line(Pixel *source, Pixel *dest, int swidth, int expand_x);

extern void pj_vdrv_tblit_line(Pixel *source, Pixel *dest, int width, Pixel tcolor);

extern void pj_vdrv_put_rectram(Raster *raster, void *pixbuf, int x, int y, int w, int h);

extern void pj_vdrv_blit_to_ram(Raster	*source, Coor src_x, Coor src_y,
					  Raster *dest, Coor dest_x, Coor dest_y,
					  Coor width, Coor height);

extern void pj_vdrv_blit_from_ram(Raster *source, Coor src_x, Coor src_y,
						Raster	*dest, Coor dest_x, Coor dest_y,
						Coor width, Coor height);

extern void pj_vdrv_mask1line(void);
extern void pj_vdrv_mask2line(void);
extern void pj_vdrv_memcpy(UBYTE *dest, UBYTE *source, int count);

#endif /* DRVCOMN_H */
