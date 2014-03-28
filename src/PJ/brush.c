/* brush.c:  code for handling drawing brushes */

#include "jimk.h"
#include "brush.h"
#include "errcodes.h"
#include "floatgfx.h"
#include "fpmath.h"
#include "inks.h"
#include "memory.h"
#include "menus.h"
#include "pentools.h"

#ifdef SLUFFED

#define BITS8(a,b,c,d,e,f,g,h)\
	((a<<7)|(b<<6)|(c<<5)|(d<<4)|(e<<3)|(f<<2)|(g<<1)|(h))

#define BITS16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)\
	(BITS8(a,b,c,d,e,f,g,h)),(BITS8(i,j,k,l,m,n,o,p))

#define _ 0
#define X 1

/* some static data statements for 16x16 pixel mask brushes */

static UBYTE box1_mask[] = {
		BITS8(X,X,_,_,_,_,_,_),
		BITS8(X,X,_,_,_,_,_,_),
		};

static UBYTE circ1_mask[] = {
		BITS8(_,X,_,_,_,_,_,_),
		BITS8(X,X,X,_,_,_,_,_),
		BITS8(_,X,_,_,_,_,_,_),
		};

static UBYTE circ2_mask[] = {
		BITS8(_,X,X,_,_,_,_,_),
		BITS8(X,X,X,X,_,_,_,_),
		BITS8(X,X,X,X,_,_,_,_),
		BITS8(_,X,X,_,_,_,_,_),
		};

static UBYTE circ3_mask[] = {
		BITS8(_,_,X,_,_,_,_,_),
		BITS8(_,X,X,X,_,_,_,_),
		BITS8(X,X,X,X,X,_,_,_),
		BITS8(_,X,X,X,_,_,_,_),
		BITS8(_,_,X,_,_,_,_,_),
		};

static UBYTE circ4_mask[] = {
		BITS8(_,_,X,X,_,_,_,_),
		BITS8(_,X,X,X,X,_,_,_),
		BITS8(X,X,X,X,X,X,_,_),
		BITS8(X,X,X,X,X,X,_,_),
		BITS8(_,X,X,X,X,_,_,_),
		BITS8(_,_,X,X,_,_,_,_),
		};

static UBYTE circ5_mask[] = {
		BITS8(_,_,X,X,X,_,_,_),
		BITS8(_,X,X,X,X,X,_,_),
		BITS8(X,X,X,X,X,X,X,_),
		BITS8(X,X,X,X,X,X,X,_),
		BITS8(X,X,X,X,X,X,X,_),
		BITS8(_,X,X,X,X,X,_,_),
		BITS8(_,_,X,X,X,_,_,_),
	};

static UBYTE circ6_mask[] = {
		BITS8(_,_,_,X,X,_,_,_),
		BITS8(_,X,X,X,X,X,X,_),
		BITS8(_,X,X,X,X,X,X,_),
		BITS8(X,X,X,X,X,X,X,X),
		BITS8(X,X,X,X,X,X,X,X),
		BITS8(_,X,X,X,X,X,X,_),
		BITS8(_,X,X,X,X,X,X,_),
		BITS8(_,_,_,X,X,_,_,_),
	};

static UBYTE circ7_mask[] = {
		BITS16(_,_,_,X,X,X,_,_,_,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,_,_,X,X,X,_,_,_,_,_,_,_,_,_,_),
	};

static UBYTE circ8_mask[] = {
		BITS16(_,_,_,_,X,X,_,_,_,_,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,_,_,_,X,X,_,_,_,_,_,_,_,_,_,_),
	};

static UBYTE circ9_mask[] =  {
		BITS16(_,_,_,_,X,X,X,_,_,_,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,_,_,_,X,X,X,_,_,_,_,_,_,_,_,_),
	};

static UBYTE circ10_mask[] =  {
		BITS16(_,_,_,_,X,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,_,_,X,X,X,X,_,_,_,_,_,_,_,_),
	};

static UBYTE circ11_mask[] =  {
		BITS16(_,_,_,_,_,X,X,X,_,_,_,_,_,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,_,_,_,X,X,X,_,_,_,_,_,_,_,_),
	};

static UBYTE circ12_mask[] =  {
		BITS16(_,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,X,_,_,_,_,_),
		BITS16(_,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_),
	};

static UBYTE circ13_mask[] =  {
		BITS16(_,_,_,_,_,X,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,_,_,_),
		BITS16(_,_,_,X,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,_,_,_,X,X,X,X,X,_,_,_,_,_,_),
	};

static UBYTE circ14_mask[] =  {
		BITS16(_,_,_,_,_,_,X,X,X,X,_,_,_,_,_,_),
		BITS16(_,_,_,_,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X),
		BITS16(X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,_,X,X,X,X,X,X,X,X,X,X,X,X,_,_),
		BITS16(_,_,_,_,X,X,X,X,X,X,X,X,_,_,_,_),
		BITS16(_,_,_,_,_,_,X,X,X,X,_,_,_,_,_,_),
	};
