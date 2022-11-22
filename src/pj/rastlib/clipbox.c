#include "rastcall.ih"
#include "libdummy.h"
#include "memory.h"

/**************************************************************/

static void pj_cbox_put_dot(Raster *clipbox, Pixel c, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;

	if ((Ucoor)x >= cb->width || (Ucoor)y >= cb->height)
		return;
	PUT_DOT(cb->root,c,x+cb->x,y+cb->y);
}
static void pj__cbox_put_dot(Raster *clipbox, Pixel c, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;
	PUT_DOT(cb->root,c,x+cb->x,y+cb->y);
}
static Pixel pj_cbox_get_dot(Raster *clipbox, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;

	if ((Ucoor)x >= cb->width || (Ucoor)y >= cb->height)
		return(0);
	return(GET_DOT(cb->root,x+cb->x,y+cb->y));
}
static Pixel pj__cbox_get_dot(Raster *clipbox, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;
	return(GET_DOT(cb->root,x+cb->x,y+cb->y));
}

static void
pj__cbox_put_hseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Clipbox *cb = (Clipbox *)clipbox;
	PUT_HSEG(cb->root,pbuf,x+cb->x,y+cb->y,width);
}
static void
pj__cbox_get_hseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Clipbox *cb = (Clipbox *)clipbox;
	GET_HSEG(cb->root,pbuf,x+cb->x,y+cb->y,width);
}
static void
pj__cbox_put_vseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;
	PUT_VSEG(cb->root,pbuf,x+cb->x,y+cb->y,height);
}
static void
pj__cbox_get_vseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;
	GET_VSEG(cb->root,pbuf,x+cb->x,y+cb->y,height);
}
static void
pj__cbox_set_hline(Raster *clipbox, Pixel color, Coor x, Coor y, Ucoor width)
{
	Clipbox *cb = (Clipbox *)clipbox;
	SET_HLINE(cb->root,color,x+cb->x,y+cb->y,width);
}
static void
pj__cbox_set_vline(Raster *clipbox, Pixel color, Coor x, Coor y, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;
	SET_VLINE(cb->root,color,x+cb->x,y+cb->y,height);
}
static void pj__cbox_put_rectpix(Raster *clipbox, Pixel *pixbuf,
							   Coor x,Coor y,Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	(cb->root->lib->put_rectpix)(cb->root,
			pixbuf, x+cb->x, y+cb->y, width, height);
}
static void pj__cbox_get_rectpix(Raster *clipbox, Pixel *pixbuf,
							   Coor x,Coor y,Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	(cb->root->lib->get_rectpix)(cb->root,
			pixbuf, x+cb->x, y+cb->y, width, height);
}
static void
pj__cbox_set_rect(Raster *clipbox, Pixel color, Coor x, Coor y,
					Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;
	SET_RECT(cb->root,color,x+cb->x,y+cb->y,width,height);
}
static void pj__cbox_xor_rect(Raster *clipbox, Pixel color, Coor x, Coor y,
					Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;
	XOR_RECT(cb->root,color,x+cb->x,y+cb->y,width,height);
}

static void
pj__cbox_mask1blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
			   Raster *clipbox, Coor x, Coor y, Ucoor width, Ucoor height,
			   Pixel oncolor )
{
	Clipbox *cb = (Clipbox *)clipbox;

	MASK1BLIT(mbytes,mbpr,mx,my,cb->root,
			  x+cb->x,y+cb->y,width,height,oncolor);
}
static void
pj__cbox_mask2blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
			   Raster *clipbox, Coor x, Coor y, Ucoor width, Ucoor height,
			   Pixel oncolor, Pixel offcolor )
{
	Clipbox *cb = (Clipbox *)clipbox;

	MASK2BLIT(mbytes, mbpr,mx,my,cb->root,x+cb->x,y+cb->y,
			  width,height,oncolor,offcolor);
}

/* decompressors */

