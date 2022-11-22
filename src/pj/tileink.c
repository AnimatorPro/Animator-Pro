#include <string.h>
#define RASTCALL_INTERNALS
#include "jimk.h"
#include "bhash.h"
#include "errcodes.h"
#include "floatgfx.h"
#include "flicel.h"
#include "inkdot.h"
#include "inks.h"
#include "memory.h"
#include "options.h"
#include "rastlib.h"
#include "render.h"

/*********** Cel tile ink stuff *************************/

typedef struct celt_cache {
	USHORT width;
	USHORT height;
	SHORT y;
	SHORT ysplit;  /* if cel is bigger than screen */
	SHORT *ytable; /* height of screen */
	SHORT x;
	SHORT xsplit;  /* if cel is bigger than screen */
	SHORT *xtable; /* width of screen */
	void (*get_hseg)(Raster *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w);

	double sin; /* sin of rotation angle if rotated */
	double cos; /* cos of rotation angle if rotated */

#define SCSH 14 /* bit shift for fixed point transform */

	LONG isin; /* integer math sin<<SCSH */
	LONG icos; /* integer math cos<<SCSH */
	double dblang;

	/* stretch dot stuff */

	SHORT sdot_x;
	SHORT sdot_wid;
	SHORT xinvert;

	SHORT sdot_y;
	SHORT sdot_hgt;
	SHORT yinvert;

} Celt_cache;


static Celt_cache *ct_cache; /* cel tile ink cache */

static void celt_scale_hseg(Raster *cel, Pixel *pbuf, Coor x, Coor y, Ucoor w)
/* will handle shrunk 4x to real big limited by size of buffer */
{
Pixel cbuf[(SBSIZE*4)+1];
Pixel *pcbuf;
SHORT *pidx;
SHORT *maxidx;

	pidx = ct_cache->xtable + x;

#ifdef NOTYET  /* special case to handle cels bigger that the screen 
			  * I figure if you use tile ink this way you deserve slowness? */

	if(x > ct_cache->xsplit)
		pidx -= ct_cache->xsplit;
#endif

	maxidx = pidx + (w - 1);

	if((Coor)(w = *maxidx - *pidx) < 0)
	{
		w = -w;
		x = *maxidx;
	}
	else
		x = *pidx;
	++w;

	pcbuf = cbuf - x;
	pj__get_hseg(cel, cbuf, x, y, w);
	while(pidx <= maxidx)
		*pbuf++ = pcbuf[*pidx++];
}
static void get_celt_xy(Short_xy *pxy, SHORT x, SHORT y)
{
	if((pxy->x = (x - ct_cache->x) % (Coor)ct_cache->width) < 0)
		pxy->x += ct_cache->width;
	if((pxy->y = (y - ct_cache->y) % (Coor)ct_cache->height) < 0)
		pxy->y += ct_cache->height;
}

