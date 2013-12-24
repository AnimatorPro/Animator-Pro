/*  RASTER.C
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


#include "syslib.h"
#include "raster.h"
#include "compaq.h"


Errcode close_raster(VRaster *r)
{
    return Success;
}


void unlccomp_rect(VRaster *r,void *ucbuf, LONG pixsize,
			Coor xorg, Coor yorg, Ucoor width, Ucoor height)
{
/*
   Uncompress data into a rectangular area inside raster using
   byte-run-length/delta compression scheme used in Autodesk Animator 1.0
   for most frames except the first.
   (Unclipped.)
*/

static short   x,y;
static short   lines;
static char    opcount;
static short   psize;
static char    *cpt;
static short   *wpt;


    wpt = (short *) ucbuf;
    cpt = (char *)(wpt+2); /* Data starts with 2 16 bit quantities then bytes */
    y = yorg + *wpt++;

    lines = *wpt++;
    while (--lines >= 0) {
	x = xorg;
	opcount = *cpt++;
	while (opcount > 0) {
	    x += *cpt++;
	    psize = *cpt++;
	    if (psize & 0x80) {
		psize = 256 - psize;
		set_rect(r, *cpt++, x, y, psize, 1);
		x += psize;
		opcount -= 1;
	    } else {
		put_rectpix(r, cpt, x, y, psize, 1);
		cpt += psize;
		x += psize;
		opcount -= 1;
	    }
	}
	y++;
    }
}

void unss2_rect(VRaster *r,void *ucbuf, LONG pixsize,
		     Coor xstart, Coor ystart, Ucoor width, Ucoor height)
{
/*
   Uncompress data into a rectangular area inside raster using
   word-run-length/delta compression scheme used in Autodesk Animator 386
   for most frames except the first.

   (Unclipped.)
*/

static unsigned short  x, y;
static short	       lp_count;
static short	       op_word;
static short	       skip_size;
static union	       {short *w; unsigned char *ub; char *b;} wpt;
static short	       lastx;
#define SBUF_SIZE   1024
static char	       sbuf[SBUF_SIZE];
static short	       *linebuf;
static short	       *word, word_val;
static int	       i;


    /*	allocate the line buffer
    */
    if(width > SBUF_SIZE)
    {
	if ((linebuf = (short *) malloc(width * sizeof(char))) == NULL) {
	    return;
	}
    } else {
	linebuf = (short *) sbuf;
    }

    /*	x-value of the last pixel in the line
    */
    lastx = xstart + width - 1;

    /*	wpt will walk the uncompress frame buffer
    */
    wpt.w = (short *) ucbuf;

    /*	get line count
    */
    lp_count = *wpt.w++;

    /*	get current line
    */
    y = ystart;

    /*	set current column
    */
    lastx = xstart + width - 1;

LINELOOP:

    /*	if line counter is zero, then done
    */
    if (lp_count == 0) {
	goto OUT;
    }

    /*	get next op word
    */
    op_word = *wpt.w++;

    /*	if bits 15 and 14 are set, then skip to appropriate line
    */
    if ((op_word & 0xc000) == 0xc000) {
	y -= op_word;
	goto LINELOOP;
    }

    /*	if only bit 15 is set, then put the lower byte on the image
    */
    if (op_word & 0x8000) {
	put_dot(r, op_word & 0xff, lastx, y);
	goto LINELOOP;
    }

    x = xstart;

DELTALOOP:

    if (op_word == 0) {
	y++;
	lp_count--;
	goto LINELOOP;
    }

    x += *wpt.ub++;

    skip_size = *wpt.ub++;

    if (skip_size & 0x80) {
	skip_size = 256 - skip_size;

	word = linebuf;
	word_val = *wpt.w;
	for (i = 0; i < skip_size; i++) {
	    *word++ = word_val;
	}

	skip_size <<= 1;
	put_rectpix(r, linebuf, x, y, skip_size, 1);
	x += skip_size;
	wpt.w++;

    } else {
	skip_size <<= 1;
	put_rectpix(r, wpt.ub, x, y, skip_size, 1);
	x += skip_size;
	wpt.ub += skip_size;
    }

    op_word--;

    goto DELTALOOP;

OUT:
    /*	if linebuf was allocated using 'malloc()', then free it
    */
    if (linebuf != (short *) sbuf) {
	free(linebuf);
    }
}



static struct rastlib raster_library;

struct rastlib *get_rlib(Vdevice *dev, int mode, Raster *r)
{
    static got_lib = 0;

    if (!got_lib) {

	raster_library.close_raster = close_raster;
	raster_library.set_colors = set_colors;
	raster_library.put_dot = put_dot;
	raster_library.get_dot = get_dot;
	raster_library.wait_vsync = wait_vsync;
	raster_library.cput_dot = cput_dot;
	raster_library.cget_dot = cget_dot;
	raster_library.put_hseg = put_hseg;
	raster_library.get_hseg = get_hseg;
	raster_library.put_vseg = put_vseg;
	raster_library.get_vseg = get_vseg;
	raster_library.put_rectpix = put_rectpix;
	raster_library.get_rectpix = get_rectpix;
	raster_library.set_hline = set_hline;
	raster_library.set_vline = set_vline;
	raster_library.set_rect = set_rect;
	raster_library.set_rast = set_rast;
	raster_library.xor_rect = xor_rect;
	raster_library.uncc64 = uncc64;
	raster_library.uncc256 = uncc256;
	raster_library.unss2_rect = unss2_rect;
	raster_library.unlccomp_rect = unlccomp_rect;

	got_lib = 1;
    }

    return &raster_library;
}