static void cbox_unbrun_rect(Raster *clipbox, void *ucbuf, LONG pixsize,
					  Coor x,Coor y,Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	(cb->root->lib->unbrun_rect)(cb->root,ucbuf,pixsize,
					x+cb->x,y+cb->y,
					width,height);
}
static void cbox_unlccomp_rect(Raster *clipbox, void *ucbuf, LONG pixsize,
					  Coor x,Coor y,Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	(cb->root->lib->unlccomp_rect)(cb->root,ucbuf,pixsize,
					x+cb->x,y+cb->y,
					width,height);
}
static Errcode cbox_unss2_rect(Raster *clipbox, void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return (cb->root->lib->unss2_rect)(cb->root,ucbuf,pixsize,
					x+cb->x,y+cb->y,
					width,height);
}

/***** binary calls *******/

static Errcode cbox_blitrect(Raster *clipbox,
			 Coor x, Coor y,
			 Raster *dest,
			 Coor dest_x, Coor dest_y,
			 Ucoor width, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return pj__blitrect(cb->root,x+cb->x,y+cb->y,
			 dest, dest_x, dest_y,width, height);
}
static Errcode cbox_f_blitrect(Raster *source,
			 Coor src_x, Coor src_y,
			 Raster *clipbox,
			 Coor x, Coor y,
			 Ucoor width, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return (cb->root->lib->blitrect[RL_FROM_BYTEMAP])
			(source, src_x, src_y,
			 cb->root, x+cb->x,y+cb->y, width, height);
}

static void cbox_swaprect(Raster *clipbox,
			  Coor x, Coor y,
			  Raster *rastb,
			  Coor rastb_x, Coor rastb_y,
			  Ucoor width, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	pj__swaprect(cb->root, x+cb->x,y+cb->y,
			  rastb, rastb_x, rastb_y, width, height);
}
static void cbox_f_swaprect(Raster *rasta,
			  Coor rasta_x, Coor rasta_y,
			  Raster *clipbox,
			  Coor x, Coor y,
			  Ucoor width, Ucoor height)
{
	Clipbox *cb = (Clipbox *)clipbox;

	(cb->root->lib->swaprect[RL_FROM_BYTEMAP])
			(rasta, rasta_x, rasta_y,
			 cb->root, x+cb->x,y+cb->y,
			 width, height);
}

static Errcode cbox_tblitrect(Raster *clipbox,
			 Coor x, Coor y,
			 Raster *dest,
			 Coor dest_x, Coor dest_y,
			 Ucoor width, Ucoor height, Pixel tcolor)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return pj__tblitrect(cb->root,x+cb->x,y+cb->y,
			  dest, dest_x, dest_y,width, height, tcolor);
}
static Errcode cbox_f_tblitrect(Raster *source,
			 Coor src_x, Coor src_y,
			 Raster *clipbox,
			 Coor x, Coor y,
			 Ucoor width, Ucoor height, Pixel tcolor)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return (cb->root->lib->tblitrect[RL_FROM_BYTEMAP])
			( source, src_x, src_y, cb->root, x+cb->x, y+cb->y,
			  width, height, tcolor);
}

static Errcode cbox_zoomblit(Raster *clipbox, Coor x, Coor y,
							 Raster *dst, Coor dx, Coor dy,
							 Ucoor dw, Ucoor dh, LONG zxs, LONG zys)
{
	Clipbox *cb = (Clipbox *)clipbox;

	return pj_zoomblit(cb->root, x+cb->x, y+cb->y,
			dst, dx, dy, dw, dh, zxs, zys);
}
static Errcode cbox_f_zoomblit(Raster *src, Coor sx, Coor sy,
							 Raster *clipbox, Coor x, Coor y,
							 Ucoor dw, Ucoor dh, LONG zxs, LONG zys)

/* this type is to handle the FROM_BYTEMAP case since the lib will be
 * called from the destination raster *****/
{
	Clipbox *cb = (Clipbox *)clipbox;

	return (cb->root->lib->zoomblit[RL_FROM_BYTEMAP])
		(src,sx,sy,cb->root,x+cb->x,y+cb->y,dw,dh,zxs,zys);
}

static void cbox_set_colors(Raster *clipbox, LONG start, LONG count, void *ctab)
{
	Clipbox *cb = (Clipbox *)clipbox;
	SET_COLORS(cb->root,start,count,ctab);
}
static void cbox_uncc64(Raster *clipbox, UBYTE *cbuf)
{
	Clipbox *cb = (Clipbox *)clipbox;
	UNCC64(cb->root,cbuf);
}
static void cbox_uncc256(Raster *clipbox, UBYTE *cbuf)
{
	Clipbox *cb = (Clipbox *)clipbox;
	UNCC256(cb->root,cbuf);
}
static void cbox_wait_vsync(Raster *clipbox)
{
	Clipbox *cb = (Clipbox *)clipbox;
	WAIT_VSYNC(cb->root);
}