static Errcode init_celt_cache(Ink *inky)
{
int cache_size;
SHORT cwidth;
SHORT cheight;
Short_xy start;
Short_xy end;
Srect cr;
SHORT *table;

/* struct cload cl; */

	/* enough cache for twice the size of the screen because rotation of
	 * the cel can exceed this by some but not twice */

	cache_size = sizeof(Celt_cache) + ((vb.pencel->width + vb.pencel->height)
											* (sizeof(SHORT)*2));

	if((ct_cache = pj_malloc(cache_size)) == NULL) 
		return(Err_no_memory);

	clear_mem(ct_cache,sizeof(Celt_cache));
	table = (SHORT *)(ct_cache + 1);

	/* get size from stretch and cel root size */

	fcel_stretchsize(thecel, &cr);

	cwidth = thecel->rc->width; 
	cheight = thecel->rc->height; 
	ct_cache->width = intabs(cr.width);
	ct_cache->height = intabs(cr.height);
	ct_cache->x = cr.x;
	ct_cache->y = cr.y;

	get_celt_xy(&start, 0, 0); /* get upper left */
	get_celt_xy(&end, vb.pencel->width, vb.pencel->height); /* lower right */

	if(thecel->cd.stretch.x == 0)
	{
		ct_cache->get_hseg = pj__get_hseg;
	}
	else
	{
		if(cr.width < 0)
		{
			ct_cache->sdot_wid = -cr.width + 2;
			ct_cache->xinvert = (thecel->cd.cent.x<<1) - 1;
		}
		else
		{
			ct_cache->xinvert = 0;
			ct_cache->sdot_wid = cr.width;
		}	
		ct_cache->sdot_x = thecel->cd.cent.x + (ct_cache->sdot_wid>>1);

		if(ct_cache->width < (vb.pencel->width*2))
		{
			ct_cache->xtable = table;
			pj_make_scale_table(cwidth,cr.width,table);
			table += ct_cache->width;
			if(cwidth < (ct_cache->width*4))
				ct_cache->get_hseg = celt_scale_hseg;
			else
				ct_cache->get_hseg = NULL;
		}
		else /* cel is too big */
		{
			ct_cache->get_hseg = NULL;
		}
	}

	if(thecel->cd.stretch.y != 0)
	{
		if(cr.height < 0)
		{
			ct_cache->sdot_hgt = -cr.height + 2;
			ct_cache->yinvert = (thecel->cd.cent.y<<1) - 1;
		}
		else
		{
			ct_cache->yinvert = 0;
			ct_cache->sdot_hgt = cr.height;
		}	
		ct_cache->sdot_y = thecel->cd.cent.y + (ct_cache->sdot_hgt>>1);

		if(ct_cache->height < (vb.pencel->height*2))
		{
			ct_cache->ytable = table;
			pj_make_scale_table(cheight,cr.height,table);
		}
		else
		{
			ct_cache->get_hseg = NULL;
		}
	}

	if(thecel->cd.rotang.z != 0) 
	{
		ct_cache->dblang = itheta_tofloat(-thecel->cd.rotang.z,FCEL_TWOPI);
		ct_cache->sin = sin(ct_cache->dblang);
		ct_cache->cos = cos(ct_cache->dblang);
		ct_cache->isin = ((double)ct_cache->sin * (double)(1<<SCSH));
		ct_cache->icos = ((double)ct_cache->cos * (double)(1<<SCSH));
		ct_cache->get_hseg = NULL;
	}

	if(inky)
		inky->hline = celt_hline;
	return(Success);
}
Errcode init_celt_ink(Ink *inky)
{
	cleanup_celt_ink(inky);
	if(thecel == NULL)
		return(Err_abort);
	if(thecel->cfit)
		make_render_cfit(thecel->rc->cmap,thecel->cfit,thecel->cd.tcolor);
	if(inky->dither)
		make_ink_bhash(inky);
	return(init_celt_cache(inky));
}
void cleanup_celt_ink(Ink *inky)
{
	free_ink_bhash(inky);
	pj_freez(&ct_cache);
	inky->hline = gink_hline;
}

static Pixel celt_get_dot(const SHORT x, const SHORT y)

