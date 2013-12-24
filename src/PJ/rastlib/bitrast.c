#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "rastlib.h"


extern void pj_free_bplanes(register PLANEPTR *bp,LONG num_planes);
extern LONG pj_get_bplanes(PLANEPTR *bp,LONG num_planes,LONG bpize);
static void *get_bitmap_lib();

static Errcode close_bitmap(Bitmap *rr)
/* frees parts but does not free Bitmap */
{
	if(rr->type != RT_BITMAP)
		return(Err_rast_type);
	pj_free_bplanes(&(rr->bm.bp[0]),rr->bm.num_planes);
	return(0);
}
static Errcode setup_bitmap(Rasthdr *spec,Bitmap *bmap)

/* Initialize libraries and setup field values */
{
	if(!spec->width || !spec->height)
		return(Err_bad_input);

	if((void *)bmap != (void *)spec)
		*((Rasthdr *)bmap) = *spec; /* move in header */
	clear_mem(&bmap->bm,sizeof(bmap->bm)); /* clear bmap */
	bmap->type = RT_BITMAP;
	bmap->lib = get_bitmap_lib();
	bmap->bm.bpr = Bitmap_bpr(bmap->width);
	bmap->bm.segment = pj_get_ds();
	bmap->bm.psize = (ULONG)bmap->bm.bpr * bmap->height;
	bmap->bm.num_planes = bmap->pdepth;
	return(Success);
}
Errcode pj_open_bitmap(Rasthdr *spec,Bitmap *rr)

/* will allocate the parts of a ramrast, initialize libraries and setup
 * field values */
{
Errcode err;

	if((err = setup_bitmap(spec,rr)) < Success)
		goto error;
	if(0 > (err=pj_get_bplanes(&(rr->bm.bp[0]),rr->bm.num_planes,rr->bm.psize)))
		goto error;
error:
	return(err);
}
Errcode pj_build_bitmap(Rasthdr *spec,Raster *r, UBYTE *pixels)
{
Errcode err;

	if((err = setup_bitmap(spec,(Bitmap *)r)) < Success)
		return(err);
	((Bitmap *)r)->bm.bp[0] = pixels;
	return(Success);
}


/************** bitmap jump table driver primitives ****************/


static void _bim_put_dot(Bitmap *bm, Pixel color, Coor x, Coor y)
/* set dot to color only one plane for now */
{
	if(color & 0x01)
	   (bm->bm.bp[0])[y*bm->bm.bpr+(x>>3)] |= (0x80>>(x&7));
	else
	   (bm->bm.bp[0])[y*bm->bm.bpr+(x>>3)] &= ~(0x80>>(x&7));
}

static Pixel _bim_get_dot(Bitmap *bm, Coor x, Coor y) /* get color of dot */
/* only one plane for now */
{
int ander;

	ander = 0x80>>(x&7);
	return(((bm->bm.bp[0])[y*bm->bm.bpr+(x>>3)]&ander)/ander);
}
static void _bim_get_hseg(Bitmap *r, Pixel *pixbuf,
						  Ucoor x, Ucoor y, Ucoor width)
/* Move pixels from a horizontal line of source Bitmap to memory buffer. */
/* (Unclipped) */
{
UBYTE byte;
UBYTE *mbytes;
UBYTE bit1;

	mbytes = (UBYTE *)&((r->bm.bp[0])[y*r->bm.bpr + (x>>3)]);
	bit1 = 0x80>>(x&7);
	byte = *mbytes++;

	while (width-- > 0)
	{
		if (bit1 & byte)
			*pixbuf++ = 1;
		else
			*pixbuf++ = 0;

		if ((bit1>>=1) == 0)
		{
			bit1 = 0x80;
			byte = *mbytes++;
		}
	}
}
static void _bim_blitrect(Bitmap *src,			 /* source raster */
						  Coor src_x, Coor src_y,  /* source Minx and Miny */
						  Raster *dest, 			 /* destination raster */
						  Coor dest_x, Coor dest_y,
						  Coor width, Coor height)	/* blit size */
{
	pj_mask2blit(src->bm.bp[0],src->bm.bpr,src_x,src_y,dest,dest_x,dest_y,width,
			  height,1,0);
}
static void bim_set_rast(Bytemap *r,Pixel color)