/* these are in cboxlib.asm */

#ifdef USE_OPTIMISED_RASTLIB
extern void pj_sclip_put_hseg();
extern void sclip_get_hseg();
extern void pj_sclip_put_vseg();
extern void sclip_get_vseg();
extern void pj_sclip_put_rectpix();
extern void sclip_get_rectpix();
extern void pj_sclip_set_hline();
extern void pj_sclip_set_vline();
extern void pj_sclip_set_rect();
extern void pj_sclip_xor_rect();
extern void pj_sclip_blitrect();
extern void sclip_swaprect();
extern void sclip_tblitrect();
extern void sclip_xor_rast();
extern void sclip_zoomblit();
extern void pj_sclip_mask1blit();
extern void pj_sclip_mask2blit();
#endif /* USE_OPTIMISED_RASTLIB */

static void *get_cbox_lib(void)
/* Return pointer to clip box display function jump-table */
{
static Rastlib cbox_lib;
static int loaded = 0;

	if(!loaded)
	{
		/* Work around bug in 3D Studio REX loader where static
		 * data isn't set to zero. */
		clear_mem(&cbox_lib,sizeof(cbox_lib)); 

		cbox_lib.put_dot = pj__cbox_put_dot;
		cbox_lib.cput_dot = pj_cbox_put_dot;
		cbox_lib.get_dot = pj__cbox_get_dot;
		cbox_lib.cget_dot = pj_cbox_get_dot;
		cbox_lib.put_hseg = pj__cbox_put_hseg;
		cbox_lib.get_hseg = pj__cbox_get_hseg;
		cbox_lib.put_vseg = pj__cbox_put_vseg;
		cbox_lib.get_vseg = pj__cbox_get_vseg;

		cbox_lib.get_rectpix = pj__cbox_get_rectpix;
		cbox_lib.put_rectpix = pj__cbox_put_rectpix;

		cbox_lib.set_hline = pj__cbox_set_hline;
		cbox_lib.set_vline = pj__cbox_set_vline;
		cbox_lib.set_rect = pj__cbox_set_rect;

#ifdef NOT_NEEDED
		cbox_lib.set_rast = cbox_set_rast;
#endif /* NOT_NEEDED */
		cbox_lib.xor_rect = pj__cbox_xor_rect;
		cbox_lib.mask1blit = pj__cbox_mask1blit;
		cbox_lib.mask2blit = pj__cbox_mask2blit;

		cbox_lib.unbrun_rect = cbox_unbrun_rect;
		cbox_lib.unlccomp_rect = cbox_unlccomp_rect;
		cbox_lib.unss2_rect = cbox_unss2_rect;

		/* binary calls */

		cbox_lib.blitrect[RL_TO_SAME] = cbox_blitrect;
		cbox_lib.blitrect[RL_TO_BYTEMAP] = cbox_blitrect;
		cbox_lib.blitrect[RL_FROM_BYTEMAP] = cbox_f_blitrect;
		cbox_lib.blitrect[RL_TO_OTHER] = cbox_blitrect;

		cbox_lib.swaprect[RL_TO_SAME] = cbox_swaprect;
		cbox_lib.swaprect[RL_TO_BYTEMAP] = cbox_swaprect;
		cbox_lib.swaprect[RL_FROM_BYTEMAP] = cbox_f_swaprect;
		cbox_lib.swaprect[RL_TO_OTHER] = cbox_swaprect;

		cbox_lib.tblitrect[RL_TO_SAME] = cbox_tblitrect;
		cbox_lib.tblitrect[RL_TO_BYTEMAP] = cbox_tblitrect;
		cbox_lib.tblitrect[RL_FROM_BYTEMAP] = cbox_f_tblitrect;
		cbox_lib.tblitrect[RL_TO_OTHER] = cbox_tblitrect;

#ifdef NOT_MADE
		cbox_lib.xor_rast[RL_TO_SAME] = cbox_xor_rast;
		cbox_lib.xor_rast[RL_TO_BYTEMAP] = cbox_xor_rast;
		cbox_lib.xor_rast[RL_FROM_BYTEMAP] = cbox_f_xor_rast;
		cbox_lib.xor_rast[RL_TO_OTHER] = cbox_xor_rast;
#endif /* NOT_MADE */

		cbox_lib.zoomblit[RL_TO_SAME] = cbox_zoomblit;
		cbox_lib.zoomblit[RL_TO_BYTEMAP] = cbox_zoomblit;
		cbox_lib.zoomblit[RL_FROM_BYTEMAP] = cbox_f_zoomblit;
		cbox_lib.zoomblit[RL_TO_OTHER] = cbox_zoomblit;

		cbox_lib.set_colors = cbox_set_colors;
		cbox_lib.uncc64 = cbox_uncc64;
		cbox_lib.uncc256 = cbox_uncc256;
		cbox_lib.wait_vsync = cbox_wait_vsync;

		pj_set_grc_calls(&cbox_lib);
		loaded = 1;
	}
	return(&cbox_lib);
}
static void pj_sclip_put_dot(Raster *clipbox, Pixel c, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;
	pj_put_dot(cb->root, c, x + cb->x, y + cb->y);
}
static Pixel pj_sclip_get_dot(Raster *clipbox, Coor x, Coor y)
{
	Clipbox *cb = (Clipbox *)clipbox;
	return pj_get_dot(cb->root, x + cb->x, y + cb->y);
}

