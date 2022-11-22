/*****************************************************************************
 * DRVBLTC.C - Higher-order blit functions for the common driver.
 *
 *	Notes:
 *
 *		The routines in this module implement most of the higher-order blit
 *		functions, such as blit_in_card().	Some of them work in conjunction
 *		with service routines in DRVBLTA.ASM.
 *
 *		For the most part, these routines work in a manner similar to the
 *		Animator generic library routines, except we use a 16k buffer, while
 *		the generic library uses a 1k buffer.  For vesa, we get a big win
 *		with the 16k buffer because we can do more at a time without needing
 *		a bank switch.	This is especially true of the in_card() functions
 *		on hardware that only supports one video window used for both reading
 *		and writing.
 *
 *		When developing code for specific hardware, it should be entirely
 *		possible to outperform these routines using hardware-specific tricks,
 *		especially when separate read & write windows are available, and
 *		when the hardware can do multi-byte internal transfers.
 *
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
*	Autodesk makes no warrantees, express or implied, as to the 		  *
*	correctness of this code or any derivative works which incorporate		  *
*	it.  Autodesk provides the code on an ''as-is'' basis and                 *
*	explicitly disclaims any liability, express or implied, for 		  *
*	errors, omissions, and other problems in the code, including			  *
*	consequential and incidental damages.									  *
*																			  *
*	Use, duplication, or disclosure by the U.S.  Government is				  *
*	subject to restrictions set forth in FAR 52.227-19 (Commercial			  *
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)		  *
*	(1) (ii) (Rights in Technical Data and Computer Software, as			  *
*	applicable. 														  *
*																			  *
******************************************************************************/

/*
	4/17/91  - jdb - adjusted some function parameters so they'd match
			 rastlib.h, and then objects that used to be (Bytemap *)
			 were cast to (Bytemap *) when necessary (field 'bm').
	5/28/91  - ian - moved mask1blit/mask2blit routines from raster.c into
			 this module (since they are blits, after all).
*/

#include "drvcomn.h"

void pj_vdrv_blit_in_card(Raster *source, Coor src_x, Coor src_y,
					   Raster *dest, Coor dest_x, Coor dest_y,
					   Coor width, Coor height)
/*****************************************************************************
 * blit a rectangle within the display raster.
 *	we do as many lines at a time as we can fit in our local buffer.
 ****************************************************************************/
{
	int  thisheight;
	int  maxheight;
	void *pbuf = pj_vdrv_wcontrol.localbuf;

	maxheight = LCLBUF_SIZE / width;

	while (height > 0) {
		thisheight = (maxheight <= height) ? maxheight : height;
		pj_vdrv_get_rectpix(source, pbuf, src_x, src_y, width, thisheight);
		pj_vdrv_put_rectpix(dest, pbuf, dest_x, dest_y, width, thisheight);
		height -= thisheight;
		src_y  += thisheight;
		dest_y += thisheight;
	}
}

void pj_vdrv_swap_in_card(Raster  *rast1, Coor x1, Coor y1,
					   Raster  *rast2, Coor x2, Coor y2,
					   Coor width, Coor height)
/*****************************************************************************
 * exchange two rectangles within the display raster.
 ****************************************************************************/
{
	int  thisheight;
	int  maxheight;
	void *pbuf1 = pj_vdrv_wcontrol.localbuf;
	void *pbuf2 = ((UBYTE *)pbuf1) + (LCLBUF_SIZE / 2);

	maxheight = (LCLBUF_SIZE / 2)  / width;

	while (height > 0) {
		thisheight = (maxheight <= height) ? maxheight : height;
		pj_vdrv_get_rectpix(rast1, pbuf1, x1, y1, width, thisheight);
		pj_vdrv_get_rectpix(rast2, pbuf2, x2, y2, width, thisheight);
		pj_vdrv_put_rectpix(rast1, pbuf2, x1, y1, width, thisheight);
		pj_vdrv_put_rectpix(rast2, pbuf1, x2, y2, width, thisheight);
		height -= thisheight;
		y1	+= thisheight;
		y2	+= thisheight;
	}
}

void pj_vdrv_swap_to_ram(Raster  *hrast, Coor hx, Coor hy,
					  Raster *rrast, Coor rx, Coor ry,
					  Coor width, Coor height)
/*****************************************************************************
 * swap a rectangle between the display raster and a bytemap raster.
 ****************************************************************************/
{
	int  thisheight;
	int  maxheight;
	void *pbuf = pj_vdrv_wcontrol.localbuf;

	maxheight = LCLBUF_SIZE / width;

	while (height > 0) {
		thisheight = (maxheight <= height) ? maxheight : height;
		pj_vdrv_get_rectpix(hrast, pbuf, hx, hy, width, thisheight);
		pj_vdrv_blit_from_ram(rrast, rx, ry, hrast, hx, hy,
							width, thisheight);
		pj_vdrv_put_rectram(rrast, pbuf, rx, ry, width, thisheight);
		height -= thisheight;
		hy	+= thisheight;
		ry	+= thisheight;
	}
}

