
#define RASTCOMP_INTERNALS
#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"
#include "fli.h"
#include "errcodes.h"

//#include "pjbasics.h"

#define START_SIXCUBE 0

void pj_make_pstamp_xlat(Rgb3 *ctab,register UBYTE *xlat,int count)
{
	if(count > COLORS)
		count = COLORS;

	while(count-- > 0)
	{
		*xlat++ = 6*ctab->r/RGB_MAX * 36 + 6*ctab->g/RGB_MAX * 6
							+ 6*ctab->b/RGB_MAX + START_SIXCUBE;
		++ctab;
	}
}

typedef struct brun_dat {
	Rcel *screen;				/* screen being read */
	Pixel *srcline; 			/* line from source screen */
	UBYTE xlat[COLORS]; 		/* color translation table */
	Pixel dline[PSTAMP_W];		/* destination line */
	BYTE *dst;					/* destination compressed record */
	BYTE *dstmax;				/* maximum destination location */
	SHORT xtable[PSTAMP_W+1];	/* scaling table */
	SHORT last_sy;
	int sx, sw; 				/* source x offset and width */
	int dw; 					/* destination width */
	char *(*comp_line)(BYTE *src,BYTE *dst,int width);
} Brundat;


static char *copy_line(BYTE *src,BYTE *dst,int width)
/* a compression function that just copies */
{
	pj_copy_bytes(src,dst,width);
	return((char *)(dst + width));
}

/* external brun compressor */

extern char *pj_brun_comp_line(char *src, char *cbuf, int count);


static void comp_pstamp_line(int sy, int dy, Brundat *brd)

{
Pixel *spix;
Pixel *dpix;
Pixel *maxdpix;
SHORT *xtable;

	if(brd->dst)
	{
		if(brd->last_sy != sy)
		{
			pj_get_hseg(brd->screen,
						(spix = brd->srcline),
						brd->sx,
						(brd->last_sy = sy),
						brd->sw);

			dpix = brd->dline;
			maxdpix = dpix + brd->dw;
			xtable = brd->xtable;

			while(dpix < maxdpix)
				*dpix++ = spix[*xtable++];

			pj_xlate(brd->xlat,brd->dline,brd->dw);
		}
		if((brd->dst = (BYTE *)(*(brd->comp_line))((BYTE *)brd->dline,
											brd->dst,brd->dw)) > brd->dstmax)
		{
			brd->dst = NULL; /* will fail test above */
		}
	}
}
void pj_get_stampsize(SHORT maxw, SHORT maxh, SHORT sw, SHORT sh,
					  SHORT *pw, SHORT *ph)

/* will give divide error if sw or sh is 0 */
{
int w,h;

	/* attempt to scale to height, if width is too big scale to width */

	if((w = pj_uscale_by(sw, maxh, sh)) > maxw)
	{
		w = pj_uscale_by(sw, maxw, sw);
		h = pj_uscale_by(sh, maxw, sw);
	}
	else
		h = pj_uscale_by(sh, maxh, sh);

	if(h <= 0)
		h = 1;
	if(w <= 0)
		w = 1;
	*ph = h;
	*pw = w;
}

LONG pj_build_rect_pstamp(Rcel *screen, void *cbuf,
						   SHORT x,SHORT y,USHORT width,USHORT height)
{
Brundat brd;
Pstamp_chunk *chunk = (Pstamp_chunk *)cbuf;
LONG square;

	pj_get_stampsize(PSTAMP_W,PSTAMP_H,width,height,
					 &chunk->width,&chunk->height);

	chunk->xlat_type = PSTAMP_SIXCUBE;
	pj_make_pstamp_xlat(screen->cmap->ctab,brd.xlat, COLORS);

	/* a little one only needs the color translation table */

	square = width*height;
	if( (chunk->width * chunk->height) > (square - (square/3)) )
	{
		brd.dst = OPTR(chunk,(sizeof(*chunk)+256));
		chunk->data.type = FPS_XLAT256;
		pj_copy_bytes(brd.xlat,chunk+1,256);
		goto chunk_done;
	}

	/* try to do a brun, if too big do a copy */

	chunk->data.type = FPS_BRUN;
	brd.screen = screen;
	brd.sx = x;
	brd.sw = width;
	brd.comp_line = pj_brun_comp_line;
	brd.dstmax = (BYTE *)(chunk+1) + PSTAMP_W*(PSTAMP_H-1);

	for(;;)
	{
		brd.srcline = (Pixel *)(brd.dstmax);
		brd.last_sy = y - 1;
		brd.dw = chunk->width;
		pj_make_scale_table(width,brd.dw,brd.xtable);
		brd.dst = (BYTE *)(chunk + 1);

		pj_do_linscale(y,height,0,chunk->height,comp_pstamp_line,&brd);
		if(brd.dst != NULL)
			break;
		chunk->data.type = FPS_COPY;
		brd.comp_line = copy_line;
		brd.dstmax = (BYTE *)(chunk+1) + PSTAMP_W*(PSTAMP_H+1);
	}

chunk_done:

	chunk->type = FLI_PSTAMP;
	chunk->size = SIZE(chunk,brd.dst);
	if(chunk->size & 1) /* make sure it's an even size */
	{
		++chunk->size;
		++brd.dst;
	}
	chunk->data.size = SIZE(&chunk->data,brd.dst);
	return(chunk->size);
}