#ifdef CCODE
static void pj_sclip_put_hseg(Clipbox *cb,void *pbuf,Coor x,Coor y, Ucoor width)
{
	pj_put_hseg(cb->root,pbuf,x+cb->x,y+cb->y,width);
}
static void pj_sclip_set_hline(Clipbox *cb,Pixel c,Coor x,Coor y, Ucoor width)
{
	pj_set_hline(cb->root,c,x+cb->x,y+cb->y,width);
}
#endif /* CCODE */

static void
scbox_get_hseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
Clipbox *cb = (Clipbox *)clipbox;
Ucoor rwid;
Raster *root;

	root = cb->root;

	if(((Ucoor)(y += cb->y)) >= root->height)
		goto clear_seg;

	if((x += cb->x) < 0)
	{
		if (width < (Ucoor)(x = -x))
			goto clear_seg;
		pj_stuff_bytes(0,pbuf,x);
		pbuf = OPTR(pbuf,x);
		width -= x;
		x = 0;
	}
	if(((Coor)width) <= 0)
		return;

	if ((Ucoor)x < (rwid = root->width))
	{
		if(width < rwid)
			rwid = width;
		GET_HSEG(root, pbuf, x, y, rwid);
		if((width -= rwid) == 0)
			return;
		pbuf = OPTR(pbuf,rwid);
	}

clear_seg:
	pj_stuff_bytes(0,pbuf,width);
}
static void
scbox_get_vseg(Raster *clipbox, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
Clipbox *cb = (Clipbox *)clipbox;
Ucoor rht;
Raster *root;

	root = cb->root;

	if(((Ucoor)(x += cb->x)) >= root->width)
		goto clear_seg;

	if((y += cb->y) < 0)
	{
		if (height < (Ucoor)(y = -y))
			goto clear_seg;
		pj_stuff_bytes(0,pbuf,y);
		pbuf = OPTR(pbuf,y);
		height -= y;
		y = 0;
	}
	if(((Coor)height) <= 0)
		return;

	if ((Ucoor)y < (rht = root->height))
	{
		if(height < rht)
			rht = height;
		GET_VSEG(root, pbuf, x, y, rht);
		if((height -= rht) == 0)
			return;
		pbuf = OPTR(pbuf,rht);
	}

clear_seg:
	pj_stuff_bytes(0,pbuf,height);
}

