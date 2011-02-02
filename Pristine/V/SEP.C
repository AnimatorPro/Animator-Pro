
/* sep.c - stuff to implement the color separation functions. Also the
   edge tool from the looks of it. */

#include "jimk.h"

static PLANEPTR ctable, abs_ctable;
static int ccount;


/* This is sqr_root(3*64*64) */
#define RGB_MAX_DIF	111

static UBYTE sep_rgb_dest[3];

static UBYTE rgb_p2[2][3];

static
rgb_close_enough(c1, c2, threshold)
PLANEPTR c1, c2;
int threshold;
{
return( sqr_root((long)color_dif(c1,c2)) <= threshold);
}

edge1(ecolor)
int ecolor;
{
int i,j;
UBYTE *linep, *p, *lp, *np;

render_full_screen();
if (make_render_cashes())
	{
	linep = uf.p+1;
	for (j=1; j<YMAX-1; j++)
		{
		lp = linep;
		p = linep+BPR;
		np = linep+2*BPR;
		for (i=1; i<XMAX-1; i++)
			{
			if (p[0] == ecolor)
				{
				if (p[-1] != ecolor || p[1] != ecolor || lp[0] != ecolor
					|| np[0] != ecolor)
					render_dot(i,j);
				}
			lp++;
			p++;
			np++;
			}
		linep = norm_pointer(linep+BPR);
		}
	free_render_cashes();
	}
}

#define SEP_SINGLE 0
#define SEP_RGB_THRESH 1
#define SEP_CLUSTER 2
#define SEP_2PART 3

static
sep1()
{
int i,j;
PLANEPTR absc, pt;
int rgb_ccount;
int threshold;
int occolor;

if (vs.sep_type == SEP_RGB_THRESH)
	{
	threshold = vs.sep_threshold * RGB_MAX_DIF/100;
	occolor = vs.ccolor;
	vs.ccolor = closestc(sep_rgb_dest,render_form->cmap, COLORS);
	rgb_ccount = 0;
	absc = abs_ctable;
	i = ccount;
	while (--i >= 0)
		{
		pt = render_form->cmap;
		for (j=0; j<COLORS; j++)
			{
			if (rgb_close_enough(pt, absc, threshold) )
				{
				if (!in_ctable(j, ctable, rgb_ccount) )
					ctable[rgb_ccount++] = j;
				}
			pt += 3;
			}
		absc += 3;
		}
	render_separate(ctable,rgb_ccount, x_0, y_0,  x_1, y_1);
	vs.ccolor = occolor;
	}
#ifdef SLUFFED
else if (vs.sep_type == SEP_2PART)
	{
	int c1, c2;
	int clix;
	int cl[2];
	int i;
	int part;
	int occolor;
	UBYTE *cmap;
	int sepcount;

	occolor = vs.ccolor;
	cl[0] = closestc(rgb_p2[0],render_form->cmap, COLORS);
	cl[1] = closestc(rgb_p2[1],render_form->cmap, COLORS);
	for (part=0; part<=1; part++)
		{
		cmap = render_form->cmap;
		sepcount = 0;
		for (i=0; i<COLORS; i++)
			{
			if (closestc(cmap, rgb_p2, 2) == part)
				{
				ctable[sepcount++] = i;
				}
			cmap += 3;
			}
		vs.ccolor = cl[part];
		render_separate(ctable,sepcount, x_0, y_0,  x_1, y_1);
		}
	vs.ccolor = occolor;
	}
#endif SLUFFED
else if (vs.sep_type == SEP_SINGLE || vs.sep_type == SEP_CLUSTER)
	{
	render_separate(ctable,ccount, x_0, y_0,  x_1, y_1);
	}
return(1);
}

static get_sep_box()
{
if (vs.sep_box)
	{
	if (!rub_box())
		return(0);
	swap_box();
	if (!rub_in_place(x_0,y_0,x_1,y_1))
		return(0);
	}
else
	{
	x_0 = y_0 = 0;
	x_1 = XMAX-1;
	y_1 = YMAX-1;
	}
}

static
gather_ctable()
/* Called at button-state PJSTDN */
{
int color;
PLANEPTR s, d;
int i;

if ((ctable = begmem(COLORS)) == NULL)
	return(0);
save_undo();
ccount = 0;
get_sep_box();
if (vs.sep_type  == SEP_CLUSTER)	/* range */
	{
	ccount = cluster_count();
	copy_bytes(cluster_bundle(), ctable, ccount);
	return(1);
	}
#ifdef SLUFFED
else if (vs.sep_type == SEP_2PART)
	{
	int c1, c2;

	c1 = dwgetdot(grid_x, grid_y);
	copy_bytes(render_form->cmap+3*c1, rgb_p2[0], 3);
	wait_click();
	if (!PJSTDN)
		goto ERROUT;
	c2 = dwgetdot(grid_x, grid_y);
	copy_bytes(render_form->cmap+3*c2, rgb_p2[1], 3);
	return(1);
	}
#endif SLUFFED
else if (vs.sep_type == SEP_SINGLE || vs.sep_type == SEP_RGB_THRESH)
	{
	for (;;)	/* gather up a table while pen down of colors under cursor */
		{
		color = dwgetdot(grid_x,grid_y);
		if (!in_ctable(color, ctable, ccount) )
			ctable[ccount++] = color;
		wait_input();
		if (!PDN)
			{
			if (vs.sep_type == SEP_RGB_THRESH)
				{
				/* copy the absolute rgb values of ctable somewhere */
				copy_bytes(render_form->cmap+3*vs.ccolor, sep_rgb_dest, 3);
				if ((abs_ctable = begmem(ccount*3)) == NULL)
					goto ERROUT;
				s = ctable;
				d = abs_ctable;
				i = ccount;
				while (--i >= 0)
					{
					copy_bytes(render_form->cmap+3* *s++,d,3);
					d+= 3;
					}
				}
			return(1);
			}
		}
	}
ERROUT:
	{
	free_ctable();
	return(0);
	}
}

static
free_ctable()
{
gentle_freemem(ctable);
ctable = NULL;
if (vs.sep_type == SEP_RGB_THRESH)
	{
	gentle_freemem(abs_ctable);
	abs_ctable = NULL;
	}
}

separate()
{
wait_click();
if (PJSTDN)
	{
	if (gather_ctable())
		{
		unzoom();
		doauto(sep1);
		rezoom();
		}
	free_ctable();
	}
}

sep_tool()
{
if (!pti_input())
	return;
if (gather_ctable())
	{
	sep1();
	}
free_ctable();
}

