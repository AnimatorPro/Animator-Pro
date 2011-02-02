
#include "jimk.h"
#include "render.h"
#include "imath.h"


void make_cfit_table(Rgb3 *scm,Rgb3 *dcm,Pixel *cnums, int clearc)
{
int i;
UBYTE c;

	for (i=0; i<COLORS; i++)
	{
		if (i == clearc) /* clear color always maps to itself! */
			*cnums++ = i;
		else
		{
			c = closestc(scm, dcm, COLORS);
			/* if clear color make sure you get another one */
			if (c == clearc)
				c = closestc_excl(scm, dcm, COLORS, &c, 1);
			*cnums++ = c;
		}
		++scm;
	}
}
void fitting_ctable(Rgb3 *scm,Rgb3 *dcm,UBYTE *cnums)
{
int clearc;

	if(vs.zero_clear)
		clearc = vs.inks[0];
	else
		clearc = -1;
	make_cfit_table(scm,dcm,(Pixel *)cnums, clearc);
}
void nz_fitting_ctable(Rgb3 *scm,Rgb3 *dcm,UBYTE *cnums)
{
	make_cfit_table(scm,dcm,(Pixel *)cnums, -1);
}

Boolean need_render_cfit(Cmap *scmap)
{
	return(vs.render_one_color
			|| (vs.fit_colors && !cmaps_same(scmap, vb.pencel->cmap)));
}
void init_celcfit(Celcfit *cfit)
{
	cfit->ccolor = -1;
	cfit->tcolor = -1;
	cfit->ink0 = -1;
	cfit->flags = 0;
	cfit->dst_cksum = (ULONG)(~0);
	cfit->src_cksum = 0;
}
Boolean make_simple_cfit(Cmap *scmap, Cmap *dcmap, Celcfit *cfit,int clearc)

/* clearc of -1 will ignore clearc */
{
ULONG cmap_cksum;
ULONG dcmap_cksum;

	if(cmaps_same(scmap,dcmap))
		return(0);

	dcmap_cksum = cmap_crcsum(dcmap);
	cmap_cksum = cmap_crcsum(scmap);
	cfit->ccolor = -1;

	if(    (cmap_cksum != cfit->src_cksum)
		|| (dcmap_cksum != cfit->dst_cksum)
		|| (cfit->ink0 != clearc))
	{
		make_cfit_table(scmap->ctab,vb.pencel->cmap->ctab,cfit->ctable,clearc);
		cfit->src_cksum = cmap_cksum;
		cfit->dst_cksum = dcmap_cksum;
		cfit->ink0 = -1;
	}
	return(TRUE);
}

void make_one_color_ctable(Pixel *ctable, SHORT tcolor)
{
pj_stuff_bytes(vs.ccolor,ctable,COLORS);
ctable[tcolor] = tcolor;
if(vs.render_under && vs.zero_clear)
	ctable[tcolor] = vs.inks[0];
}


Boolean make_render_cfit(Cmap *scmap, Celcfit *cfit, SHORT tcolor)
{
ULONG cmap_cksum;
ULONG dcmap_cksum;

	if(vs.render_one_color)
	{
		if(   (vs.ccolor != cfit->ccolor)
		   || (tcolor != cfit->tcolor)
		   || (cfit->ink0 != vs.inks[0]))
		{
			pj_stuff_bytes(vs.ccolor,cfit->ctable,COLORS);
			cfit->ctable[tcolor] = tcolor;
			if(vs.render_under && vs.zero_clear)
				cfit->ctable[tcolor] = vs.inks[0];
			cfit->ccolor = vs.ccolor;
			cfit->tcolor = tcolor;
			cfit->ink0 = vs.inks[0];
		}
		/* invalidate render cfit */
		cfit->dst_cksum = (ULONG)(~0);
		cfit->src_cksum = 0;
	}
	else if(need_render_cfit(scmap))
	{
		dcmap_cksum = cmap_crcsum(vb.pencel->cmap);
		cmap_cksum = cmap_crcsum(scmap);

		if(    (cmap_cksum != cfit->src_cksum)
			|| (dcmap_cksum != cfit->dst_cksum)
			|| (cfit->ink0 != vs.inks[0]))
		{
			make_cfit_table(scmap->ctab,vb.pencel->cmap->ctab,cfit->ctable,-1);
			cfit->ctable[tcolor] = tcolor;
			if(vs.render_under && vs.zero_clear)
				cfit->ctable[tcolor] = vs.inks[0];
			cfit->src_cksum = cmap_cksum;
			cfit->dst_cksum = dcmap_cksum;
			cfit->ink0 = vs.inks[0];
		}
		/* invalidate one color cfit */
		cfit->ccolor = -1;
	}
	else if(vs.render_under
			&& vs.zero_clear
			&& vs.inks[0] != tcolor)
	{
	int i;

		/* a translate is better than having to check other places against
		 * another tcolor too */

		for(i = 0;i < COLORS;++i)
			cfit->ctable[i] = i;
		cfit->ctable[tcolor] = vs.inks[0];

		/* invalidate other cfits */
		cfit->ink0 = -1;
		goto done;
	}
	else
	{
		cfit->flags |= CCFIT_NULL;
		return(FALSE);
	}

done:
	cfit->flags &= ~CCFIT_NULL;
	return(TRUE);
}

void get_cmap_blend(int bscale, Cmap *cmapa, Cmap *cmapb, Cmap *dcmap)
{
int count;
int as;
Rgb3 *a, *b, *d;

	count = Min(cmapa->num_colors,cmapb->num_colors);
	as = SCALE_ONE - bscale;

	a = cmapa->ctab + (count-=1);
	b = cmapb->ctab + count;
	d = dcmap->ctab + count;

	while(count-- >= 0)
	{
		d->r = ((((int)a->r)*as+((int)b->r)*bscale)+SCALE_ONE/2)/SCALE_ONE;
		d->g = ((((int)a->g)*as+((int)b->g)*bscale)+SCALE_ONE/2)/SCALE_ONE;
		d->b = ((((int)a->b)*as+((int)b->b)*bscale)+SCALE_ONE/2)/SCALE_ONE;
		--a;
		--d;
		--b;
	}
}