/* sets entire raster to a color fast only for one plane now */
{
	if(color & 0x01)
	{
		pj_stuff_words((USHORT)(~0),r->bm.bp[0],
					 (USHORT)(r->bm.psize>>1));
		if(r->bm.psize & 0x01)
			r->bm.bp[0][r->bm.psize - 1] = (UBYTE)~0;
	}
	else
	{
		zero_lots(r->bm.bp[0], r->bm.psize);
		if(r->bm.psize & 0x01)
			r->bm.bp[0][r->bm.psize - 1] = (UBYTE)0;
	}
}
static void _bim_xor_rect(Bitmap *bm,
						 Pixel color,
						 Coor x, Coor y,
						 Ucoor width, Ucoor height)
{
register UBYTE *pbyte;
register SHORT bnum;
UBYTE startmask;
UBYTE endmask;
SHORT startbyte;
SHORT endbyte;
SHORT num_planes;
SHORT num_rows;
SHORT rowcount;

	endbyte = startbyte = x;
	startmask = (0xFF)>>(startbyte & 0x07);
	endbyte += width;
	endmask = ~((0xFF)>>(endbyte & 0x07));
	startbyte >>= 3;
	endbyte >>= 3;

	if(startbyte >= bm->bm.bpr)
		return;

	if(startbyte == endbyte)
		startmask &= endmask;
	else if(endbyte >= bm->bm.bpr)
	{
		endbyte = bm->bm.bpr;
		endmask = 0;
	}

	num_planes = bm->bm.num_planes;
	num_rows = height;

	if((num_rows + y) > bm->height)
		num_rows -= (num_rows + y - bm->height);

	while(num_planes > 0)
	{
		--num_planes;

		rowcount = num_rows;
		pbyte = (UBYTE *)(bm->bm.bp[num_planes]);
		pbyte += startbyte + (bm->bm.bpr * y);

		while(rowcount > 0)
		{
			--rowcount;
			bnum = startbyte;
			pbyte[bnum] ^= startmask;

			while(++bnum < endbyte)
				pbyte[bnum] ^= 0xFF;

			if((bnum == endbyte) && endmask)
				pbyte[bnum] ^= endmask;

			pbyte += bm->bm.bpr;
		}
	}
}

static void *get_bitmap_lib(void)
{
static Rastlib bitmap_lib;
static int loaded = 0;

	if(!loaded)
	{
		/* Work around bug in 3D Studio REX loader where static
		 * data isn't set to zero. */
		clear_mem(&bitmap_lib,sizeof(bitmap_lib)); 

		bitmap_lib.close_raster = (rl_type_close_raster)close_bitmap;
		bitmap_lib.put_dot = (rl_type_put_dot)_bim_put_dot;
		bitmap_lib.get_dot = (rl_type_get_dot)_bim_get_dot;
		bitmap_lib.get_hseg = (rl_type_get_hseg)_bim_get_hseg;
		bitmap_lib.xor_rect = (rl_type_xor_rect)_bim_xor_rect;
		bitmap_lib.set_rast = (rl_type_set_rast)bim_set_rast;

		/* bitmap_lib.blitrect[RL_TO_SAME] */
		bitmap_lib.blitrect[RL_TO_BYTEMAP] = (rl_type_blitrect)_bim_blitrect;
		/* bitmap_lib.blitrect[RL_FROM_BYTEMAP] */
		bitmap_lib.blitrect[RL_TO_OTHER] = (rl_type_blitrect)_bim_blitrect;

		pj_set_grc_calls(&bitmap_lib);
		loaded = 1;
	}
	return(&bitmap_lib);
}

