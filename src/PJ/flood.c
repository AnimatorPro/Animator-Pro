
/* Flood.c - a little flood fill routine.  Nearly portable! A horizontal
   segment/FIFO oriented flood fill.
  */

#include "ptrmacro.h"
#include "jimk.h"
#include "errcodes.h"
#include "inks.h"


#define FSZ 1024
#define FMOD 1023

struct fseg
{
	SHORT y;
	SHORT left, right;
};

typedef struct floodata {
	Rcel *testscr;			/* screen to build fseg list from */
	Bitmap vmask;              /* the visit"mask" */
	PLANEPTR visitbuf;          /* loaded with plane pointer from vmask */
	struct fseg *fsegments;
	int fread_pt, fwrite_pt, fcount;
	int floodto;
	int fhx0, fhx1, fhy0, fhy1;
	Pixel fcolor;
	UBYTE *linebuf;
} Floodata;

static Errcode some_flood(USHORT x, USHORT y, int floodto, Pixel fcolor);

static Errcode
flood_visit(Floodata *fd, EFUNC hout, USHORT x, USHORT y, USHORT color);

static void visit_hline(Floodata *fd, SHORT y, SHORT x1, SHORT x2)
{
set_bit_hline(fd->visitbuf, fd->vmask.bm.bpr, y, x1, x2);
}

static void expand_fseg(Floodata *fd, struct fseg *s)
/* expand_fseg() - given a horizontal line segment that needs to be
 * flooded,  search to the left and right to see if a wider horizontal
 * line segment can be flooded. */
{
register int x;
int f2 = fd->floodto;

	x = s->left;
	while (--x >= 0)
	{
		if ((pj_get_dot(fd->testscr,x,s->y) != fd->fcolor)^f2)
		{
			break;
		}
	}
	s->left = x+1;
	x = s->right;

	while(++x < fd->testscr->width)
	{
		if ((pj_get_dot(fd->testscr,x,s->y) != fd->fcolor)^f2)
			break;
	}
	s->right = x - 1;
}

static void add_fseg(register Floodata *fd, SHORT y, SHORT left, SHORT right)
{
register struct fseg *new;

	new = fd->fsegments + fd->fwrite_pt;
	fd->fwrite_pt++;
	fd->fwrite_pt &= FMOD;
	fd->fcount++;
	fd->fcount &= FMOD;
	new->y = y;
	new->left = left;
	new->right = right;
}


static void scan_seg(Floodata *fd, SHORT y, SHORT left, SHORT right)
{
register SHORT fillit;
SHORT ln;
register SHORT x;
int f2 = fd->floodto;
UBYTE *pbuf;
UBYTE fcolor;

	pbuf = fd->linebuf;
	x = left;
	fcolor = fd->fcolor;
	pj__get_hseg(fd->testscr, pbuf, x, y, right-x+1);
	fillit = 0;
	for (; x<=right; x++)
	{
		if ((*pbuf++ == fcolor)^f2)
		{
			if (!fillit)
			{
				fillit = 1;
				ln = x;
			}
		}
		else
		{
			if (fillit)
			{
				fillit = 0;
				add_fseg(fd, y, ln, x-1);
			}
		}
	}
	if (fillit)
		add_fseg(fd, y, ln, x-1);
}


static Errcode nofunc() /* always successful */ { return(Success); }

static Errcode flood_rhline(SHORT y, SHORT x0, SHORT x1, SHORT color)
{
	return(render_hline(y, x0, x1,vb.pencel));
}

Errcode fill(USHORT x, USHORT y)
{
	return(some_flood(x,y,0,pj_get_dot(undof,x,y)));
}

Errcode flood(USHORT x, USHORT y, Pixel endcolor)
{
	return(some_flood(x,y,1,endcolor));
}


Errcode csd_some_flood(USHORT x, USHORT y,int floodto, Pixel fcolor, Rcel *r)
{
Errcode err;
register Floodata *fd;

	if((fd = pj_malloc(PADSIZE(Floodata) + (FSZ*sizeof(struct fseg)))) == NULL)
		return(Err_no_memory);
	if ((fd->linebuf = pj_malloc(vb.pencel->width)) == NULL)
	{
		err = Err_no_memory;
		goto free_fd_out;
	}
	fd->fsegments = OPTR(fd,PADSIZE(Floodata));
	fd->testscr = r;
	fd->floodto = floodto;
	fd->fcolor = fcolor;

	copy_rasthdr(vb.pencel,&fd->vmask);
	fd->vmask.pdepth = 1;
	if((err = pj_open_bitmap((Rasthdr *)&fd->vmask,&fd->vmask)) < 0)
		goto free_lb_out;

	fd->visitbuf = fd->vmask.bm.bp[0];

	switch (vs.ink_id)
	{
	    case vsp_INKID: /* VGRAD */
	    case hsp_INKID: /* HGRAD */
			if ((err = flood_visit(fd,nofunc,x,y,vs.ccolor))< Success)
				goto free_vmask_out;
			set_xy_gradrect(fd->fhx0,fd->fhy0,fd->fhx1,fd->fhy1);
			break;
		default:
			break;
	}
	err = flood_visit(fd,flood_rhline,x,y,vs.ccolor);

free_vmask_out:
	pj_close_raster(&(fd->vmask));
free_lb_out:
	pj_gentle_free(fd->linebuf);
free_fd_out:
	pj_gentle_free(fd);
	return(err);
}

static Errcode some_flood(USHORT x, USHORT y,int floodto, Pixel fcolor)
{
Errcode err;

	if((err = make_render_cashes()) < 0)
		return(err);
	err = csd_some_flood(x,y,floodto,fcolor,undof);
	free_render_cashes();
	return(err);
}


static Errcode flood_visit(register Floodata *fd,
						EFUNC hout, USHORT x, USHORT y, USHORT color)
{
int left, right;
struct fseg *next;
Errcode err;

	pj_clear_rast(&fd->vmask);
	fd->fread_pt = fd->fwrite_pt = fd->fcount = 0;
	fd->fhx0 = fd->fhx1 = x;
	fd->fhy0 = fd->fhy1 = y;
	add_fseg(fd, y, x, x);
	while (fd->fcount > 0)
	{
		if ((err = poll_abort())<Success)
			return(err);
		next = fd->fsegments + fd->fread_pt;
		fd->fread_pt++;
		fd->fread_pt&=FMOD;
		fd->fcount -= 1;
		y = next->y;
		left = next->left;
		if(!(fd->visitbuf[y*fd->vmask.bm.bpr + (left>>3)] & bit_masks[left&7] ))
		{
			expand_fseg(fd,next);
			left = next->left;
			right = next->right;
			visit_hline(fd, y, left, right);

			if((err = hout(y, left, right, color)) < Success)
				goto out;

			if (y<fd->fhy0)
				fd->fhy0 = y;
			if (y>fd->fhy1)
				fd->fhy1 = y;
			if (left < fd->fhx0)
				fd->fhx0 = left;
			if (right > fd->fhx1)
				fd->fhx1 = right;
			if (y < fd->testscr->height - 1)
				scan_seg(fd,y+1, left, right);
			if (y > 0)
				scan_seg(fd,y-1, left, right);
		}
	}
out:
	return(err);
}