static void *get_safecbox_lib(void)
/* Return pointer to clip box display function jump-table for clip boxes
 * that are larger than root raster, forces clipping around root raster
 * and will read zeros with get_dot() and get_hseg() outside root rast */
{
static Rastlib safe_cbox_lib;
static int loaded = 0;

/* the use of generics will force all pixels read from a clipbox outside
 * the root raster to read as zeros since the get_dot and get_hseg return
 * zeros and the generic reader routines use these. Forcing decompressors
 * through generics also will force them to go through clipped writing
 * routines */

	if(!loaded)
	{
		/* Work around bug in 3D Studio REX loader where static
		 * data isn't set to zero. */
		clear_mem(&safe_cbox_lib,sizeof(safe_cbox_lib)); 

		safe_cbox_lib.get_dot = pj_sclip_get_dot;
		safe_cbox_lib.cget_dot = pj_sclip_get_dot;
		safe_cbox_lib.put_dot = pj_sclip_put_dot;
		safe_cbox_lib.cput_dot = pj_sclip_put_dot;

		safe_cbox_lib.get_hseg = scbox_get_hseg; /* reads 0s */
		safe_cbox_lib.get_vseg = scbox_get_vseg; /* reads 0s */

#ifdef USE_OPTIMISED_RASTLIB

		safe_cbox_lib.put_hseg = (rl_type_put_hseg)pj_sclip_put_hseg;
		safe_cbox_lib.put_vseg = (rl_type_put_vseg)pj_sclip_put_vseg;

		safe_cbox_lib.get_rectpix = (rl_type_get_rectpix)sclip_get_rectpix;
		safe_cbox_lib.put_rectpix = (rl_type_put_rectpix)pj_sclip_put_rectpix;

		safe_cbox_lib.set_hline = (rl_type_set_hline)pj_sclip_set_hline;
		safe_cbox_lib.set_vline = (rl_type_set_vline)pj_sclip_set_vline;
		safe_cbox_lib.set_rect = (rl_type_set_rect)pj_sclip_set_rect;

/*		safe_cbox_lib.xor_rect = (rl_type_xor_rect)pj_sclip_xor_rect; */
		safe_cbox_lib.mask1blit = (rl_type_mask1blit)pj_sclip_mask1blit;
		safe_cbox_lib.mask2blit = (rl_type_mask2blit)pj_sclip_mask2blit;

		safe_cbox_lib.blitrect[RL_FROM_BYTEMAP] 
		=	(rl_type_blitrect)pj_sclip_blitrect;
		safe_cbox_lib.blitrect[RL_TO_SAME] 
		=	(rl_type_blitrect)pj_sclip_blitrect;
		safe_cbox_lib.blitrect[RL_TO_BYTEMAP] 
		=	(rl_type_blitrect)pj_sclip_blitrect;
		safe_cbox_lib.blitrect[RL_TO_OTHER] 
		=	(rl_type_blitrect)pj_sclip_blitrect;

		safe_cbox_lib.unbrun_rect = (rl_type_unbrun_rect)NOFUNC;
		safe_cbox_lib.unlccomp_rect = (rl_type_unlccomp_rect)NOFUNC;
		safe_cbox_lib.unss2_rect = (rl_type_unss2_rect)NOFUNC;

		safe_cbox_lib.swaprect[RL_TO_SAME] 
		=	(rl_type_swaprect)sclip_swaprect;
		safe_cbox_lib.swaprect[RL_TO_BYTEMAP] 
		=	(rl_type_swaprect)sclip_swaprect;
		safe_cbox_lib.swaprect[RL_FROM_BYTEMAP] 
		=	(rl_type_swaprect)sclip_swaprect;
		safe_cbox_lib.swaprect[RL_TO_OTHER] 
		=	(rl_type_swaprect)sclip_swaprect;

		safe_cbox_lib.tblitrect[RL_TO_SAME] 
		=	(rl_type_tblitrect)sclip_tblitrect;
		safe_cbox_lib.tblitrect[RL_TO_BYTEMAP] 
		=	(rl_type_tblitrect)sclip_tblitrect;
		safe_cbox_lib.tblitrect[RL_FROM_BYTEMAP] 
		=	(rl_type_tblitrect)sclip_tblitrect;
		safe_cbox_lib.tblitrect[RL_TO_OTHER] 
		=	(rl_type_tblitrect)sclip_tblitrect;

		safe_cbox_lib.xor_rast[RL_TO_SAME] = (rl_type_xor_rast)NOFUNC;
		safe_cbox_lib.xor_rast[RL_TO_BYTEMAP] = (rl_type_xor_rast)NOFUNC;
		safe_cbox_lib.xor_rast[RL_FROM_BYTEMAP] = (rl_type_xor_rast)NOFUNC;
		safe_cbox_lib.xor_rast[RL_TO_OTHER] = (rl_type_xor_rast)NOFUNC;

		safe_cbox_lib.zoomblit[RL_TO_SAME] 
		=	(rl_type_zoomblit)sclip_zoomblit;
		safe_cbox_lib.zoomblit[RL_TO_BYTEMAP] 
		=	(rl_type_zoomblit)sclip_zoomblit;
		safe_cbox_lib.zoomblit[RL_FROM_BYTEMAP] 
		=	(rl_type_zoomblit)sclip_zoomblit;
		safe_cbox_lib.zoomblit[RL_TO_OTHER] 
		=	(rl_type_zoomblit)sclip_zoomblit;

#endif /* USE_OPTIMISED_RASTLIB */

		safe_cbox_lib.set_colors = cbox_set_colors;
		safe_cbox_lib.uncc64 = cbox_uncc64;
		safe_cbox_lib.uncc256 = cbox_uncc256;
		safe_cbox_lib.wait_vsync = cbox_wait_vsync;

		pj_set_grc_calls(&safe_cbox_lib);
		loaded = 1;
	}
	return(&safe_cbox_lib);
}

