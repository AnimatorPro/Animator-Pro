
/* thikline. - one of the more horrifying modules at the time of writing.
   The idea is simply to draw a line wider than one pixel thick.  I used
   to do this by 'dragging a brush', but this gets less and less efficient
   as the brush size gets larger.  Instead we make up a 4 sided polygon 
   and polyfill it.  Figuring out exactly the values for these endpoints is
   the fun part.  I never could get the arithmetic to round correctly in
   some of the smaller cases (especially 4 and 8), so I'm using a table
   instead (ptps[]).  You figure out what angle the line is and use it as
   an index into this table to get the offsets of 2 points in the 4-gon
   from a vertex.  After that it's not so bad... */

#include <stdio.h>
#include "jimk.h"
#include "brush.h"
#include "fpmath.h"
#include "gfx.h"
#include "inkdot.h"
#include "inks.h"
#include "memory.h"
#include "poly.h"
#include "ptrmacro.h"
#include "render.h"

#ifdef SLUFFED

#define PDIV 8		/* How many sides in circular brush */
#define PDMK (PDIV-1)
#define PINC (TWOPI/PDIV)

static Short_xy ptp1[] = {
{0,0,}, {0,0,}, {0,0,}, {-1,0,}, 
{-1,0,}, {-1,-1,}, {0,-1,}, {0,-1,}, 
};

static Short_xy ptp2[] = {
{1,0,}, {1,0,}, {0,1,}, {0,1,}, 
{-1,0,}, {-1,0,}, {0,-1,}, {0,-1,}, 
};

/* A trouble case */
static Short_xy ptp3[] = {
{1,0,}, {0,1,}, {-1,1,}, {-2,0,}, 
{-2,-1,}, {-1,-2,}, {0,-2,}, {1,-1,}, 
};

static Short_xy ptp4[] = {
{2,0,}, {1,1,}, {0,2,}, {-1,1,}, 
{-2,0,}, {-1,-1,}, {0,-2,}, {1,-1,}, 
};

static Short_xy ptp5[] = {
{2,0,}, {1,1,}, {0,2,}, {-2,1,}, 
{-3,0,}, {-2,-2,}, {0,-3,}, {1,-2,}, 
};

static Short_xy ptp6[] = {
{3,0,}, {2,2,}, {0,3,}, {-2,2,}, 
{-3,0,}, {-2,-2,}, {0,-3,}, {2,-2,}, 
};

/* trouble case */
static Short_xy ptp7[] = {
{3,0,}, {2,1,}, {0,3,}, {-2,2,}, 
{-4,0,}, {-3,-2,}, {0,-4,}, {1,-3,}, 
};

static Short_xy ptp8[] = {
{4,0,}, {3,3,}, {0,4,}, {-3,3,}, 
{-4,0,}, {-3,-3,}, {0,-4,}, {3,-3,}, 
};

/* too thin in diagonals originally */
static Short_xy ptp9[] = {
{4,0,}, {3,2,}, {0,4,}, {-3,3,}, 
{-5,0,}, {-4,-3,}, {0,-5,}, {2,-4,}, 
};

/* too thick in diagonals originally */
static Short_xy ptp10[] = {
{5,0,}, {3,4,}, {0,5,}, {-4,3,}, 
{-5,0,}, {-3,-4,}, {0,-5,}, {4,-3,}, 
};

static Short_xy *ptps[] = {
NULL,ptp1,ptp2,ptp3,
ptp4,ptp5,ptp6,ptp7,
ptp8,ptp9,ptp10,
};

#endif /* SLUFFED */

/********* fat line from brush *********/

static LLpoint blp[4] =
	{
		{blp+1,0,0,0,},
		{blp+2,0,0,0,},
		{blp+3,0,0,0,},
		{blp+0,0,0,0,},
	};

static Poly bl_poly =
	{
	4, blp, 0, POLYMAGIC,
	};

typedef LONG fpoint;

