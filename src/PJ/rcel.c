#include "jimk.h"
#include "errcodes.h"
#include "inks.h"
#include "marqi.h"
#include "memory.h"
#include "rastcall.h"
#include "rcel.h"
#include "zoom.h"

void cfit_rcel(Rcel *c, Cmap *dcmap)
/* Find closest colors in this color map to cel's color map, and then
   remap cel's pixels appropriately. */
{
UBYTE ctable[COLORS];

	fitting_ctable(c->cmap->ctab, dcmap->ctab, ctable);
	xlat_rast(c, ctable, 1);
}
void refit_rcel(Rcel *c, Cmap *ncmap, Cmap *ocmap)
{
UBYTE cvtab[COLORS];

	nz_fitting_ctable(ocmap->ctab, ncmap->ctab, cvtab);
	xlat_rast(c,cvtab,1);
}
void see_a_cel(register Rcel *cl)
/* Plop down a cel on screen */
{
Tcolxldat xld;

	xld.tcolor = vs.inks[0];

	(*(get_celblit(0)))(cl, 0, 0, vb.pencel, cl->x, cl->y, 
					   cl->width, cl->height,&xld, undof);
	zoom_cel(cl);
}
void unsee_a_cel(Rcel *c)
/* Erase cel image (presuming undo screen's been saved). */
{
	zoom_undo_rect(c->x,c->y,c->width,c->height);
}
void set_one_val(Rcel *rc, UBYTE clearc, UBYTE destc)
/* map everything but clearc to destc */
{
UBYTE table[COLORS];

	pj_stuff_words((destc<<8)+destc, table, COLORS/sizeof(SHORT));
	table[clearc] = clearc;
	xlat_rast(rc,table,1);
}
void show_cel_a_sec(Rcel *cel)
{
Marqihdr mh;

	if (cel != NULL)
	{
		vinit_marqihdr(&mh,1,1);
		save_undo();
		see_a_cel(cel);
		mh.smod = 8;
		while (--mh.smod >= 0)
		{
			mh.dmod = mh.smod;
			marqi_rect(&mh,(Rectangle *)&(cel->RECTSTART));
			wait_a_jiffy(4);
		}
		unsee_a_cel(cel);
	}
}
void zoom_cel(Rcel *c)
{
	rect_zoom_it(c->x, c->y, c->width, c->height);
}
Errcode clip_celrect(Rcel *src, Rectangle *rect, Rcel **clip)

/* this will clip to source bounds, returns Err_clipped if clipped out
 * in which case there is no cel */
{
Rcel clipcel;

	*clip = NULL;
	if(!pj_rcel_make_virtual(&clipcel,src,rect))
		return(Err_clipped);
	if((*clip = clone_rcel(&clipcel)) == NULL)
		return(Err_no_memory);
	return(0);
}
static void delta_move_rcel(Rcel *c, SHORT dx, SHORT dy,
	Tcolxldat *txl, Boolean fit_cel)
/* Move an rcel while minimizing horrible screen flashing.  */
{
SHORT ox, oy;

	ox = c->x;
	oy = c->y;
	c->x = ox+dx;
	c->y = oy+dy;
	do_leftbehind(ox,oy,c->x,c->y,c->width,c->height
	,	(do_leftbehind_func)undo_rect);
	(*(get_celmove(fit_cel)))(c,0,0,vb.pencel, c->x, c->y, c->width, c->height,
		    		  txl, undof);
	if(vs.zoom_open) /* a few nanoseconds here ... */
	{
		do_leftbehind(ox,oy,c->x,c->y,c->width,c->height
		,	(do_leftbehind_func)rect_zoom_it);
		zoom_cel(c);
	}
}