#undef BITS8
#undef BITS16
#undef _
#undef X

Image dot_pens[] = {
	IMAGE_INIT1(ITYPE_BITPLANES,1,box1_mask,2,2),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ1_mask,3,3),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ2_mask,4,4),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ3_mask,5,5),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ4_mask,6,6),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ5_mask,7,7),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ6_mask,8,8),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ7_mask,9,9),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ8_mask,10,10),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ9_mask,11,11),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ10_mask,12,12),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ11_mask,13,13),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ12_mask,14,14),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ13_mask,15,15),
	IMAGE_INIT1(ITYPE_BITPLANES,1,circ14_mask,16,16),
};

#endif /* SLUFFED */


/* FUDGE for funny cursors store what is under brush */

static Rbrush thebrush;
static Bytemap *ubrush;

void save_ubrush(Rbrush *rb, void *src, Coor x, Coor y)
{
	pj_blitrect(src,
				ubrush->x = x - rb->cent.x,
				ubrush->y = y - rb->cent.y,
				ubrush,0,0,rb->width,rb->height);
}
void rest_ubrush(Rbrush *rb,void *dst)
{
	pj_blitrect(ubrush,0,0,dst,ubrush->x, ubrush->y, rb->width,rb->height);
}
/* end KLUDGE Y stuff */

void cleanup_brushes(void)
{
	pj_rast_free(thebrush.rast);
	pj_rast_free(ubrush);
	pj_freez(&thebrush.tcxl.xlat);
	clear_struct(&thebrush);
}
Errcode init_brushes(void)
{
Errcode err;

	if((err = valloc_bytemap((Raster **)&thebrush.rast, BRUSH_MAX_WIDTH,
							 BRUSH_MAX_HEIGHT)) < Success)
	{
		return(err);
	}
	if((err = valloc_bytemap((Raster **)&ubrush, BRUSH_MAX_WIDTH,
							 BRUSH_MAX_HEIGHT)) < Success)
	{
		return(err);
	}
	if((thebrush.tcxl.xlat = pj_malloc(COLORS * sizeof(Pixel))) == NULL)
		goto no_mem_error;

	pj_clear_rast(thebrush.rast);
	thebrush.type = NO_BRUSH;

	vl.brush = &thebrush;

	return(Success);
no_mem_error:
	err = Err_no_memory;
	cleanup_brushes();
	return(err);
}

static Errcode set_circle_brush(int size)
/* size of 0 is a dot and not a circle brush 1 is 2x2 square and on up */
{
Rbrush *rb = &thebrush;

	if(size < 0)
		size = 0;
	else if(size >= rb->rast->width)
		size = rb->rast->width - 1;

	if(rb->type == CIRCLE_BRUSH
		&& rb->b.circ.size == size)
	{
		return(Success);
	}

	++size;
	rb->cent.x = rb->cent.y = (size>>1);
	pj_set_rect(rb->rast,0,0,0,size,size); /* clear where it will be */
	circle(rb->rast,1,rb->cent.x,rb->cent.y,size,TRUE); /* make circle */
	rb->tcxl.xlat[0] = 0;
	rb->tcxl.tcolor = 0;
	rb->width = rb->height = size;
	rb->b.circ.size = --size;

	vs.pen_brush_type = CIRCLE_BRUSH;
	vs.circle_brush_size = size;
	rb->type = CIRCLE_BRUSH;
	return(Success);
}

static Errcode set_square_brush(int size)
/* size of 0 is a dot and 1 is 2x2 square and on up */
{
Rbrush *rb = &thebrush;

	if(size < 0)
		size = 0;
	else if(size >= rb->rast->width)
		size = rb->rast->width-1;

	if(rb->type == SQUARE_BRUSH
		&& rb->b.square.size == size)
	{
		return(Success);
	}

	++size;
	rb->cent.x = rb->cent.y = (size>>1);
	pj_set_rect(rb->rast,1,0,0,size,size); /* set square to color 1 */
	circle(rb->rast,1,rb->cent.x,rb->cent.y,size,TRUE); /* make circle */
	rb->tcxl.xlat[0] = 0;
	rb->tcxl.tcolor = 0;
	rb->width = rb->height = size;
	rb->b.square.size = --size;

	rb->type = vs.pen_brush_type = SQUARE_BRUSH;
	vs.square_brush_size = size;
	return(Success);
}
static void get_line_end(Short_xy *pt,int size, int angle)
/* since the final max is Max/2 + 1 we must make sure it doesn't get too big
 * This is why we clip the size to 1 less than the max. MAX in this case is
 * an even number */
{
Short_xy point;

	point.x = 0;
	point.y = size;

	if(size)
	{
		frotate_points2d(to_radians(angle),NULL,&point,&point,1);

		point.x >>= 1;
		point.y >>= 1;

		if(point.x >= (BRUSH_MAX_WIDTH/2))
			point.x = (BRUSH_MAX_WIDTH/2) - 1;
		else if(point.x <= -(BRUSH_MAX_WIDTH/2))
			point.x = -((BRUSH_MAX_WIDTH/2) - 1);

		if(point.y >= (BRUSH_MAX_HEIGHT/2))
			point.y = (BRUSH_MAX_HEIGHT/2) - 1;
		else if(point.y <= -(BRUSH_MAX_HEIGHT/2))
			point.y = -((BRUSH_MAX_HEIGHT/2) - 1);
	}
	*pt = point;
}
void draw_line_brush(void *rast,Short_xy *cent, Pixel color,
					 int size, int angle)
{
Short_xy pt;

	get_line_end(&pt,size,angle);

	line(rast, color, cent->x - pt.x, cent->y - pt.y,
		  cent->x + pt.x, cent->y + pt.y );
}

