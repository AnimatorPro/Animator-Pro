/* sep.c - stuff to implement the color separation functions. Also the
   edge tool from the looks of it. */

#include "errcodes.h"
#include "jimk.h"
#include "auto.h"
#include "memory.h"
#include "redo.h"

/* separate control block. */
typedef struct sep_cb
	{
	Sep_p p;
	Rgb3 *abs_ctable;
	Rgb3 sep_rgb_dest;
	} Sep_cb;

void free_ctable();


int in_cnums(UBYTE color, UBYTE *table, int count)
/* returns 0 if not found otherwise ix+1 of color in table */
{
	table += count;
	while(count > 0)
	{
		if(color == *(--table))
			break;
		--count;
	}
	return(count);
}


static int rgb_close_enough(Rgb3 *c1,Rgb3 *c2,int threshold)
{
	return( sqr_root((long)color_dif(c1,c2)) <= threshold);
}


static Errcode sep1(Sep_cb *sep)
{
Errcode err;
int i,j;
Rgb3 *absc, *pt;
UBYTE *rgb_cnums;
int rgb_ccount;
int threshold;
int occolor;
int cscale;

	if (vs.sep_rgb == 1)	/* case NEAR */
	{
		cscale = sqr_root((long)RGB_MAX*RGB_MAX*3)+1;
		threshold = vs.sep_threshold * cscale/100;
		if ((rgb_cnums = pj_malloc(COLORS)) == NULL)
			return(Err_no_memory);
		occolor = vs.ccolor;
		vs.ccolor = closestc(&sep->sep_rgb_dest,vb.pencel->cmap->ctab, COLORS);
		rgb_ccount = 0;
		absc = sep->abs_ctable;
		i = sep->p.ccount;
		while (--i >= 0)
		{
			pt = vb.pencel->cmap->ctab;
			for (j=0; j<COLORS; j++)
			{
				if (rgb_close_enough(pt, absc, threshold) )
				{
				if (!in_cnums(j, rgb_cnums, rgb_ccount))
					rgb_cnums[rgb_ccount++] = j;
				}
				++pt;
			}
			++absc;
		}
		err = render_separate(rgb_cnums,rgb_ccount,&sep->p.rect);
		pj_free(rgb_cnums);
		vs.ccolor = occolor;
	}
	else
	{
		err = render_separate(sep->p.ctable,sep->p.ccount,&sep->p.rect);
	}
	if(vs.cycle_draw && err >= Success)
		cycle_ccolor();
	return(err);
}

static Errcode get_sep_source_colors(Sep_cb *sep)
{
int color;
Errcode err;
SHORT ogrid;

	if ((sep->p.ctable = pj_malloc(COLORS)) == NULL)
		return(Err_no_memory);
	save_undo();
	if (vs.sep_box)
	{
		if((err = get_rub_rect(&sep->p.rect)) < 0)
			goto error;
		if((err = rub_rect_in_place(&sep->p.rect)) < 0)
			goto error;
	}
	else
	{
		sep->p.rect.x = sep->p.rect.y = 0;
		sep->p.rect.width = vb.pencel->width;
		sep->p.rect.height = vb.pencel->height;
	}

	if (vs.sep_rgb	== 2)	/* range */
	{
		sep->p.ccount = cluster_count();
		pj_copy_bytes(cluster_bundle(), sep->p.ctable, sep->p.ccount);
		return(Success);
	}

	ogrid = vs.use_grid;
	vs.use_grid = FALSE;

	for (;;)	/* gather up a table while pen down of colors under cursor */
	{
		color = pj_get_dot(vb.pencel,icb.mx,icb.my);
		if (!in_cnums(color, sep->p.ctable, sep->p.ccount) )
			sep->p.ctable[sep->p.ccount++] = color;
		wait_any_input();
		if (!ISDOWN(MBPEN))
			break;
	}
	vs.use_grid = ogrid;
	err = Success;
error:
	return(err);
}

