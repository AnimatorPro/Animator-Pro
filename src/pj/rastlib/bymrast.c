#include "rastcall.ih"
#include "errcodes.h"
#include "memory.h"

/* #define USE_OPTIMISED_RASTLIB */

static Errcode close_bytemap(Raster *bytemap)
/* frees parts but does not free Bytemap */
{
	Bytemap *rr = (Bytemap *)bytemap;
	if(rr->type != RT_BYTEMAP)
		return(Err_rast_type);
	pj_free_bplanes(&(rr->bm.bp[0]),rr->bm.num_planes);
	return(0);
}
static Errcode setup_bytemap(Rasthdr *spec, Bytemap *bmap)

/* Initialize libraries and setup field values */
{
	if(!spec->width || !spec->height)
		return(Err_bad_input);

	if((void *)bmap != (void *)spec)
		*((Rasthdr *)bmap) = *spec; /* move in header */
	clear_mem(&bmap->bm,sizeof(bmap->bm)); /* clear bmap */
	bmap->type = RT_BYTEMAP;
	bmap->lib = pj_get_bytemap_lib();
	bmap->bm.bpr = Bytemap_bpr(bmap->width);
	bmap->bm.num_planes = (bmap->pdepth+7) >> 3;

#ifdef USE_OPTIMISED_RASTLIB
	bmap->bm.segment = pj_get_ds();
#else /* USE_OPTIMISED_RASTLIB */
	bmap->bm.segment = 0;
#endif /* USE_OPTIMISED_RASTLIB */

	bmap->bm.psize = (ULONG)bmap->bm.bpr * bmap->height;
	return(Success);
}
Errcode pj_open_bytemap(Rasthdr *spec, Bytemap *rr)

/* will allocate the parts of a ramrast, initialize libraries and setup
 * field values */
/* use pj_close_raster() to close this screen. it takes
 * width, height, pdepth, aspect_dx, and aspect_dy from the spec Raster
 * struct provided.  All other fields in the raster struct are ignored.
 * no fields in the spec raster struct are altered. */
{
Errcode err;

	if((err = setup_bytemap(spec,rr)) < Success)
		goto error;
	if(0 > (err=pj_get_bplanes(&(rr->bm.bp[0]),rr->bm.num_planes,rr->bm.psize)))
		goto error;
error:
	return(err);
}
Errcode pj_build_bytemap(Rasthdr *spec,Raster *r, UBYTE *pixels)
{
Errcode err;
#define rr ((Bytemap *)r)

	if((err = setup_bytemap(spec,rr)) < Success)
		return(err);
	rr->bm.bp[0] = pixels;
	return(Success);
#undef rr
}

/************** bytemap jump table driver primitives ****************/

#ifdef USE_OPTIMISED_RASTLIB

/* from assembler library *****/




extern void pj_bym_dto_ptable(Bytemap *r,Pixel *pt, Ucoor psz,
						   Coor x0, Coor y0, Coor x1, Coor y1 );
void pj_bym_put_dot(Bytemap *v, Pixel color, Coor x, Coor y);
void pj_bym_cput_dot(Bytemap *v, Pixel color, Coor x, Coor y);
Pixel pj_bym_get_dot(Bytemap *v, Coor x, Coor y);
void pj_bym_set_vline(Bytemap *r,Pixel color,Ucoor x,Ucoor y,Ucoor height);
void pj_bym_set_hline(Bytemap *r,Pixel color,Ucoor x,Ucoor y,Ucoor width);
Coor pj_bym_get_vseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor height);
Coor pj_bym_put_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);
Coor pj_bym_put_vseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor height);
void pj_bym_set_rast(Bytemap *v, Pixel color);
void pj_bym_set_rect(Bytemap *bm,  Pixel color,
				  Coor x, Coor y, Ucoor width, Ucoor height);
void pj_bym_xor_rect(Bytemap *bm, Pixel color,
				 Coor x, Coor y, Ucoor width, Ucoor height);
void pj_bym_mask1blit(UBYTE *mbytes, Coor mbpr,
				   Coor mx, Coor my,
				   Bytemap *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
				   Pixel oncolor );