void pj_vdrv_swap_from_ram(Raster *rrast, Coor rx, Coor ry,
						Raster	*hrast, Coor hx, Coor hy,
						Coor width, Coor height)
/*****************************************************************************
 * swap a rectangle between a bytemap raster and the display raster.
 ****************************************************************************/
{
	int  thisheight;
	int  maxheight;
	void *pbuf = pj_vdrv_wcontrol.localbuf;

	maxheight = LCLBUF_SIZE / width;

	while (height > 0) {
		thisheight = (maxheight <= height) ? maxheight : height;
		pj_vdrv_get_rectpix(hrast, pbuf, hx, hy, width, thisheight);
		pj_vdrv_blit_from_ram(rrast, rx, ry, hrast, hx, hy,
							width, thisheight);
		pj_vdrv_put_rectram(rrast, pbuf, rx, ry, width, thisheight);
		height -= thisheight;
		hy += thisheight;
		ry += thisheight;
	}
}

void pj_vdrv_tblit_in_card(Raster  *srast, Coor sx, Coor sy,
						Raster	*drast, Coor dx, Coor dy,
						Coor width, Coor height, Pixel tcolor)
/*****************************************************************************
 * do a transparent blit within the display raster.
 ****************************************************************************/
{
	int  thisheight;
	int  maxheight;
	void *pbuf1 = pj_vdrv_wcontrol.localbuf;
	void *pbuf2 = ((UBYTE *)pbuf1) + (LCLBUF_SIZE / 2);

	maxheight = (LCLBUF_SIZE / 2)  / width;

	while (height > 0) {
		thisheight = (maxheight <= height) ? maxheight : height;
		pj_vdrv_get_rectpix(srast, pbuf1, sx, sy, width, thisheight);
		pj_vdrv_get_rectpix(drast, pbuf2, dx, dy, width, thisheight);
		pj_vdrv_tblit_line(pbuf1, pbuf2, width*thisheight, tcolor);
		pj_vdrv_put_rectpix(drast, pbuf2, dx, dy, width, thisheight);
		height -= thisheight;
		sy += thisheight;
		dy += thisheight;
	}
}

void pj_vdrv_tblit_to_ram(Raster  *hrast, Coor hx, Coor hy,
					   Raster *rrast, Coor rx, Coor ry,
					   Coor width, Coor height, Pixel tcolor)
/*****************************************************************************
 * do a transparent blit from the display raster to a bytemap raster.
 ****************************************************************************/
{
	int bpr = ((Bytemap *)rrast)->bm.bpr;
	Pixel	*pram = ((Bytemap *)rrast)->bm.bp[0] + rx + ry * bpr;
	Pixel	*pbuf = pj_vdrv_wcontrol.localbuf;

	while (--height >= 0) {
		pj_vdrv_get_hseg(hrast, pbuf, hx, hy, width);
		pj_vdrv_tblit_line(pbuf, pram, width, tcolor);
		++hy;
		pram += bpr;
	}
}

void pj_vdrv_tblit_from_ram(Raster *rrast, Coor rx, Coor ry,
						 Raster  *hrast, Coor hx, Coor hy,
						 Coor width, Coor height, Pixel tcolor)
/*****************************************************************************
 * do a transparent blit from a bytemap raster to the display raster.
 ****************************************************************************/
{
	int bpr = ((Bytemap *)rrast)->bm.bpr;
	Pixel	*pram = ((Bytemap *)rrast)->bm.bp[0] + rx + ry * bpr;
	Pixel	*pbuf = pj_vdrv_wcontrol.localbuf;

	while (--height >= 0) {
		pj_vdrv_get_hseg(hrast, pbuf, hx, hy, width);
		pj_vdrv_tblit_line(pram, pbuf, width, tcolor);
		pj_vdrv_put_hseg(hrast, pbuf, hx, hy, width);
		++hy;
		pram += bpr;
	}
}

void pj_vdrv_zoom_from_ram(Raster *rrast, Coor rx, Coor ry,
						Raster	*hrast, Coor hx, Coor hy,
						Ucoor width,  Ucoor height,
						LONG zoom_x, LONG zoom_y)
/*****************************************************************************
 * zoom data froma bytemap raster to the display raster.
 ****************************************************************************/
{
	int zy;
	int swidth;
	int bpr = ((Bytemap *)rrast)->bm.bpr;
	UBYTE	*pram = ((Bytemap *)rrast)->bm.bp[0] + rx + ry * bpr;
	UBYTE	*pbuf = pj_vdrv_wcontrol.localbuf;

	swidth = (width+zoom_x-1) / zoom_x;

	while (height > 0) {
		zy = (zoom_y <= height) ? zoom_y : height;
		pj_vdrv_zoom_line(pram, pbuf, swidth, zoom_x);
		height -= zy;
		pram += bpr;
		while (--zy >= 0)
			pj_vdrv_put_hseg(hrast, pbuf, hx, hy++, width);
	}
}