/* returns color of dot and x,y relative to cels raster of dot given dot on
 * screen */
{
Rcel *cel;
Short_xy dot;
SHORT celx;
SHORT cely;

	cel = thecel->rc;
	dot.x = x;
	dot.y = y;

	if(thecel->cd.rotang.z)
	{
		if (dot.x != thecel->cd.cent.x || dot.y != thecel->cd.cent.y)
		{
#ifdef TOOSLOW
			/* float */
			{
			double dblx,dbly;
			double fx,fy;

				dblx = dot.x - thecel->cd.cent.x;
				dbly = dot.y - thecel->cd.cent.y;

				fx = ct_cache->cos*dblx - ct_cache->sin*dbly;
				fy = ct_cache->sin*dblx + ct_cache->cos*dbly;

				dot.x = (SHORT)(fx + (fx<0?-0.5:+0.5)) + thecel->cd.cent.x;
				dot.y = (SHORT)(fy + (fy<0?-0.5:+0.5)) + thecel->cd.cent.y;
			}
#endif
			/* integer fixed point */
			{
			LONG transx;
			LONG transy;
			LONG fx, fy;

				transx = dot.x - thecel->cd.cent.x;
				transy = dot.y - thecel->cd.cent.y;

				fx = ((ct_cache->icos*transx) - (ct_cache->isin*transy));
				fy = ((ct_cache->isin*transx) + (ct_cache->icos*transy));

				dot.x = ((LONG)(fx+((fx<0?-1:1)<<(SCSH-1)))>>SCSH)
							+ thecel->cd.cent.x;

				dot.y = ((LONG)(fy+((fy<0?-1:1)<<(SCSH-1)))>>SCSH)
							+ thecel->cd.cent.y;
			}
		}
	}

	if(ct_cache->xtable) /* you know it stretched */
	{
		if((celx = (dot.x - ct_cache->x) % (Coor)ct_cache->width) < 0)
			celx += ct_cache->width;
		celx = ct_cache->xtable[celx];
	}
	else if(thecel->cd.stretch.x)
	{
		if(ct_cache->xinvert)
			dot.x = ct_cache->xinvert - dot.x;
		if((celx = (dot.x - ct_cache->sdot_x) % ct_cache->sdot_wid) < 0)
			celx += ct_cache->sdot_wid;
		celx =(SHORT)(((long)celx*(long)cel->width)/(long)ct_cache->sdot_wid);
	}
	else
	{
		if((celx = (dot.x - cel->x) % cel->width) < 0)
			celx += cel->width;
	}

	if(ct_cache->ytable) /* you know it stretched */
	{
		if((cely = (dot.y - ct_cache->y) % (Coor)ct_cache->height) < 0)
			cely += ct_cache->height;
		cely = ct_cache->ytable[cely];
	}
	else if(thecel->cd.stretch.y)
	{
		if(ct_cache->yinvert)
			dot.y = ct_cache->yinvert - dot.y;
		if((cely = (dot.y - ct_cache->sdot_y) % ct_cache->sdot_hgt) < 0)
			cely += ct_cache->sdot_hgt;
		cely=(SHORT)(((long)cely*(long)cel->height)/(long)ct_cache->sdot_hgt);
	}
	else
	{
		if((cely = (dot.y - cel->y) % cel->height) < 0)
			cely += cel->height;
	}
	return(GET_DOT(thecel->rc,celx,cely));
}

Pixel celt_color(SHORT x, SHORT y)
/* called in cel menu for getting tcolor color */
{
Pixel color;

	if(init_celt_cache(NULL) < Success)
		return(0);
	color = celt_get_dot(x,y);
	pj_freez(&ct_cache);
	return(color);
}

Pixel celt_dot(const Ink *inky, SHORT x, SHORT y)
{
Pixel color, undoc;
Celcfit *cfit;

	if(thecel == NULL)
		return(vs.ccolor);

	if(vs.render_under)
	{
		if((undoc = GET_DOT(undof,x,y)) != vs.inks[0])
			return(undoc);
		color = celt_get_dot(x,y);
	}
	else 
	{
		color = celt_get_dot(x,y);
		if( vs.zero_clear && (color == thecel->cd.tcolor))
			return(GET_DOT(undof,x,y));
	}

	if(inky->dither)
		return(bclosest_col(&(thecel->rc->cmap->ctab[color]),COLORS,TRUE));

	if( NULL != (cfit = thecel->cfit)
		&& !(cfit->flags & CCFIT_NULL))
	{
		return(cfit->ctable[color]);
	}
	return(color);
}