void bym_mask2blit(UBYTE *mbytes, Coor mbpr,
						  Coor mx, Coor my,
						  Bytemap *r,Coor rx,Coor ry,Ucoor width, Ucoor height,
						  Pixel oncolor, Pixel offcolor );

void pj_bli8(Bytemap *dv, int dx, int dy, int w, int h, int sx, int sy,
	Bytemap *sv);

void pj_tbli8(Bytemap *dv, int dx, int dy, int w, int h, int sx, int sy,
	Bytemap *sv, int tcolor);
void zoom2blit(int swidth, int dheight,
	int sx, int sy, Pixel *sp, int spseg, int sbpr,
	int dx, int dy, Pixel *dp, int dpseg, int dbpr);
void zoom4blit(int swidth, int dheight,
	int sx, int sy, Pixel *sp, int spseg, int sbpr,
	int dx, int dy, Pixel *dp, int dpseg, int dbpr);
void *pj_unlccomp(void *csource, Pixel *byteplane, int plane_bpr,
	USHORT plane_segment);

/**** written in "c" here *************/

static void bym_get_rectpix(Bytemap *r,void *pixbuf,
						Coor x,Coor y,Ucoor width,Ucoor height)

/* loads all pixels from a screen rectangle into a linear buffer
 * on a line by line basis.
 * For now assumes pixsize is 1 if the rectangle is clipped garbage will
 * be in the clipped off section of the buffer (could be faster) */
{
	while(height--)
	{
		pj_bym_get_hseg(r,pixbuf,x,y++,width);
		pixbuf = OPTR(pixbuf,width);
	}
}
static void bym_put_rectpix(Bytemap *r,void *pixbuf,
							Coor x,Coor y,Ucoor width,Ucoor height)

/* loads all pixels from a linear pixel buffer into a screen rectangle
 * on a line by line basis (could be faster) */
{
	while(height--)
	{
		pj_bym_put_hseg(r,pixbuf,x,y++,width);
		pixbuf = OPTR(pixbuf,width);
	}
}
void pj_bym_mask1blit(UBYTE *mbytes, Coor mbpr,
				   Coor mx, Coor my,
				   Bytemap *r, Coor rx, Coor ry,Ucoor width, Ucoor height,
				   Pixel oncolor )

/* sets rectangle of raster rectangle of mask
	(mask on = oncolor, off = noaction ) */
{
	pj_bli1(r, rx, ry, width, height, mx, my, mbytes, mbpr, oncolor);
}

static void bym_mask2blit(UBYTE *mbytes, Coor mbpr,
						  Coor mx, Coor my,
						  Bytemap *r,Coor rx,Coor ry,Ucoor width, Ucoor height,
						  Pixel oncolor, Pixel offcolor )

/* sets rectangle of raster rectangle of mask
	(mask on = oncolor, off = noaction ) */
{
	pj_bli2(r, rx, ry, width, height, mx, my, mbytes, mbpr, oncolor, offcolor);
}

/*************************************************************************/
/* these are the decompressors that need to be converted to new format and
 * function */

extern void pj_bym_unbrun_rect();

static void bym_unlccomp_rect(Bytemap *r,void *ucbuf, LONG pixsize,
				   LONG x,LONG y,Ucoor width,Ucoor height)
{
	pj_unlccomp(ucbuf, ((Bytemap *)r)->bm.bp[0] + y*r->bm.bpr + x,
		r->bm.bpr,
		r->bm.segment);
}

static void bym_unss2_rect(Bytemap *r,void *ucbuf, LONG pixsize,
				   LONG x,LONG y,Ucoor width,Ucoor height)
{
	pj_unss2(ucbuf, ((Bytemap *)r)->bm.bp[0] + y*r->bm.bpr + x,
		  r->bm.bpr,
		  r->bm.segment,width);
}


/***************** Binary calls that have two rasters ***********/

/* binary calls names are xxx_yyy for same to same xxx_f_yyy() for from
 * bytemap and xxx_t_yyy() for to bytemap
 * note a from other may not needed since it will be the others to other
 * since the calls are symetrical */

