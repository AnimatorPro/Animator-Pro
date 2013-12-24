/* grcflird.c - Generic lib calls for fli decompressing only and a call to
 * load them into a library */

#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "rastcall.h"
#include "rastlib.h"

static void _grc_put_hseg(const Raster *v, Pixel *pixbuf,
	Ucoor x, Ucoor y, Ucoor width)
/* Move pixels from memory to a horizontal line of destination raster. */
/* (Unclipped) */
{
while (width-- > 0)
	PUT_DOT(v, *pixbuf++, x++, y);
}

static void grc_put_rectpix(const Raster *v,Pixel *pixbuf,
	Ucoor x,Ucoor y,Ucoor width,Ucoor height)
/* Move pixels from a memory buffer into a rectangular area of the
   destination raster. (Much like a blit, but assumes source is in
   memory, and all of source is used.) */
/* (Clipped.) */
{
while(height--)
	{
	pj_put_hseg(v,pixbuf,x,y++,width);
	pixbuf = OPTR(pixbuf,width);
	}
}

static void _grc_set_hline(const Raster *v, Pixel color,
	Ucoor x, Ucoor y, Ucoor width)
/* Draw a solid horizontal line. */
/* (Unclipped) */
{
while (width-- > 0)
	PUT_DOT(v, color, x++, y);
}

static void _grc_set_rect(const Raster *v, Pixel color,
	Ucoor x, Ucoor y, Ucoor width, Ucoor height)
/* Set a rectangular piece of the raster to a solid color. */
/* (Unclipped) */
{
while (height-- > 0)
	{
	SET_HLINE(v, color, x, y++, width);
	}
}

static void grc_set_rast(const Raster *v, Pixel color)
/* Set entire raster to a solid color. */
{
	SET_RECT(v,color,0,0,v->width,v->height);
}

static void grc_unbrun_rect(const Raster *v, void *ucbuf, const LONG pixsize,
	Coor xorg, Coor yorg, Coor width, Coor height)
/* Uncompress data into a rectangular area inside raster using
   byte-run-length compression scheme used in Autodesk Animator 1.0
   for the first frame of a FLI. */
/* Note the width/height passed to this currently must be the same
   as the width/height in the fli header. */
/* (Unclipped.) */
{
Coor x,y;
BYTE psize;
BYTE *cpt;
Coor end;

	y = yorg;
	cpt = ucbuf;
	end = xorg + width;
	while (--height >= 0)
	{
		x = xorg;
		cpt += 1;	/* skip over obsolete opcount byte */
		psize = 0;
		while ((x+=psize) < end)
		{
			psize = *cpt++;
			if (psize >= 0)
			{
				SET_HLINE(v, *cpt++, x, y, psize);
			}
			else
			{
				psize = -psize;
				PUT_HSEG(v, cpt, x, y, psize);
				cpt += psize;
			}
		}
	y++;
	}
}

static void grc_unlccomp_rect(const Raster *v, void *ucbuf, LONG pixsize,
	Coor xorg, Coor yorg, Coor width, Coor height)
/* Uncompress data into a rectangular area inside raster using
   byte-run-length/delta compression scheme used in Autodesk Animator 1.0
   for most frames except the first. */
/* (Unclipped.) */
{
Coor x,y;
SHORT lines;
UBYTE opcount;
BYTE psize;
UBYTE *cpt;
SHORT *wpt;

wpt = ucbuf;
cpt = (UBYTE *)(wpt+2); /* Data starts with 2 16 bit quantities then bytes */
y = yorg + *wpt++;
lines = *wpt++;
while (--lines >= 0)
	{
	x = xorg;
	opcount = *cpt++;
	while (opcount > 0)
		{
		x += *cpt++;
		psize = *cpt++;
		if (psize < 0)
			{
			psize = -psize;
			SET_HLINE(v, *cpt++, x, y, psize);
			x += psize;
			opcount-=1;
			}
		else
			{
			PUT_HSEG(v, cpt, x, y, psize);
			cpt += psize;
			x += psize;
			opcount -= 1;
			}
		}
	y++;
	}
}

static Errcode grc_unss2_rect(const Raster *v, void *ucbuf, LONG pixsize,
	Coor xorg, Coor yorg, Coor width, Coor height)
/* Uncompress data into a rectangular area inside raster using
   word-run-length/delta compression scheme used in Autodesk Animator 386
   for most frames except the first. */