void celt_hline(const Ink *inky, SHORT x0, SHORT y, SHORT width)
{
UBYTE sbuf[SBSIZE];
UBYTE dbuf[SBSIZE];
UBYTE *spt;
UBYTE *dpt;
Rgb3 *ctab;
SHORT celleft, w, x;
Rcel *cel;
Procline pline;
Tcolxldat tcxl;
Short_xy cxy;

	if(thecel == NULL)
	{
		opq_hline(inky,x0,y,width);
		return;
	}

	cel = thecel->rc;

	if(ct_cache->get_hseg == NULL)
	{
		/* we must get them a dot at a time */

		spt = sbuf;
		x = x0;

		if( thecel->cd.rotang.z != 0
			|| width <= ct_cache->width )
		{
			dpt = sbuf + width;
			while(spt < dpt)
				*spt++ = celt_get_dot(x++, y);
		}
		else /* try tiling it */
		{
			dpt = sbuf + ct_cache->width;
			while(spt < dpt)
				*spt++ = celt_get_dot(x++, y);
			w = width;
			for(;;) /* tile what we got */
			{
				if((w -= ct_cache->width) < ct_cache->width)
				{
					if(w > 0)
						memcpy(spt,spt - ct_cache->width,w);
					break;
				}
				else
					memcpy(spt,spt - ct_cache->width,ct_cache->width);
				spt += ct_cache->width;
			}
		}
	}
	else
	{
		/* get initial cel position from screen position */

		get_celt_xy(&cxy, x0, y);
		if(ct_cache->ytable)
			cxy.y = ct_cache->ytable[cxy.y];

		celleft = ct_cache->width - cxy.x;
		if (celleft >= width)	/* hline is all inside cel */
		{
			ct_cache->get_hseg((Raster *)cel, sbuf, cxy.x, cxy.y, width);
		}
		else  /* have to get hline in pieces */
		{
			ct_cache->get_hseg((Raster *)cel, sbuf, cxy.x, cxy.y, celleft);
			w = width - celleft;
			spt = sbuf + celleft;

			for (;;)
			{
				if(w <= ct_cache->width)
				{
					ct_cache->get_hseg((Raster *)cel, spt, 0, cxy.y, w);
					break;
				}
				else
				{
					ct_cache->get_hseg((Raster *)cel, spt, 0, cxy.y,
							ct_cache->width);
					spt += ct_cache->width;
					w -= ct_cache->width;
				}
			}
		}
	}

	tcxl.tcolor = thecel->cd.tcolor;

	if(!vs.render_one_color && inky->dither)
	{
		spt = sbuf;
		dpt = dbuf;
		w = width;

		if(vs.fit_colors)
			ctab = cel->cmap->ctab;
		else
			ctab = vb.pencel->cmap->ctab;

		if(vs.zero_clear || vs.render_under)
		{
			pj__get_hseg((Raster *)undof, dpt, x0, y, w);
			--spt;
			--dpt;
			while (--w >= 0)
			{
				++spt;
				++dpt;
				if(vs.render_under)
				{
					if(*dpt != vs.inks[0])
						continue;
				}
				else if(vs.zero_clear && *spt == tcxl.tcolor)
					continue;
				*dpt = bclosest_col(ctab+*spt,COLORS,TRUE);
			}
		}
		else
		{
			while (--w >= 0)
				*dpt++ = bclosest_col(ctab+*spt++,COLORS,TRUE);
		}
	}
	else
	{
		tcxl.xlat = NULL;
		if(thecel->cfit && !(thecel->cfit->flags & CCFIT_NULL))
			tcxl.xlat = thecel->cfit->ctable;

		pline = get_celprocline(tcxl.xlat != NULL);

		if(pline)
		{
			if(vs.render_under)
				tcxl.tcolor = vs.inks[0];
			pj__get_hseg((Raster *)undof, dbuf, x0, y, width);
			(*pline)(sbuf,dbuf,width,&tcxl);
		}
		else
		{
			if(tcxl.xlat != NULL)
				pj_xlate(tcxl.xlat,sbuf,width);
			pj__put_hseg((Raster *)vb.pencel, sbuf, x0, y, width);
			return;
		}
	}
	pj__put_hseg((Raster *)vb.pencel, dbuf, x0, y, width);
}
