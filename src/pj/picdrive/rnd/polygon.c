/***************************************************************
RND file pdr modules:

	Created by Peter Kennard.  Oct 11, 1991
		These modules implement reading scan line and polygon data in
		256 color Autoshade render slide files and drawing the image into
		a screen.
****************************************************************/
#include "errcodes.h"
#include "polygon.h"
#include "rnd.h"


static LLpoint *get_llpoint(Pointlist *freep)
/* Allocate a new polygon point from a free list if not present in the free 
 * list zalloc() it */
{
LLpoint *pt;

	if((pt = freep->head) != NULL)
	{
		freep->head = pt->next;
		pt->next = NULL;
		pt->flags = 0;
	}
	else
	{
		pt = zalloc(sizeof(LLpoint));
	}
	return(pt);
}
void release_polypoints(Poly *pg, Pointlist *pts)
/* Move LLpoints from a polygon to a free llpoint list */
{
LLpoint *head;
LLpoint *l;
LLpoint *next;
LLpoint *free;

	free = pts->head;
	l = head = pg->clipped_list;
	while(l != NULL)
	{
		next = l->next;
		l->next = free;
		free = l; 
		if((l = next) == head)
			break;
	}
	pts->head = free;
	pg->clipped_list = NULL;
	pg->pt_count = 0;
}
void free_pointlist(Pointlist *pts)
/* Free all LLpoints in a ploygon. */
{
LLpoint *next;
LLpoint *head;

	head = pts->head;
	while(head != NULL)
	{
		next = head->next;
		free(head);
		head = next;
	}
	pts->head = NULL;
}
static LLpoint *add_polypoint(Rfile *rf, USHORT x, USHORT y)
/* Add a new llpoint to a polygon with the value x and y */
{
LLpoint *pt;

	if((pt = get_llpoint(&rf->pts)) == NULL)
	{
		rf->lasterr = Err_no_memory;
		goto out;
	}
	if(rf->pg.clipped_list == NULL)
	{
		rf->pg.clipped_list = rf->pg.tail = pt;
		pt->next = pt;
	}
	else
	{
		pt->next = rf->pg.clipped_list;
		rf->pg.tail->next = pt;
		rf->pg.tail = pt;
	}

	pt->x = x;
	pt->y = y;
	++rf->pg.pt_count;

out:
	return(pt);
}
static Errcode fill_rf_pgline(SHORT y, SHORT x1, SHORT x2, Rfile *rf)
/* Draw line funtion for poly filler. */
{
	pj_set_hline(rf->screen, rf->pgcolor, x1, y, x2-x1+1);
	return(Success);
}
static void draw_line(Rcel *screen, short x1, short y1, short x2, short y2, 
	short color)
/* Draw a line. */
{
register SHORT   duty_cycle;
SHORT incy;
register SHORT delta_x, delta_y;
register SHORT dots;

	delta_y = y2-y1;
	delta_x = x2-x1;
	if (delta_y < 0) 
	{
		delta_y = -delta_y;
		incy = -1;
	}
	else
	{
		incy = 1;
	}
	if ((delta_x) < 0) 
	{
		delta_x = -delta_x;
		incy = -incy;
		x1 = x2;
		y1 = y2;
	}
	duty_cycle = (delta_x - delta_y)/2;

	if (delta_x >= delta_y)
	{
		dots = ++delta_x;
		while (--dots >= 0)
		{
			pj_put_dot(screen, color, x1,y1);
			duty_cycle -= delta_y;
			++x1;
			if (duty_cycle < 0)
			{
				duty_cycle += delta_x;	  /* update duty cycle */
				y1+=incy;
			}
		}
	}
	else /* dy > dx */
	{
		dots = ++delta_y;
		while (--dots >= 0)
		{
			pj_put_dot(screen, color, x1,y1);
			duty_cycle += delta_x;
			y1+=incy;
			if (duty_cycle > 0)
			{
				duty_cycle -= delta_y;	  /* update duty cycle */
				++x1;
			}
		}
	}
}
Errcode read_polygon(Rfile *rf)
/* Read a polygon record from a RND file,  add points to a Poly structure
 * and if it is a terminal polygon record draw the filled polygon on the 
 * screen. */
{
Rd_poly pg;
USHORT *vert;
LLpoint *pt;


	/* read polygon record header. */
	if(rf_read(rf,&pg,8) < Success)
		goto error;

#ifdef PRINTSTUFF
	printf("nvert %d f %04x c %d", pg.nvert, pg.flags, pg.color);
#endif


	if((USHORT)pg.nvert > 10) /* bad data! */
	{
		rf->lasterr = Err_format;
		goto error;
	}

	if(pg.nvert > 0) /* Add vertices to polygon if there are any. */
	{
		if(rf_read(rf,&pg.vx,pg.nvert<<2) < Success)
			goto error;

		vert = pg.vx;
		while(pg.nvert-- > 0)
		{
			if(add_polypoint(rf,vert[0],rf->hdr.ydots-1-vert[1]) == NULL)
				goto error;
			vert += 2;
		}
	}

	/* If this is a partial record we are done for now. */
	if(pg.flags & RF_MORE)
		return(Success);

	/* Otherwise fill the polygon and draw the "edges" in the edge color. */
	rf->pgcolor = pg.color;

	if((rf->lasterr = fill_poly_inside(&rf->pg,fill_rf_pgline,rf)) < Success)
		goto error;

	pt = rf->pg.clipped_list;
	for(;;)
	{
		draw_line(rf->screen, pt->x, pt->y, pt->next->x, pt->next->y,
				  pg.ecolor);
		if((pt = pt->next) == rf->pg.clipped_list)
			break;
	}

error:
	release_polypoints(&rf->pg,&rf->pts);
	return(rf->lasterr);
}