void pj_vdrv_zoom_in_card(Raster  *rast1, Coor x1, Coor y1,
					   Raster  *rast2, Coor x2, Coor y2,
					   Ucoor width, Ucoor height,
					   LONG zoom_x, LONG zoom_y)
/*****************************************************************************
 * do a zoom within the display raster.
 ****************************************************************************/
{
	int zy;
	int swidth;
	int sheight;
	int thisheight;
	int maxheight;
	UBYTE	*outbuf = pj_vdrv_wcontrol.localbuf;
	UBYTE	*inbuf;
	UBYTE	*pbuf;

	swidth	= (width+zoom_x-1) / zoom_x;
	sheight = (height+zoom_y-1) / zoom_y;
	inbuf = outbuf + width + zoom_x;
	maxheight = (LCLBUF_SIZE-width-zoom_x) / swidth;

	while (height > 0) {
		thisheight = (maxheight <= sheight) ? maxheight : sheight;
		pj_vdrv_get_rectpix(rast1, inbuf, x1, y1, swidth, thisheight);
		sheight -= thisheight;
		y1 += thisheight;
		pbuf = inbuf;
		while (height > 0 && thisheight > 0) {
			zy = (zoom_y <= height) ? zoom_y : height;
			pj_vdrv_zoom_line(pbuf, outbuf, swidth, zoom_x);
			height -= zy;
			pbuf += swidth;
			--thisheight;
			while (--zy >= 0)
				pj_vdrv_put_hseg(rast2, outbuf, x2, y2++, width);
		}
	}
}

void pj_vdrv_zoom_to_ram(Raster  *hrast, Coor hx, Coor hy,
					  Raster *rrast, Coor rx, Coor ry,
					  Ucoor width,	Ucoor height,
					  LONG zoom_x, LONG zoom_y)
/*****************************************************************************
 * do a zoom from the display raster to a bytemap raster.
 ****************************************************************************/
{
	int zy;
	int swidth;
	int bpr = ((Bytemap *)rrast)->bm.bpr;
	UBYTE	*pram = ((Bytemap *)rrast)->bm.bp[0] + rx + ry * bpr;
	UBYTE	*inbuf = pj_vdrv_wcontrol.localbuf;
	UBYTE	*outbuf;

	swidth = (width+zoom_x-1) / zoom_x;
	outbuf = inbuf + swidth;

	while (height > 0) {
		zy = (zoom_y <= height) ? zoom_y : height;
		pj_vdrv_get_hseg(hrast, inbuf, hx, hy++, swidth);
		pj_vdrv_zoom_line(inbuf, outbuf, swidth, zoom_x);
		height -= zy;
		while (--zy >= 0) {
			pj_vdrv_memcpy(pram, outbuf, width); /* is this sleezy or what? */
			pram += bpr;
		}
	}
}

void pj_vdrv_mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   Raster *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
				   Pixel oncolor)
/*****************************************************************************
 * drive the mask1blit process...
 *	this routine pre-calcs some values used by the low-level blit routine,
 *	then repeatedly calls the low-level routine to process each line.
 ****************************************************************************/
{
	pj_vdrv_wcontrol.mbytes  = mbytes + (mx >> 3) + (my * mbpr);
	pj_vdrv_wcontrol.bit1	 = 0x0080 >> (mx & 7);
	pj_vdrv_wcontrol.count	 = rw;
	pj_vdrv_wcontrol.oncolor = oncolor;
	pj_vdrv_wcontrol.rx  = rx;
	pj_vdrv_wcontrol.ry  = ry;

	while (rh--)
		{
		pj_vdrv_mask1line();
		++pj_vdrv_wcontrol.ry;
		pj_vdrv_wcontrol.mbytes += mbpr;
		}
}

void pj_vdrv_mask2blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   Raster *r, Coor rx, Coor ry, Ucoor rw, Ucoor rh,
				   Pixel oncolor, Pixel offcolor)
/*****************************************************************************
 * drive the mask2blit process...
 *	this routine pre-calcs some values used by the low-level blit routine,
 *	then repeatedly calls the low-level routine to process each line.
 ****************************************************************************/
{
	pj_vdrv_wcontrol.mbytes   = mbytes + (mx >> 3) + (my * mbpr);
	pj_vdrv_wcontrol.bit1	  = 0x0080 >> (mx & 7);
	pj_vdrv_wcontrol.count	  = rw;
	pj_vdrv_wcontrol.oncolor  = oncolor;
	pj_vdrv_wcontrol.offcolor = offcolor;
	pj_vdrv_wcontrol.rx   = rx;
	pj_vdrv_wcontrol.ry   = ry;

	while (rh--)
		{
		pj_vdrv_mask2line();
		++pj_vdrv_wcontrol.ry;
		pj_vdrv_wcontrol.mbytes += mbpr;
		}
}