static void bym_blitrect(Bytemap *source,			 /* source raster */
					  LONG src_x, LONG src_y,  /* source Minx and Miny */
				  Bytemap *dest,			 /* destination raster */
				  LONG dest_x, LONG dest_y, /* destination minx and miny */
				  LONG width, LONG height)	/* blit size */


/* (should) copys rectangle from source to destination these should handle
 * overlapping by comparing source and dest and branching to routine
 * depending on whether they are the same or not */
{
	pj_bli8(dest, dest_x, dest_y, width, height, src_x, src_y, source);
}

#define bym_f_blitrect bym_blitrect
#define bym_t_blitrect bym_blitrect

static void bym_tblitrect(Bytemap *source,			  /* source raster */
				LONG src_x, LONG src_y, /* source Minx and Miny */
		  Bytemap *dest,			  /* destination raster */
		  LONG dest_x, LONG dest_y, /* destination minx and miny */
		  LONG width, LONG height,	/* blit size */
		  Pixel tcolor )			 /* color to ignore in source */

/* copy all of source that is not tcolor to destination */
{
  pj_tbli8(dest, dest_x, dest_y, width, height,
			src_x, src_y, source, tcolor);
}

#define bym_f_tblitrect bym_tblitrect
#define bym_t_tblitrect bym_tblitrect

static void bym_xor_rast(Bytemap *s, Bytemap *d)
/* xors one raster with raster of same dimensions */
{
#ifdef SAYWHAT
/* ???? xor_group's first parameter is a constant */
	xor_group(s->bm.bp[0], d->bm.bp[0], ((ULONG)d->bm.psize) );
#endif /* SAYWHAT */
}

#define bym_f_xor_rast bym_xor_rast
#define bym_t_xor_rast bym_xor_rast

#else /* USE_OPTIMISED_RASTLIB */

static void
pj_bym_put_dot(Raster *bytemap, Pixel col, Coor x, Coor y)
{
	Bytemap *r = (Bytemap *)bytemap;
	(r->bm.bp[0])[r->bm.bpr * y + x] = col;
}

static Pixel
pj_bym_get_dot(Raster *bytemap, Coor x, Coor y)
{
	Bytemap *r = (Bytemap *)bytemap;
	return (r->bm.bp[0])[r->bm.bpr * y + x];
}

#endif /* USE_OPTIMISED_RASTLIB */

/* library vector tables ****************************************************/
/* I know it would save code to have these vector tables staticly initialized
 * but it is difficult to maintain the order of the entries so this is much
 * more convenient and less prone to error during developement */