/* (Unclipped.) */
{
Coor x,y;
SHORT lp_count;
SHORT opcount;
int psize;
union {SHORT *w; UBYTE *ub; BYTE *b;} wpt;
Coor lastx;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
Pixel *linebuf;

	if(width > Array_els(sbuf))
	{
		if ((linebuf = pj_malloc(width*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}
	else
		linebuf = sbuf;

	lastx = xorg + width - 1;
	wpt.w = ucbuf;
	lp_count = *wpt.w++;
	y = yorg;
	goto LPACK;

SKIPLINES:

	y -= opcount;

LPACK:	/* do next line */
	if ((opcount = *wpt.w++) >= 0)
		goto DO_SS2OPS;

	if( ((USHORT)opcount) & 0x4000) /* skip lines */
		goto SKIPLINES;

	PUT_DOT(v,(UBYTE)opcount,lastx,y); /* put dot at eol with low byte */
	if((opcount = *wpt.w++) == 0)
	{
		++y;
		if (--lp_count > 0)
			goto LPACK;
		goto OUT;
	}
DO_SS2OPS:
	x = xorg;

PPACK:				/* do next packet */
	x += *wpt.ub++;
	psize = *wpt.b++;
	if ((psize += psize) >= 0)
	{

#ifdef DEBUG
		if(!psize)
			boxf("zero putsize opcnt %d", opcount);
#endif

		PUT_HSEG(v, wpt.ub, x, y, psize);
		x += psize;
		wpt.ub += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
	else
	{
		psize = -psize;
		if (wpt.b[0] == wpt.b[1])
		{
#ifdef DEBUG
			if((x + psize) >= v->width
				|| x < 0
				|| y < 0
				|| y >= v->height
				|| psize <= 0)
			{
				boxf("oops out of screen bounds x %d y %d sz %d oc %d",
					  x, y, psize, opcount );
			}
#endif
			SET_HLINE(v, *wpt.b, x, y, psize);
			++wpt.w;
			x += psize;
			if (--opcount != 0)
				goto PPACK;
			++y;
			if (--lp_count > 0)
				goto LPACK;
		}
		else
		{
#ifdef DEBUG
			if((psize>>1) == 0)
				boxf("zero stuff count");
#endif

			pj_stuff_words(*wpt.w++, (SHORT *)linebuf, psize>>1);
			PUT_HSEG(v, linebuf, x, y, psize);
			x += psize;
			if (--opcount != 0)
				goto PPACK;
			++y;
			if (--lp_count > 0)
				goto LPACK;
		}
	}
OUT:
	if(linebuf != sbuf)
		pj_free(linebuf);
	return(Success);
}


static void grc_uncc64(Raster *s, UBYTE *cbuf)
/* Update raster's palette from compressed source */
{
int ops;
ULONG count;
ULONG start;
SHORT *wp;
UBYTE shift_kludge[256*3];

start = 0;
wp = (SHORT *)cbuf;
ops = *wp;
cbuf += sizeof(*wp);
while (--ops >= 0)
	{
	start += *cbuf++;
	if ((count = *cbuf++) == 0)
		count = 256;
	pj_shift_cmap(cbuf,shift_kludge,count*3);
	pj_set_colors(s, start, count, shift_kludge);
	cbuf += 3*count;
	start += count;
	}
}

static void grc_uncc256(Raster *s, UBYTE *cbuf)
/* Update raster's palette from compressed source */
{
int ops;
ULONG count;
ULONG start;
SHORT *wp;

start = 0;
wp = (SHORT *)cbuf;
ops = *wp;
cbuf += sizeof(*wp);
while (--ops >= 0)
	{
	start += *cbuf++;
	if ((count = *cbuf++) == 0)
		count = 256;
	SET_COLORS(s, start, count, cbuf);
	cbuf += 3*count;
	start += count;
	}
}

void pj_grc_load_dcompcalls(struct rastlib *lib)
/* loads player relevant subset of generic calls into a library */
{
	pj_grc_load_commcalls(lib);

	lib->put_hseg = _grc_put_hseg;
	lib->put_rectpix = grc_put_rectpix;
	lib->set_hline = _grc_set_hline;
	lib->set_rect = _grc_set_rect;
	lib->set_rast = grc_set_rast;

	lib->unbrun_rect = grc_unbrun_rect;
	lib->unlccomp_rect = grc_unlccomp_rect;
	lib->unss2_rect = grc_unss2_rect;

	lib->uncc64 = grc_uncc64;
	lib->uncc256 = grc_uncc256;
}