Boolean pj_clipbox_make(Clipbox *cb, Raster *r,
					 Coor x,Coor y,Coor width,Coor height)

/* makes a clip box usable, returns 0 if clipped out 0 if some part of it
 * is on the raster puts a null lib in cbox if clipped out may be called
 * repeatedly for moving the box */
{
Boolean outside = FALSE;

	*((Rasthdr *)cb) = *((Rasthdr *)r);

	/* check if the rect overlaps the raster at least a little bit */

	if(x < 0)
	{
		if(width <= -x)
			goto clipout;
		outside = TRUE;
	}
	else if( x > r->width)
		goto clipout;

	if(y < 0)
	{
		if(height <= -y)
			goto clipout;
		outside = TRUE;
	}
	else if( y > r->height)
		goto clipout;

	/* if the clip rectangle extends outside the root raster we have to install
	 * the library to force clip for the root raster as well */

	if(outside
		|| r->width < (x + width)
		|| r->height < (y + height))
	{
		cb->lib = get_safecbox_lib();
	}
	else
	{
		cb->lib = get_cbox_lib();
	}

	cb->type = RT_CLIPBOX;
	cb->x = x;
	cb->y = y;
	cb->width = width;
	cb->height = height;
	cb->root = r;

	return(1);
clipout:
	cb->lib = pj_get_null_lib();
	return(0);
}

#ifdef NEVER
Boolean pj_clipbox_make(Clipbox *cb, Raster *r,
					 Coor x,Coor y,Coor width,Coor height)

/* makes a clip box usable, returns 0 if clipped out 0 if some part of it
 * is on the raster puts a null lib in cbox if clipped out may be called
 * repeatedly for moving the box */
{
Boolean outside = FALSE;

	*((Rasthdr *)cb) = *((Rasthdr *)r);

	/* check if the rect overlaps the raster at least a little bit */

	if(x < 0)
	{
		if(width <= -x)
			goto clipout;
		outside = TRUE;
		cb->left_border = -x;
	}
	else
	{
		if( x > r->width)
			goto clipout;
		cb->left_border = 0;
	}

	if(y < 0)
	{
		if(height <= -y)
			goto clipout;
		outside = TRUE;
		cb->top_border = -y;
	}
	else
	{
		if( y > r->height)
			goto clipout;
		cb->top_border = 0;
	}

	if((cb->right_border = (x + width) - r->width) < 0)
		cb->right_border = 0;

	if((cb->bot_border = (y + height) - r->height) < 0)
		cb->bot_border = 0;

	/* if the clip rectangle extends outside the root raster we have to install
	 * the library to force clip for the root raster as well */

	if(outside || cb->right_border || cb->bot_border)
	{
		cb->lib = get_safecbox_lib();
	}
	else
	{
		cb->lib = get_cbox_lib();
	}

	cb->type = RT_CLIPBOX;
	cb->x = x;
	cb->y = y;
	cb->width = width;
	cb->height = height;
	cb->root = r;

	return(1);
clipout:
	cb->lib = pj_get_null_lib();
	return(0);
}
#endif