static Errcode get_abs_ctable(Sep_cb *sep)
{
UBYTE *s;
Rgb3 *d;
int i;

if (vs.sep_rgb == 1)
	{
	/* copy the absolute rgb values of cnums somewhere */
	get_color_rgb(vs.ccolor,vb.pencel->cmap,&sep->sep_rgb_dest);

	if ((sep->abs_ctable = pj_malloc(sep->p.ccount*3)) == NULL)
		return(Err_no_memory);

	s = sep->p.ctable;
	d = sep->abs_ctable;
	i = sep->p.ccount;
	while (--i >= 0)
		{
		get_color_rgb(*s++,vb.pencel->cmap,d);
		++d;
		}
	}
return(Success);
}

static Errcode gather_ctable(Sep_cb *sep)
{
Errcode err;

if ((err = get_sep_source_colors(sep)) < Success)
	return(err);
return(get_abs_ctable(sep));
}

static void free_ctable(Sep_cb *sep)
{
	pj_gentle_free(sep->p.ctable);
	pj_gentle_free(sep->abs_ctable);
}


Errcode do_sep_redo(Sep_p *sep)
{
Sep_cb scb;
Errcode err;

clear_struct(&scb);
scb.p = *sep;
if ((err = get_abs_ctable(&scb)) >= Success)
	{
	sep1(&scb);
	}
free_ctable(&scb);
return(err);
}

static UBYTE from_menu;
static Errcode do_sep_tool()
{
Errcode err;
Sep_cb sep;
USHORT flags;

	if (!pti_input())
		return(Success);
	clear_struct(&sep);
	if((err = gather_ctable(&sep)) >= 0)
	{
		if(from_menu)
		{
			if(vs.multi)
			{
				flags = AUTO_UNZOOM;
				if(vl.ink->needs & INK_NEEDS_CEL)
					flags |= AUTO_USESCEL;
			}
			else
				flags = 0;

			err = go_autodraw(sep1,&sep,flags);
		}
		else
		{
			sep1(&sep);
			err = save_redo_sep(&sep.p);
		}
	}
	free_ctable(&sep);
	return(err);
}
Errcode sep_tool(void)
{
	return(do_sep_tool());
}
void separate(void)
{
extern Pentool sep_ptool_opt;

	from_menu = TRUE;
	do_pentool_once(&sep_ptool_opt);
	from_menu = FALSE;
}

Errcode csd_edge1(Pixel ecolor)
{
Errcode err;
UBYTE *linebufs;
UBYTE *prevline, *line, *nextline;
UBYTE *pbyte, *lbyte, *nbyte;
UBYTE *swaper;
int linesize;
SHORT x, y;

	set_full_gradrect();
	linesize = undof->width + 2; /* one Pixel on each end to be an edge */
	if((linebufs = pj_malloc(linesize * 3)) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}

	prevline = linebufs;
	line = prevline + linesize;
	nextline = line + linesize;

	/* Fast enough, set them all to ecolor */
	pj_stuff_bytes(ecolor, linebufs, linesize * 3);
	pj__get_hseg(undof,&line[1],0,0,undof->width);

	y = -1;
	for(;;)
	{
		/* for Y, +2 and -- make a net +1 */

		if((y += 2) >= undof->height)
		{
			if(y > undof->height)
				break;
			/* last time set line to ecolor for one beyond screen */
			pj_stuff_bytes(ecolor,nextline,linesize);
		}
		else
			pj__get_hseg(undof,&nextline[1],0,y, undof->width);
		--y;

		/* note first pixel is one before screen edge */

		pbyte = prevline;
		lbyte = line;
		nbyte = nextline;

		for (x=0; x < undof->width; ++x)
		{
			++pbyte;
			++nbyte;
			++lbyte;
			if (lbyte[0] == ecolor)
			{
				if (  lbyte[-1] != ecolor || lbyte[1] != ecolor
					|| pbyte[0] != ecolor || nbyte[0] != ecolor)
				{
					render_brush(x,y);
				}
			}
		}
		swaper = prevline;
		prevline = line;
		line = nextline;
		nextline = swaper;
	}
	pj_free(linebufs);
error:
	return(err);
}

int edge1(Pixel ecolor)
{
Errcode err;

if((err = make_render_cashes()) < 0)
	return(err);
err = csd_edge1(ecolor);
free_render_cashes();
return(err);
}