Rastlib *pj_get_bytemap_lib(void)
{
static Rastlib bytemap_lib;
static int loaded = 0;

	if(!loaded)
	{
		/* Work around bug in 3D Studio REX loader where static
		 * data isn't set to zero. */
		clear_mem(&bytemap_lib,sizeof(bytemap_lib)); 

		/* Fill in functions that exist. */
		bytemap_lib.close_raster = close_bytemap;
		bytemap_lib.put_dot = pj_bym_put_dot;
		bytemap_lib.get_dot = pj_bym_get_dot;

#ifdef USE_OPTIMISED_RASTLIB

		bytemap_lib.cput_dot = (rl_type_cput_dot)pj_bym_cput_dot;
#ifdef NOTYET
		bytemap_lib.cget_dot = (rl_type_cget_dot)pj_bym_cget_dot;
#endif /* NOTYET */

		bytemap_lib.put_hseg = (rl_type_put_hseg)pj_bym_put_hseg;
		bytemap_lib.get_hseg = (rl_type_get_hseg)pj_bym_get_hseg;
		bytemap_lib.put_vseg = (rl_type_put_vseg)pj_bym_put_vseg;
		bytemap_lib.get_vseg = (rl_type_get_vseg)pj_bym_get_vseg;

		bytemap_lib.get_rectpix = (rl_type_get_rectpix)bym_get_rectpix;
		bytemap_lib.put_rectpix = (rl_type_put_rectpix)bym_put_rectpix;

		bytemap_lib.set_hline = (rl_type_set_hline)pj_bym_set_hline;
		bytemap_lib.set_vline = (rl_type_set_vline)pj_bym_set_vline;

		bytemap_lib.set_rect = (rl_type_set_rect)pj_bym_set_rect;

		bytemap_lib.set_rast = (rl_type_set_rast)pj_bym_set_rast;
		bytemap_lib.xor_rect = (rl_type_xor_rect)pj_bym_xor_rect;
		bytemap_lib.mask1blit = (rl_type_mask1blit)pj_bym_mask1blit;
		bytemap_lib.mask2blit = (rl_type_mask2blit)bym_mask2blit;

		bytemap_lib.unbrun_rect = (rl_type_unbrun_rect)pj_bym_unbrun_rect;
		bytemap_lib.unlccomp_rect = (rl_type_unlccomp_rect)bym_unlccomp_rect;
		bytemap_lib.unss2_rect = (rl_type_unss2_rect)bym_unss2_rect;

		/* binary calls */

		bytemap_lib.blitrect[RL_TO_SAME] 
		=	(rl_type_blitrect)bym_blitrect;
		bytemap_lib.blitrect[RL_TO_BYTEMAP] 
		=	(rl_type_blitrect)bym_t_blitrect;
		bytemap_lib.blitrect[RL_FROM_BYTEMAP] 
		= 	(rl_type_blitrect)bym_f_blitrect;

		bytemap_lib.tblitrect[RL_TO_SAME] 
		=	(rl_type_tblitrect)bym_tblitrect;
		bytemap_lib.tblitrect[RL_TO_BYTEMAP] 
		=	(rl_type_tblitrect)bym_t_tblitrect;
		bytemap_lib.tblitrect[RL_FROM_BYTEMAP] 
		=	(rl_type_tblitrect)bym_f_tblitrect;

		bytemap_lib.xor_rast[RL_TO_SAME] 
		=	(rl_type_xor_rast)bym_xor_rast;
		bytemap_lib.xor_rast[RL_TO_BYTEMAP] 
		=	(rl_type_xor_rast)bym_t_xor_rast;
		bytemap_lib.xor_rast[RL_FROM_BYTEMAP] 
		=	(rl_type_xor_rast)bym_f_xor_rast;

#ifdef NOT_DONE
		bytemap_lib.zoomblit[RL_TO_SAME] 
		=	(rl_type_zoomblit)bym_zoomblit;
		bytemap_lib.zoomblit[RL_TO_BYTEMAP] 
		=	(rl_type_zoomblit)bym_t_zoomblit;
		bytemap_lib.zoomblit[RL_FROM_BYTEMAP] 
		=	(rl_type_zoomblit)bym_f_zoomblit;
#endif /* NOT_DONE */

#ifndef FLILIB_CODE
		bytemap_lib.diag_to_ptable 
		=	(rl_type_diag_to_ptable)pj_bym_dto_ptable;
#endif /* FLILIB_CODE */

#endif /* USE_OPTIMISED_RASTLIB */

		pj_set_grc_calls(&bytemap_lib);
		loaded = 1;
	}
	return(&bytemap_lib);
}

#ifdef SLUFFED

#include "jimk.h"

test_driver()
{
int i,j;
Pixel buf[320];

for (i=0; i<160; i++)
	{
	pj_bym_get_vseg((Bytemap *)(vl.screen->viscel), buf, i, 0, 200);
	pj_bym_put_vseg((Bytemap *)(vl.screen->viscel), buf, 319-i, 0, 200);
	}
for (i=0; i<100; i++)
	{
	pj_bym_get_hseg((Bytemap *)(vl.screen->viscel), buf, 0, i, 320);
	pj_bym_put_hseg((Bytemap *)(vl.screen->viscel), buf, 0, 199-i, 320);
	}
wait_click();
for (i=0; i<256; i+=8)
	pj_bym_set_rast((Bytemap *)(vl.screen->viscel), i);
}
#endif /* SLUFFED */