#define FSHIFT 8
#define FVAL(a)	((long)(a)<<FSHIFT)
#define FINT(a) ((int)((a)>>FSHIFT))
#define FADD(a,b) ((a)+(b))
#define FSUB(a,b) ((a)-(b))
#define FMUL(a,b) (((a)*(b))>>FSHIFT)
#define FDIV(a,b) (((a<<FSHIFT)+b/2)/b)
#define MAXFP (((ULONG)~0)>>1)
#define MINFP (~MAXFP)

#ifdef SLUFFED 
static fpoint fdiv(fpoint a,fpoint b)
{
	return(((a<<FSHIFT)+b/2)/b);
}
#endif /* SLUFFED */

static Errcode brush_line(Rbrush *rb, Short_xy *ends)

/* renders a line using the current brush */
{
#define x0 ends[0].x
#define y0 ends[0].y
#define x1 ends[1].x
#define y1 ends[1].y

Errcode err;
SHORT dx, dy;
Short_xy min;
Short_xy max;

	disable_lsp_ink();
	if(rb->type == LINE_BRUSH)
	{
		min = rb->b.line.endoff;

		blp[0].x = x0 + min.x;	
		blp[0].y = y0 + min.y;
		
		blp[1].x = x0 - min.x;	
		blp[1].y = y0 - min.y;

		blp[2].x = x1 - min.x;	
		blp[2].y = y1 - min.y;

		blp[3].x = x1 + min.x;	
		blp[3].y = y1 + min.y;

		err = (filled_polygon(&bl_poly,render_hline,
							  vb.pencel, poly_cline_with_render_dot, NULL));
		goto done;
	}

	dx = x1 - x0;
	dy = y1 - y0;

	if(dx == 0)
	{
		if(dy == 0)
		{
			render_brush(x0,y0);
			err = Success;
			goto done;
		}
		blp[1].x = blp[0].x = x0 - rb->cent.x;
		blp[3].y = blp[0].y = y0;
		blp[3].x = blp[2].x = blp[0].x + rb->width - 1;
		blp[1].y = blp[2].y = y1;
	}
	else if(dy == 0)
	{
		blp[1].y = blp[0].y = y0 - rb->cent.y;
		blp[3].x = blp[0].x = x0;
		blp[3].y = blp[2].y = blp[0].y + rb->height - 1;
		blp[1].x = blp[2].x = x1;
	}
	else /* check to find two dots farthest away from line */
	{
	int bx,by; /* brush_x, brush_y */
	int numsame;
	int lastx;
	fpoint fpx, fpy;
	fpoint m, m_inv;
	fpoint dcur, dmin, dmax;
	#define bmap ((Bytemap *)rb->rast)
	Pixel *line;

		/* slope and inverse slope (inverse so no division in loop and
		 * no worrys about it's being 0) */

		m = FDIV(FVAL(dx),FVAL(dy));
		m_inv = FDIV(FVAL(dy),FVAL(dx));

	  /* make sure whole brush is on one side of line for test 
	   * line effectively goes through 0,0 if slope is posititive line
	   * will go through brush cel, so we moce brush down below line
	   * Note: I'm thinking cartesian and the screen is inverted but math
	   * works upside down */

		if(m > 0)
	  		fpy = FVAL(-rb->height);
	  	else
	  		fpy = FVAL(0);

		dmin = MAXFP;
		dmax = MINFP;
		lastx = rb->width - 1;
		line = bmap->bm.bp[0]; 

	  	for(by = 0;by < rb->height;++by)
	  	{
			/* find first non zero dot on hline */

			if(*line)
				bx = 0;
			else
			{
				if((bx = pj_bsame(line,rb->width)) == rb->width)
					goto nexty;
			}

			for(;;)
			{
				/* check dot against line getting two legs of right triangle
				 * that goes through dot and intersects line with same slope
				 * that goes through 0,0 since all triangles are similar
				 * we only need check the magnitude of the sum, not the
				 * sum of the squares (the square root of which is the actual
				 * distance from the brush dot to the line.
				 * we find min and max brush dot.  These are the points from
				 * which we draw the poly from one brush to the next */

				fpx = FVAL(bx);

				/* get absolute value of dx from line */
				if((dcur = FSUB(fpy,FMUL(fpx,m_inv))) < 0)
					dcur = -dcur;	

				/* get absolute value of dy from line in fpx as temp */
				if((fpx = FSUB(FMUL(fpy,m),fpx)) < 0)
					fpx = -fpx;

				/* sum them, save dot values of min and max distance dots */

				if((dcur = FADD(dcur,fpx)) < dmin)
				{
					dmin = dcur;
					min.x = bx;
					min.y = by;
				}
				if(dcur > dmax)
				{
					dmax = dcur;
					max.x = bx;
					max.y = by;
				}

				if(bx == lastx) /* if last on line done keep goin' */
					break;

				/* if we have THE first and last dot get next line */
				if(0 == (numsame = pj_bsame(line + bx,rb->width - bx)-1))
					break; 
				bx += numsame;
			}

		nexty:

			fpy = FADD(fpy,FVAL(1));
			line += bmap->bm.bpr;
	  	}


		/* load poly points */
		min.x = rb->cent.x - min.x;
		min.y = rb->cent.y - min.y;
		max.x = rb->cent.x - max.x;
		max.y = rb->cent.y - max.y;

		blp[0].x = x0 - min.x;	
		blp[0].y = y0 - min.y;
		
		blp[1].x = x0 - max.x;	
		blp[1].y = y0 - max.y;

		blp[2].x = x1 - max.x;	
		blp[2].y = y1 - max.y;

		blp[3].x = x1 - min.x;	
		blp[3].y = y1 - min.y;

	#undef bmap
	}


#ifdef FLOATMATH
	else /* check to find two dots farthest away from line */
	{
	int bx,by,y; /* brush_x, brush_y, test_y */
	double m;
	double dcur;
	double dmin;
	double dmax;

	  m = (double)dx/(double)dy; 
	  dmin = +100000000;
	  dmax = -100000000;

	  /* make sure whole brush is on one side of line for test */

	  if(m > 0)
	  	y = -rb->height;
	  else
	  	y = 0;

	  for(by = 0;by < rb->height;++by,++y)
	  {
		for(bx = 0;bx < rb->width;++bx)
		{
			if(!pj_get_dot(rb->rast,bx,by))
				continue;

			dcur = fabs(y - bx/m) + fabs(y*m - bx);

			if(dcur < dmin)
			{
				dmin = dcur;
				min.x = bx;
				min.y = by;
			}
			if(dcur > dmax)
			{
				dmax = dcur;
				max.x = bx;
				max.y = by;
			}
		}
	  }

		/* load poly points */
		min.x = rb->cent.x - min.x;
		min.y = rb->cent.y - min.y;
		max.x = rb->cent.x - max.x;
		max.y = rb->cent.y - max.y;

		blp[0].x = x0 - min.x;	
		blp[0].y = y0 - min.y;
		
		blp[1].x = x0 - max.x;	
		blp[1].y = y0 - max.y;

		blp[2].x = x1 - max.x;	
		blp[2].y = y1 - max.y;

		blp[3].x = x1 - min.x;	
		blp[3].y = y1 - min.y;
	}
#endif

	/* fudge for cycle color drawing */
	if(vs.cycle_draw && vl.ink->needs & INK_NEEDS_COLOR)
		render_brush(x0,y0);
	if((err = filled_polygon(&bl_poly,render_hline,vb.pencel,
							 poly_cline_with_render_dot, NULL)) >= Success)
	{
		render_brush(x1,y1);
	}
done:
	enable_lsp_ink();
	return(err);
}

Errcode render_line(SHORT x,SHORT y,SHORT xx,SHORT yy)

/* renders a line using the current brush */
{
Short_xy ends[2];

	if(!vs.use_brush)
	{
		pj_cline(x, y, xx, yy, render_dot, NULL);
		return(Success);
	}
	ends[0].x = x;
	ends[0].y = y;
	ends[1].x = xx;
	ends[1].y = yy;
	return(brush_line(vl.brush,ends));
}
Errcode render_1_line(SHORT x,SHORT y,SHORT xx,SHORT yy)
{
	if(!(vs.cycle_draw && vl.ink->needs & INK_NEEDS_COLOR))
		render_brush(x,y);
	return(render_line(x,y,xx,yy));
}