static Errcode set_line_brush(int size, int degrees)
/* size of 0 is a dot and 1 is 2 dots on up */
{
Rbrush *rb = &thebrush;
Short_xy pt;

	if(size < 0)
		size = 0;
	else if(size > rb->rast->width)
		size = rb->rast->width;

	if(rb->type == LINE_BRUSH
		&& rb->b.line.size == size
		&& rb->b.line.angle == degrees)
	{
		return(Success);
	}

	get_line_end(&pt,size,degrees);

	rb->cent.x = Absval(pt.x);
	rb->cent.y = Absval(pt.y);

	rb->width = (rb->cent.x * 2) + 1;
	rb->height = (rb->cent.y * 2) + 1;

	pj_set_rect(rb->rast,0,0,0,rb->width,rb->height);

	line(rb->rast, 1, rb->cent.x - pt.x, rb->cent.y - pt.y,
					  rb->cent.x + pt.x, rb->cent.y + pt.y );

	rb->b.line.endoff = pt;
	vs.pen_brush_type = rb->type = LINE_BRUSH;
	vs.line_brush_size = rb->b.line.size = size;
	vs.line_brush_angle = rb->b.line.angle = degrees;

	return(Success);
}

Errcode set_brush_type(int type)
{
	switch(type)
	{
		default:
		case CIRCLE_BRUSH:
			return(set_circle_brush(vs.circle_brush_size));
		case SQUARE_BRUSH:
			return(set_square_brush(vs.square_brush_size));
		case LINE_BRUSH:
			return(set_line_brush(vs.line_brush_size,vs.line_brush_angle));
	}
}
int get_brush_size(void)
{
	switch(vs.pen_brush_type)
	{
		case CIRCLE_BRUSH:
			return(vs.circle_brush_size);
		case SQUARE_BRUSH:
			return(vs.square_brush_size);
		case LINE_BRUSH:
			return(vs.line_brush_size);
		default:
			return(0);
	}
}
void set_brush_size(int size)
{
	switch(vs.pen_brush_type)
	{
		case CIRCLE_BRUSH:
			vs.circle_brush_size = size;
			break;
		case SQUARE_BRUSH:
			vs.square_brush_size = size;
			break;
		case LINE_BRUSH:
			vs.line_brush_size = size;
			break;
		default:
			break;
	}
	set_brush_type(vs.pen_brush_type);
}
static void check_rbrush_xlat(Rbrush *rb)
{
	if(rb->xlat_ccolor != vs.ccolor)
	{
		if(rb->type == CIRCLE_BRUSH)
			rb->tcxl.xlat[1] = vs.ccolor;
		else
			pj_stuff_bytes(vs.ccolor, &rb->tcxl.xlat[1],COLORS-1);
		rb->xlat_ccolor = vs.ccolor;
	}
}
void blit_brush(Rbrush *rb, void *dest, Coor x, Coor y)
{
	check_rbrush_xlat(rb);
	procblit(rb->rast,0,0,dest,x - rb->cent.x,y - rb->cent.y,
			 rb->width, rb->height,tbli_xlatline,&rb->tcxl);
}
void zoom_blit_brush(Rbrush *rb,Coor x, Coor y)
{
	check_rbrush_xlat(rb);
	zoom_txlatblit(rb->rast,0,0, rb->width, rb->height,
				   x - rb->cent.x,y - rb->cent.y, &rb->tcxl);
}
void save_undo_brush(SHORT y)
{
	if(vs.use_brush)
		save_lines_undo(y-vl.brush->cent.y, vl.brush->height);
	else
		save_line_undo(y);
}
void see_pen(Button *b)
{
Pixel occolor;
Clipbox cb;

	mb_make_clip(b,&cb);

	white_block(b);
	if(!vs.use_brush)
	{
		pj_put_dot(&cb,mc_grey(b),(b->width>>1),(b->height>>1));
		return;
	}

	occolor = vs.ccolor;
	vs.ccolor = mc_grey(b);
	blit_brush(vl.brush, &cb,(b->width>>1),(b->height>>1));
	vs.ccolor = occolor;

	if(vl.brush->width >= b->width
		|| vl.brush->height >= b->height)
	{
		mc_frame(b,MC_BLACK);
	}
}