Errcode move_rcel(Rcel *rc, Boolean fit_cel, Boolean one_color)
/* moves and rcel over the vb.pencel using the undo buffer to refresh,
 * undo must be saved before this is called */
{
Errcode err;
SHORT lx, ly, firstx, firsty;
Tcolxldat xld;
Pixel fitab[256];
Boolean need_remap = fit_cel || one_color;

	xld.tcolor = vs.inks[0];
	if (fit_cel)
		{
		fitting_ctable(rc->cmap->ctab, vb.pencel->cmap->ctab, fitab);
		xld.xlat = fitab;
		}
	else if (one_color)
		{
		make_one_color_ctable(fitab, xld.tcolor);
		xld.xlat = fitab;
		}
	else
		{
		xld.xlat = NULL;
		}
	firstx = rc->x;
	firsty = rc->y;

	(*(get_celblit(need_remap)))(rc, 0, 0, vb.pencel, rc->x, rc->y, 
					   rc->width, rc->height,&xld, undof);
	zoom_cel(rc);
	if((err = rub_rect_in_place((Rectangle *)(&rc->RECTSTART))) < 0)
		goto out;

	for (;;)
	{
		box_coors(rc->x, rc->y, firstx, firsty);
		lx = icb.mx;
		ly = icb.my;
		wait_input(MMOVE|ANY_CLICK);

		if(JSTHIT(MMOVE))
			delta_move_rcel(rc, icb.mx-lx, icb.my-ly, &xld, need_remap);

		if(JSTHIT(ANY_CLICK))
			break;
	}

	if(JSTHIT(KEYHIT|MBRIGHT))
		err = Err_abort;
	else
		err = 0;

out:
	cleanup_toptext();
	unsee_a_cel(rc);
	if(err)
	{
		rc->x = firstx;
		rc->y = firsty;
	}
	return(err);
}
/********************************************************/
/* blitfuncs for cel blitting and moving returned by get_celblit and 
 * get_celmove() */

static void celblit(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* blits a rectangle fron source to dest */
{
	(void)xld;
	pj_blitrect(src,sx,sy,dest,dx,dy,w,h);
}
static void celblitxl(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* xlate blits a rectangle from source to dest */
{
	xlatblit(src,sx,sy,dest,dx,dy,w,h,xld->xlat);
}
static void celtblit(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* "T" blits a rectangle fron source to dest */
{
	pj_tblitrect(src,sx,sy,dest,dx,dy,w,h,xld->tcolor);
}
static void celtblitxl(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* xlate "T" blits a rectangle from source to dest */
{
	procblit(src,sx,sy,dest,dx,dy,w,h,tbli_xlatline,xld);
}
static void celublit(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* "U" blits a rectangle from source to dest */
{
	ublitrect(src,sx,sy,dest,dx,dy,w,h,xld->tcolor);
}
static void celublitxl(Raster *src,SHORT sx,SHORT sy,
					  Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					  Tcolxldat *xld,... )

/* xlate "U" blits a rectangle from source to dest */
{
	procblit(src,sx,sy,dest,dx,dy,w,h,ubli_xlatline,xld);
}
static void celabtblit(Raster *src,SHORT sx,SHORT sy,
					 Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					 Tcolxldat *xld, Raster *src_b )
{
	abprocblit(src,sx,sy,dest,dx,dy,w,h,src_b,dx,dy,pj_tbli_line,xld);
}
static void celabtxlblit(Raster *src,SHORT sx,SHORT sy,
					 Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					 Tcolxldat *xld, Raster *src_b )
{
	abprocblit(src,sx,sy,dest,dx,dy,w,h,src_b,dx,dy,tbli_xlatline,xld);
}
static void celabublit(Raster *src,SHORT sx,SHORT sy,
					 Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					 Tcolxldat *xld, Raster *src_b )
{
	abprocblit(src,sx,sy,dest,dx,dy,w,h,src_b,dx,dy,ubli_line,xld);
}
static void celabuxlblit(Raster *src,SHORT sx,SHORT sy,
					 Raster *dest,SHORT dx,SHORT dy,SHORT w,SHORT h,
					 Tcolxldat *xld, Raster *src_b )
{
	abprocblit(src,sx,sy,dest,dx,dy,w,h,src_b,dx,dy,ubli_xlatline,xld);
}
Celblit get_celmove(Boolean cfit)
{
	if(vs.render_under)
	{
		if(cfit)
			return((Celblit)celabuxlblit);
		return((Celblit)celabublit);
	}
	if(vs.zero_clear)
	{
		if(cfit)
			return((Celblit)celabtxlblit);
		return((Celblit)celabtblit);
	}
	if(cfit)
		return((Celblit)celblitxl);
	return(celblit);
}
Celblit get_celblit(Boolean cfit)
{
	if(vs.render_under)
	{
		if(cfit)
			return((Celblit)celublitxl);
		return(celublit);
	}
	if(vs.zero_clear)
	{
		if(cfit)
			return((Celblit)celtblitxl);
		return(celtblit);
	}
	if(cfit)
		return((Celblit)celblitxl);
	return(celblit);
}
Procline get_celprocline(Boolean cfit)
{
	if(vs.render_under)
	{
		if(cfit)
			return(ubli_xlatline);
		return(ubli_line);
	}
	if(vs.zero_clear)
	{
		if(cfit)
			return(tbli_xlatline);
		return(pj_tbli_line);
	}
	return(NULL); /* straight blit no processing required (may need cfit) */
}


