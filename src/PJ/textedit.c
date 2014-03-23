/* textedit.c - This file tries to be a what-you-see is what you get
   text editor with dynamic word-wrap on a possibly proportial spaced
   font - an editor open on an arbitrarily sized window.  Heck, it
   works most of the time!  See also textwind.c wordwrap.c and rastext.c */

#include <stdio.h>
#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "marqi.h"
#include "memory.h"
#include "softmenu.h"
#include "textedit.h"
#include "util.h"

extern char *strstr(char *s1, char *s2);

static void marqi_twinbox(Text_file *gf);
static slide_down();
static slide_up();
static void scroll_up();
static void scroll_down();
static void define_twin();
static void move_twin();

static void clip_cursorp(Text_file *gf)
/* Make sure cursor is inside defined text area */
{
if (gf->tcursor_p < 0)
	gf->tcursor_p = 0;
if (gf->tcursor_p > gf->text_size)
	gf->tcursor_p = gf->text_size;
}

static char *nextline(Text_file *gf, char *s)
/* Do wordwrap to find next line assuming current font and text window width. */
{
char buf[512];

	wwnext_line(gf->font, &s, gf->twin.width, buf, 0);
	return(s);
}

static char *seek_line(Text_file *gf, char *s, int line)
/* Find character pointer to start of a specific line of buffer */
{
while (--line >= 0)
	{
	s = nextline(gf, s);
	}
return(s);
}



static void wwrefresh(Text_file *gf, char *s, Linedata *ldat,
			 		   SHORT x, SHORT y,SHORT w,
				  	   SHORT lines, Pixel color, SHORT skiplines, SHORT forceit)

/* Update lines of text display if forceit == 0 it will only redraw
 * and rezoom changes if forceit == 2 it will redraw all text and not zoom 
 * if == 1 will zoom too */
{
char buf[512];
SHORT dy;
SHORT zoomstart, textstart;
SHORT zoomwid, textwid;
char *nexts;
ULONG crcsum;
Vfont *font = gf->font;

	dy = font_cel_height(font);
	if(ldat)
	{
		ldat += skiplines;
	}

	lines += skiplines;
	nexts = s;

	while(--lines >= 0)
	{
		if((s = nexts) != NULL)
			wwnext_line(gf->font, &nexts, w, buf, 0);

		if (--skiplines < 0)
		{
			if(ldat)
			{
				if(s == NULL)
					crcsum = 0;
				else
					crcsum = str_crcsum(buf);
				ldat->cstart = s;
				if(!forceit && ldat->crcsum == crcsum)
				{
					++ldat;
					y += dy;
					continue;
				}
				textwid = ldat->width;
				textstart = x + ldat->xstart;
			}
			else 
			{
				textwid = w;
				textstart = x;
			}

			gf->undraw_rect(gf->raster, gf->undraw_data
			, textstart-font->left_overlap, y
			,  textwid+font->left_overlap+font->right_overlap+1
			, gf->line_height);

			zoomwid = textwid;
			zoomstart = textstart;

			if(s != NULL)
			{
				textwid = justify_line(gf->raster, gf->font,buf,x,y,w,color,
									   TM_MASK1,sblack, gf->justify_mode,
									   &textstart, 0);

				if(!zoomwid)
				{
					zoomwid = textwid;
					zoomstart = textstart;
				}
				else
				{
					if(textstart < zoomstart) 
					{
						zoomwid += zoomstart - textstart;
						zoomstart = textstart;
					}
					if(textwid > zoomwid)
						zoomwid = textwid;
				}
			}
			else
			{
				textstart = x;
				textwid = 0;
			}
			if(ldat)
			{
				ldat->crcsum = crcsum;
				ldat->xstart = textstart - x;
				ldat->width = textwid;
				++ldat;
			}
			if(forceit != 2)
				rect_zoom_it(zoomstart, y, zoomwid, gf->line_height);
		}
		y += dy;
	}
return;
}

static draw_twintext(Text_file *gf, int forceall)
{
	wwrefresh(gf, gf->wstart, gf->ldat, gf->twin.x, gf->twin.y, 
			  gf->twin.width, gf->text_lines_visible, gf->ccolor, 0, forceall);
}

static void fix_overlapped_border(Text_file *gf)
/* If the font can go beyond the text editor border, redraw the border. */
{
	Vfont *font = gf->font;

	if (font->left_overlap != 0 || font->right_overlap != 0)
		marqi_twinbox(gf);
}

static void refresh_twintext(Text_file *gf)
/* only redraws parts that have changed */
{
	draw_twintext(gf, 0);
	fix_overlapped_border(gf);
}

static void marqtwin(Text_file *gf, void *dotdat, VFUNC pdot)
{
	cline_frame(gf->twin.x - 1, gf->twin.y - 1,
				gf->twin.x + gf->twin.width,
				gf->twin.y + gf->twin.height,
				pdot,dotdat);
}

static void marqi_twinbox(Text_file *gf)
{
Marqihdr mh;

	if(gf->raster == (Raster *)(vb.pencel))
		vinit_marqihdr(&mh,0,1);
	else
		init_marqihdr(&mh, (Wndo *)gf->raster, NULL, gf->ccolor, gf->ccolor);

	marqtwin(gf,&mh,mh.pdot);
}

static void undo_twinbox(Text_file *gf)
{
Marqihdr mh;

	if (gf->undraw_dot == NULL)
		{
		vinit_marqihdr(&mh,0,1);
		marqtwin(gf,&mh,undo_marqidot);
		}
	else
		{
		marqtwin(gf, gf->undraw_data, gf->undraw_dot);
		}
}

static void dtextcel(Text_file *gf, Boolean doborder)
{
	if (gf->text_buf != NULL)
	{
		/* Force draw of all text in window */
		draw_twintext(gf, 2);
		rect_zoom_it(gf->twin.x,gf->twin.y,gf->twin.width,gf->twin.height);
	}
	if(doborder)
		marqi_twinbox(gf);
}

void free_text_file(Text_file *gf)
{
	pj_freez(&gf->text_buf);
}


static void cursor_to_pixel(Text_file *gf)
/* get pixel location of cursor.  Return result in pixelx, pixely, and
   pixelw.  Set up a bunch of variables to track current character and
   line position. */
{
char lbuf[512];
char *np;
char *cstart;

	clip_cursorp(gf);
	gf->pixely = gf->twin.y;
	cstart = gf->text_buf + gf->tcursor_p;
	gf->lstart = gf->wstart;
	gf->llstart = gf->lwstart;
	for (gf->twypos=0;gf->twypos<gf->text_lines_visible;gf->twypos++)
	{
		np = gf->lstart;
		wwnext_line(gf->font, &np, gf->twin.width, lbuf, 0);

		if (np == NULL || np > cstart)
			break;
		gf->llstart = gf->lstart;
		gf->lstart = np;
		gf->pixely += gf->line_height;
	}
	gf->twxpos = cstart - gf->lstart;

	/* calculate cursor position values, used only by cursor display */

	gf->pixelx = just_charstart(gf->font,gf->twin.x, gf->twin.width,
				   			lbuf, gf->twxpos, gf->justify_mode );

	if (gf->tcursor_p == gf->text_size || *cstart == '\n')
	{
		if(gf->justify_mode == JUST_FILL && gf->twxpos != 0)
			gf->pixelx = gf->tw_maxx;
		gf->pixelw = 0;
	}
	else
	{
		gf->pixelw = fendchar_width(gf->font,cstart);
		if(gf->pixelw < 1) /* some fonts are weird */
			gf->pixelw = (gf->font->end_space);
	}
}

#ifdef OLDWAY
void hide_text_cursor(Text_file *gf)
{
SHORT hgt, top;
int width;

	top = gf->pixely;
	hgt = gf->line_height;
	width = gf->pixelw;

	if(gf->overwrite)
	{
		top += hgt;
		hgt >>= 2;
		top -= hgt;

		pj_xor_rect(gf->raster, gf->text_cursor_color, 
				    gf->pixelx, gf->pixely, width, hgt);

	}

	pj_xor_rect(gf->raster, gf->text_cursor_color, 
			    gf->pixelx, top, width, hgt);

	rect_zoom_it(gf->pixelx, gf->pixely, width, gf->line_height);
}
#endif /* OLDWAY */

void xor_text_cursor(Text_file *gf)
{
SHORT hgt, top, top2;
int width, x;
Rectangle owr;

	top = gf->pixely;
	hgt = gf->cursor_hgt;

	/* cursor can exit right of window by width, clip it, pull back 2 pixels */
	width = 2;
	x = gf->pixelx + width;
	if((x = gf->pixelx + width) >= gf->tw_maxx)
		x = gf->tw_maxx;
	x -= width;

	if(gf->overwrite)
	{
		top2 = top;
		top += hgt;
		owr.y = top - 2;

		if(gf->pixelw > 2)
		{
			top -= 2;
			hgt>>=1;
			top -= hgt;

			/* draw underline */

			if((owr.width = x + gf->pixelw) > gf->tw_maxx)
				owr.width = gf->tw_maxx;
			owr.width -= x;

			pj_xor_rect(gf->raster, gf->text_cursor_color, 
						 x, owr.y, owr.width, 2 );

			rect_zoom_it(x, owr.y, owr.width, 2);


			if(owr.width > (gf->pixelw>>1))
				goto do_vertical;

			/* draw overbar for thin ones on end */
			owr.y = gf->pixely;

			pj_xor_rect(gf->raster, gf->text_cursor_color, 
						 x, owr.y, owr.width, 2 );

			rect_zoom_it(x, owr.y, owr.width, 2);
			top2 += 2;
		}
		else
		{
			hgt >>= 2;
			top -= hgt;
		}

		pj_xor_rect(gf->raster, gf->text_cursor_color, 
				    x, top2, width, hgt);
	}
do_vertical:

	pj_xor_rect(gf->raster, gf->text_cursor_color, 
			    x, top, width, hgt);

	rect_zoom_it(x, gf->pixely, width, gf->cursor_hgt);
	gf->cursor_up = !gf->cursor_up;
}

void hide_text_cursor(Text_file *gf)
{
	if(gf->cursor_up)
		xor_text_cursor(gf);
}

#ifdef SLUFFED
void show_text_cursor(Text_file *gf)
{
	cursor_to_pixel(gf);
	gf->cursor_up = FALSE;
	xor_text_cursor(gf);
}
#endif /* SLUFFED */

static void find_wstart(Text_file *gf)
{
if (gf->text_yoff == 0)
	{
	gf->wstart = gf->text_buf;
	gf->lwstart = NULL;
	}
else
	{
	gf->lwstart = seek_line(gf, gf->text_buf, gf->text_yoff-1);
	gf->wstart = seek_line(gf, gf->lwstart, 1);
	}
}

cleanup_twin(Text_file *gf)
{
	pj_gentle_free(gf->ldat);
	gf->ldat = NULL;
}
static Errcode init_twin(Text_file *gf)
{
SHORT h;
SHORT font_dx, font_dy;

	/* if don't have a gf->text_buf make one */
	if (gf->text_buf == NULL)
    {
	    if ((gf->text_buf = begmem(DTSIZE)) == NULL)
	        return(Err_reported);
		gf->text_yoff = gf->tcursor_p = 0;
	    gf->text_size = 0;
	    gf->text_buf[0] = 0;
	    gf->text_alloc = DTSIZE;
    }
	/* calculate some constants */
	gf->line_height = font_dy = font_cel_height(gf->font);
	gf->cursor_hgt = gf->font->image_height + gf->font->default_leading;

	font_dx = widest_char(gf->font)+2;
	if (font_dx <= 0)
		font_dx = CH_WIDTH;
	/* make sure window is big enough for one character or word wrap will 
	   hang...*/

	/* cast necessary.  Sometimes have negative values 
	   in unsigned sizes */

	if ((SHORT)gf->twin.height < font_dy)
		gf->twin.height = font_dy;
	if ((SHORT)gf->twin.width < font_dx)
		gf->twin.width = font_dx;

	/* calculate how many lines visible */
	h = gf->twin.height;
	gf->text_lines_visible = 0;

	gf->tw_maxx = gf->twin.x + gf->twin.width;

	for (;;)
    {
	    if (h < gf->line_height)
	        break;
	    gf->text_lines_visible++;
	    h -= font_dy;
    }

	/* if not gotten not used */ 
	pj_gentle_free(gf->ldat);
	gf->ldat = pj_zalloc(sizeof(Linedata)*gf->text_lines_visible); 

	if (gf->ccolor == 0)
		gf->text_cursor_color = 255;
	else
		gf->text_cursor_color = gf->ccolor;

	find_wstart(gf);
	slide_up(gf);
	slide_down(gf);
	dtextcel(gf,1);
	gf->prev_xpos = gf->twxpos;
	return(Success);
#ifdef NOT_USED
error:
	cleanup_twin(gf);
	return(err);
#endif
}

static Boolean past_window(Text_file *gf)
/* return 1 if cursor is now below text window */
{
char *p, *curp;
SHORT i;

p = gf->wstart;
curp = gf->text_buf + gf->tcursor_p;
for (i=0;i<gf->text_lines_visible;i++)
	{
	p = nextline(gf, p);
	if (p == NULL || p > curp)
		{
		return(0);
		}
	}
return(1);
}

static slide_down(Text_file *gf)
/* if cursor position past end of window, slide it down. */
{
SHORT slid = 0;

clip_cursorp(gf);
while (past_window(gf))
	{
	gf->text_yoff++;
	gf->wstart = nextline(gf, gf->wstart);
	slid = 1;
	}
if (slid)
	find_wstart(gf);
return(slid);
}

static slide_up(Text_file *gf)
/* if cursor position before start of window slide it up */
{
clip_cursorp(gf);
if (gf->wstart == NULL || gf->tcursor_p < gf->wstart - gf->text_buf)
	{
	gf->text_yoff = 0;
	find_wstart(gf);
	return(TRUE);
	}
return(FALSE);
}

static Boolean xseek(Text_file *gf, char *p, SHORT x)
/* move cursor to closest x position possible in line p */
/* Returns FALSE if at EOF */
{
char *pp;

if ((pp = nextline(gf, p)) != NULL)
	{
	if (x < pp - p)	/* if enough characters in line hit it exactly */
		{
		gf->tcursor_p = p + x - gf->text_buf;
		}
	else				/* else position cursor at end of line */
		{
		gf->tcursor_p = pp - gf->text_buf - 1;
		}
	return(TRUE);
	}
else	/* if on last line position at end of file and let clip_cursorp clean
           up any overshoot. */
	{
	gf->tcursor_p = p - gf->text_buf + x;
	clip_cursorp(gf);
	return(FALSE);
	}
}

static void xyseek(Text_file *gf, char *p, SHORT x, SHORT y)
/* Move cursor position to close as it can find to x,y offset from p */
{
char *np;

while (--y >= 0)
	{
	if ((np = nextline(gf, p)) == NULL)
		break;
	p = np;
	}
xseek(gf, p, x);
}

static void update_end(Text_file *gf, char *nline)
/* refresh cursor line to bottom of window.  If cursor line bottom of
   window scroll down too. */
{
	if (nline != NULL && 
		gf->twypos >= gf->text_lines_visible - 1 &&
		nline - gf->text_buf <= gf->tcursor_p)
	{
		scroll_down(gf);
		refresh_twintext(gf);
	}
	else
	{
		wwrefresh(gf, gf->wstart, gf->ldat, gf->twin.x,gf->twin.y,gf->twin.width,
			gf->text_lines_visible - gf->twypos, gf->ccolor,gf->twypos,0);
	}
}

static void update_mark(Text_file *gf,
	long alterp, long count, long *pmark)
{
long mark = *pmark;
(void)gf;

if (alterp <= mark)
	{
	mark += count;
	if (mark < 0)
		mark = 0;
	*pmark = mark;
	}
}

static void update_win(Text_file *gf, 
		long extracs) /* # of characters added or subtracted 
		 							 * at cursor position */

/* Called when characters inserted or deleted. Tries to minimize lines
   that have to be redrawn. */
{
char *new_lstart, *new_nlstart;

update_mark(gf, gf->tcursor_p, (long)extracs, &gf->block_start);
update_mark(gf, gf->tcursor_p, (long)extracs, &gf->block_end);
/* only one line, just make sure it's one with
									cursor in it and redraw */
if (gf->text_lines_visible == 1)	
	{
	if (gf->llstart == NULL)	/* 1st line of text */
		{
		new_lstart = gf->lstart;
		}
	else
		{
		new_lstart = nextline(gf, gf->llstart);
		}
	if (gf->tcursor_p < new_lstart - gf->text_buf)	/* prev line? */
		{
		scroll_up(gf);
		}
	else
		{
		/* check and see if gone on to next line */
		cursor_to_pixel(gf);
		if (gf->twypos >= 1)
			{
			scroll_down(gf);
			}
		}
	refresh_twintext(gf);
	}
	else /* more than one line, figure out which ones to update */
	{
		if(gf->llstart==NULL) /*if no llstart we are on first line of file */
			new_lstart = gf->lstart;
		else
			new_lstart = nextline(gf, gf->llstart);
		new_nlstart = nextline(gf, new_lstart);

		/* check if effected previous line somehow... */
		if (gf->llstart != NULL && new_lstart != gf->lstart)
		{
				/* and previous line is outside window */
			if (gf->twypos == 0)	
			{
				scroll_up(gf);	
				refresh_twintext(gf);	/* update everyone and split */
				return;
			}
			else				/* previous line in window and needs update */
			{
				wwrefresh(gf, gf->wstart, gf->ldat, gf->twin.x, gf->twin.y, 
						  gf->twin.width, 1, gf->ccolor, gf->twypos-1,0);
			}
		}
		if (new_nlstart == NULL && gf->nlstart == NULL) /* typing at EOF */
		{
			wwrefresh(gf, gf->wstart, gf->ldat, 
				gf->twin.x,gf->twin.y,gf->twin.width,1,
					  gf->ccolor,gf->twypos,0);
		}
		else if (new_nlstart == NULL || gf->nlstart == NULL)
											/* update next line near EOF */
		{
			update_end(gf, new_nlstart);
		}
		else if (new_nlstart != gf->nlstart+extracs)	
											/* next line effected... */
		{
			update_end(gf, new_nlstart);
		}
		else /* not at EOF, but doesn't effect next line either. normal case */
		{
			wwrefresh(gf, gf->wstart, gf->ldat, 
					gf->twin.x,gf->twin.y,gf->twin.width,
					  1,gf->ccolor,gf->twypos,0);
		}
	}
	fix_overlapped_border(gf);

}

static void scroll_up(Text_file *gf)
{
if (gf->text_yoff > 0)
	{
	gf->text_yoff -= 1;
	find_wstart(gf);
	}
}

static void scroll_down(Text_file *gf)
{
gf->text_yoff += 1;
for (;;)		/* this loop handles going past bottom... */
	{
	find_wstart(gf);
	if (gf->wstart != NULL)
		break;
	gf->text_yoff -= 1;
	}
}


static void insert_at_cp(Text_file *gf, char *buf,  long count)
{
	if (gf->text_size < ((gf->text_alloc-count)-1))
	{
		if (!gf->read_only)
			{
			gf->is_changed = TRUE;
			gf->nlstart = nextline(gf, gf->lstart);
			back_copy_mem(gf->text_buf+gf->tcursor_p, 
						  gf->text_buf+gf->tcursor_p+count,
				gf->text_size - gf->tcursor_p+count);
			pj_copy_bytes(buf,  gf->text_buf+gf->tcursor_p, (int)count);
			gf->tcursor_p += count;
			gf->text_size += count;
			}
	}
	else
	{
		softerr(Err_too_big, "cant_expand");
	}
}

static void insert_char(Text_file *gf, char c)
{
	insert_at_cp(gf, &c, (long)1);
	update_win(gf,(long)1);
}
static void overwrite_char(Text_file *gf, char c)
{
char *cc;

	cc = gf->text_buf + gf->tcursor_p;

	/* if at end of text or hard end of line insert chars */

	if( (gf->text_size == gf->tcursor_p)
		|| *cc == '\n'
		|| *cc == 0 )
	{
		insert_char(gf,c);
		return;
	}
	*cc = c; /* replace existing char */
	++gf->tcursor_p; /* cursor forward by one */
	update_win(gf,0);
}

delete_at_cp(Text_file *gf, long dcount)
{
int count;

if (gf->read_only || gf->text_size <= 0 || dcount <= 0)
	return;
count = gf->text_size - gf->tcursor_p;
if (count > 0)
	{
	gf->is_changed = TRUE;
	gf->nlstart = nextline(gf, gf->lstart);
	pj_copy_bytes(gf->text_buf+gf->tcursor_p+dcount, gf->text_buf+gf->tcursor_p,
		count);
	gf->text_size -= dcount;
	}
}

static void delete_char(Text_file *gf)
{
delete_at_cp(gf, 1L);
update_win(gf,-1L);
}

static void delete_line(Text_file *gf)
{
long startp, endp;
long dcount;

/* seek to end */
xseek(gf, gf->lstart, 10000);
endp = gf->tcursor_p;
dcount = xseek(gf, gf->lstart, 0);
startp = gf->tcursor_p;
dcount += endp-startp;

delete_at_cp(gf, dcount);
update_win(gf,-dcount);
}
/* subroutines for text_edit */

static void left_one(Text_file *gf)
{
	if (gf->tcursor_p > 0)
	{
		gf->tcursor_p-=1;
		if (gf->tcursor_p < gf->wstart-gf->text_buf)
		{
			scroll_up(gf);
			refresh_twintext(gf);
		}
	}
	return;
}
static void right_one(Text_file *gf)
{
	if (gf->tcursor_p < gf->text_size)
	{
		gf->tcursor_p+=1;
		if (past_window(gf))
		{
			scroll_down(gf);
			refresh_twintext(gf);
		}
	}
	return;
}
static void up_line(Text_file *gf)
{
char *seekline;

	if (gf->twypos <= 0)
	{
		if (gf->wstart > gf->text_buf)
		{
			scroll_up(gf);
			refresh_twintext(gf);
		}
		xseek(gf, gf->wstart, gf->prev_xpos);
	}
	else
	{
		seekline = seek_line(gf, gf->wstart, gf->twypos-1);
		xseek(gf, seekline, gf->prev_xpos);
	}
	return;
}
static void down_line(Text_file *gf)
{
char *nxtline;

	if ((nxtline = nextline(gf, gf->lstart)) != NULL)
	{
		if (gf->twypos >= gf->text_lines_visible-1)
		{
			scroll_down(gf);
			refresh_twintext(gf);
		}
		xseek(gf, nxtline, gf->prev_xpos);
	}
}
/* end subroutines for text_edit */

static text_key_edit(Text_file *gf, USHORT inkey)

/* iit is assumed when this is called that the text cursor is hidden */
{
int askey = inkey&0xff;

    switch (askey)
    {
        case 0:
		{
            switch(inkey)
            {
				case FKEY1:
				{
					if (gf->help_function != NOFUNC)
						gf->help_function(gf);
					break;
				}
				case INSERTKEY:
				{
					gf->overwrite = !gf->overwrite;
					break;
				}
                case PAGEUP:
				{
					if(gf->text_yoff == 0)
					{
						gf->tcursor_p = 0;
					}
					else
					{
						if (gf->text_lines_visible == 1)
							gf->text_yoff -= 1;
						else
							gf->text_yoff -= gf->text_lines_visible-1;
						if (gf->text_yoff < 0)
							gf->text_yoff = 0;
						find_wstart(gf);
						refresh_twintext(gf);
						xyseek(gf, gf->wstart, gf->prev_xpos, gf->twypos);
					}
					goto keep_prevx;
				}
				case PAGEDN:
				{
					if (seek_line(gf, gf->wstart, gf->text_lines_visible) 
						!= NULL)
					{
						if (gf->text_lines_visible == 1)
							gf->text_yoff += 1;
						else
							gf->text_yoff += gf->text_lines_visible-1;
						find_wstart(gf);
						refresh_twintext(gf);
						xyseek(gf, gf->wstart, gf->prev_xpos, gf->twypos);
					}
					else
					{
						gf->tcursor_p = gf->text_size;
					}
					goto keep_prevx;
				}
				case DELKEY:
					if(ISDOWN(KBSHIFT))
						delete_line(gf);
					else
						delete_char(gf);
					break;
				case ENDKEY:
					xseek(gf, gf->lstart, 10000);
					break;
				case HOMEKEY:
					xseek(gf, gf->lstart, 0);
					break;
				case LARROW:
					left_one(gf);
					break;
				case RARROW:
					right_one(gf);
					break;
				case UARROW:
					up_line(gf);
					goto keep_prevx;
				case DARROW:
					down_line(gf);
					goto keep_prevx;

			} /* end inkey switch */
			break;
		}
		case ESCKEY:
			return(Err_abort);
		case '\b':	/* backspace */
			if (gf->tcursor_p > 0)
			{
				gf->tcursor_p -= 1;
				delete_char(gf);
			}
			break;
		case '\r':		/* Carriage return translates to newline */
			askey = '\n';
			goto new_char;
		case '\t':		/* Tab used even if not in font */
			if(gf->font->tab_width >= gf->twin.width) /* no tabs, too small */
				break;
			goto new_char;
		default:
			if(!in_font(gf->font, askey))
			{
				soft_continu_box("!%d", "nofont_char", askey );
				break;
			}
		new_char:
			if(gf->overwrite)
				overwrite_char(gf,askey);
			else
				insert_char(gf,askey);
			break;
	} /* end icb.inkey & 0x00FF switch */

	gf->prev_xpos = gf->twxpos;

keep_prevx:

	return(Success);
}

static long pixel_to_cursor(Text_file *gf, int x, int y)
{
int dy;
int cy;
int endy;
char *line, *nline;
int chars_in_line;
int glb; 	/* greatest lower bound */
int lub;	/* least upper bound */
int cbound;

dy = font_cel_height(gf->font);
cy = gf->twin.y;
endy = cy + gf->twin.height;
line = gf->wstart;
while (cy < endy)
	{
	cy += dy;
	if (y <= cy)
		break;
	if ((nline = nextline(gf, line)) == NULL)
		break;
	line = nline;
	}
xseek(gf, line, 10000);		/* seek to end of line */
nline = gf->text_buf + gf->tcursor_p;
chars_in_line = nline - line + 1;
/* set up to do a binary search to find position in line */
glb =  0;
lub = chars_in_line;
cbound = chars_in_line;
while (glb+1 < lub)
	{
	if (just_charstart(gf->font, gf->twin.x, gf->twin.width,
		line, cbound, gf->justify_mode) < x)
		{
		glb = cbound;
		/* if mouse past end of text, cursor at end */
		if (cbound == chars_in_line)	/* == lub in this case */
			{
			if (line[glb-1] == '\n')
				--glb;
			break;
			}
		cbound = (cbound+lub)>>1;
		}
	else
		{
		lub = cbound;
		cbound = (cbound+glb)>>1;
		}
	}
return(line - gf->text_buf + glb);
}

#ifdef SLUFFED
static  force_into_window(Text_file *gf)
{
int slid;

slid = slide_up(gf);
slid |= slide_down(gf);
if (slid)
	dtextcel(gf,0);
}

cursor_to_pat(Text_file *gf, char *pat)
{
char *match;

	if  ((match = strstr(gf->text_buf  + gf->tcursor_p + 1, pat)) != NULL)
		goto GOT_MATCH;
	else if ((match = strstr(gf->text_buf, pat)) !=  NULL)
	{
		if(soft_yes_no_box("text_find"))
		{
			goto GOT_MATCH;
		}
	}
	else
	{
		soft_continu_box("!%s","text_notfnd", pat);
	}
	return;

GOT_MATCH:
	gf->tcursor_p = match -  gf->text_buf;
	force_into_window(gf);
}

ted_block_start(Text_file *gf)
{
gf->block_start  = gf->tcursor_p;
}

ted_block_end(Text_file *gf)
{
gf->block_end  = gf->tcursor_p;
}

ted_swap_bends(Text_file *gf)
{
long swap;

if (gf->block_end < gf->block_start)
	{
	swap = gf->block_end;
	gf->block_end = gf->block_start;
	gf->block_start = swap;
	}
}

long ted_copy_block(Text_file *gf, char *clip_name)
{
long count;
Errcode err;

ted_swap_bends(gf);
if ((count = (gf->block_end - gf->block_start)) > 0)
	{
	if ((err = write_gulp(clip_name, gf->text_buf+gf->block_start, count))
		< Success)
		{
		return(err);
		}
	}
return(count);
}

ted_cut_block(Text_file *gf, char *clip_name)
{
long count;

if ((count = ted_copy_block(gf, clip_name)) > 0)
	{
	gf->tcursor_p = gf->block_start;
	force_into_window(gf);
	delete_at_cp(gf, count);
	refresh_twintext(gf);
	}
}

void ted_load_block(Text_file *gf, char *dir, char *clip_name)
{
char path[PATH_SIZE];
char titbuf[80];

	make_file_path(dir,"",path);
	if(get_aafilename(stack_string("load_block",titbuf), ".TXT", 
					  load_str,path, path, 0) != NULL)
	{
		pj_copyfile(path, clip_name);
	}
}

void ted_save_block(Text_file *gf,  char *dir,  char *clip_name)
{
char path[PATH_SIZE];
char titbuf[80];

	make_file_path(dir,"",path);
	if(get_aafilename(stack_string("save_block",titbuf), ".TXT", 
					  save_str, path, path, 0) != NULL)
	{
		ted_copy_block(gf, path);
	}
}

ted_insert_block(Text_file *gf, char *clip_name)
{
FILE  *f;
char buf[80];
long len;
long ltotal = 0;

if ((f = fopen(clip_name, r_str)) != NULL)
	{
	while (fgets(buf, sizeof(buf), f) != NULL)
		{
		len = strlen(buf);
		insert_at_cp(gf, buf, len);
		ltotal += len;
		}
	refresh_twintext(gf);
	fclose(f);
	}
}



ted_doit(Text_file *gf)
/* Wndo doit function for text-editor */
{
Errcode err = Success;

hide_text_cursor(gf);
if (JSTHIT(KEYHIT))
	err  = text_key_edit(gf, icb.inkey);
else if (JSTHIT(MBPEN))
	gf->tcursor_p = pixel_to_cursor(gf, icb.mx, icb.my);
else if (JSTHIT(MBRIGHT))
	err = Err_abort;
show_text_cursor(gf);
return(err);
}
#endif /* SLUFFED */

static int anim_text_cursor(Text_file *gf)
{
	xor_text_cursor(gf);
	return(0);
}

static void display_two_texts(char *key1, char *key2)
/*
 * Load in two text strings from menu file and display
 * a continue-dialog-box with both strings. 
 */
{
	char *text1, *text2;

	if (smu_load_text(&smu_sm, key1, &text1) >= Success)
		{
		if (smu_load_text(&smu_sm, key2, &text2) >= Success)
			{
			continu_box("%s%s", text1, text2);
			smu_free_text(&text2);
			}
		smu_free_text(&text1);
		}
}

static void movable_editor_help(Text_file *gf)
/* 
 * Help for editors where left mouse button moves
 * window around.
 */
{
(void)gf;
display_two_texts("editor_help", "editor_move_window");
}

static void mouse_position_editor_help(Text_file *gf)
/*
 * Help for editors where left mouse button positions cursor.
 */
{
(void)gf;
display_two_texts("editor_help", "editor_move_cursor");
}

static void set_editor_help(Text_file *gf)
/*
 * If no help function yet, fill in with generic help about
 * text editor.
 */
{
	if (gf->help_function == NOFUNC)
		{
		if (gf->is_movable)
			gf->help_function = movable_editor_help;
		else
			gf->help_function = mouse_position_editor_help;
		}
}

static Errcode ed_text_file(Text_file *gf)
{
Errcode err;
int inside;

	set_editor_help(gf);
	for(;;)
	{
	    /* show_text_cursor(gf); */
		/* wait_wndo_input(ANY_CLICK); */

		cursor_to_pixel(gf);
		anim_wndo_input(ANY_CLICK, 0, 30, anim_text_cursor, gf);

		inside = ptin_rect(&gf->twin, icb.mx, icb.my);
	    hide_text_cursor(gf);

		/* this is a little cludge to allow zoom window input */

		if(check_zoom_drag())
			continue;

		if (JSTHIT(MBRIGHT))
			return(Err_abort);
	    if (JSTHIT(KEYHIT))
		{
			if ((err = text_key_edit(gf, icb.inkey)) < Success)
				return(err);
			continue;
		}
	    if (JSTHIT(MBPEN))
		{
		if (gf->is_movable)
			{
			if (!inside)
				return(Success);
			move_twin(gf);
			if ((err = init_twin(gf)) < Success)
				return(err);
			}
		else
			{
			if (inside)
				{
				gf->tcursor_p = pixel_to_cursor(gf, icb.mx, icb.my);
				}
			}
		}
  	} 
}

Boolean edit_text_file(Text_file *gf)
{

	for (;;)
	{
		if (init_twin(gf) < Success)
			return(0);
		if (ed_text_file(gf) < Success)
			break;
		define_twin(gf);
	}

	cleanup_twin(gf);
	return(1);
}

static Boolean undraw_twin(Text_file *gf, Boolean got_border)

/* returns 1 if there was text in the window 0 if not */
{
Linedata *ld = gf->ldat;
Vfont *font = gf->font;
Coor dy;
Coor y;
SHORT linesleft;
SHORT istext;

	if(!ld)
	{
		gf->undraw_rect(gf->raster, gf->undraw_data
		, gf->twin.x-font->left_overlap-1, gf->twin.y-1
		, gf->twin.width+font->left_overlap+font->right_overlap+3
		, gf->twin.height+2);
		return(gf->text_buf != NULL);
	}

	y = gf->twin.y;
	dy = font_cel_height(gf->font);
	linesleft = gf->text_lines_visible;

	istext = 0;
	while(linesleft > 0 && ld->cstart != NULL)
	{
		if(ld->width)
		{
			istext = 1;
			gf->undraw_rect(gf->raster, gf->undraw_data
			, gf->twin.x + ld->xstart - font->left_overlap, y
			, ld->width + font->left_overlap + font->right_overlap + 1
			, gf->line_height);
		}
		y += dy;
		ld->width = 0; /* erased */
		++ld;
		--linesleft;
	}
	if (got_border)
		undo_twinbox(gf);
	return(istext);
}

Errcode get_rub_twin(Text_file *gf, Boolean cutout)
{
Errcode err;
Marqihdr mh;

	if(cutout)
	{
		if(gf->raster == (Raster*)vb.pencel)
		{
			if((err = cut_out_rect(&gf->twin)) < 0)
				return(err);
		}
		else
		{
			init_marqihdr(&mh, (Wndo *)gf->raster, NULL,
					gf->ccolor, gf->ccolor);
			if((err = mh_cut_rect(&mh,&gf->twin,NULL)) < 0)
				return(err);
		}
	}
	else
	{
		if(gf->raster != (Raster*)vb.pencel)
			return(Err_bad_input);
		if((err = get_srub_rect(&gf->twin)) < 0)
			return(err);
	}
	/* because the window itself is inside the border */
	++gf->twin.x;
	++gf->twin.y;
	gf->twin.width -= 2;
	gf->twin.height -= 2;
	return(0);
}
static void define_twin(Text_file *gf)
{
	gf->undraw_rect(gf->raster, gf->undraw_data, 0,0,gf->raster->width,
					gf->raster->height );

	zoom_it();
	get_rub_twin(gf,1);
}
static void move_twin(Text_file *gf)
{
SHORT ix, iy;
SHORT lx, ly;
Boolean notext;

	ix = gf->twin.x;
	iy = gf->twin.y;
	notext = !undraw_twin(gf, TRUE);
	for (;;)
	{
		lx = icb.mx;
		ly = icb.my;

		box_coors(gf->twin.x,gf->twin.y,ix,iy);
		dtextcel(gf,notext); /* this zooms */
		wait_any_input(); 
		undraw_twin(gf, notext);  /* this doesn't zoom */

		lx = gf->twin.x + icb.mx - lx; /* new position in lx,ly */
		ly = gf->twin.y + icb.my - ly;

		/* zoom part erased and left behind by move */
		do_leftbehind(gf->twin.x,gf->twin.y,
					  lx,ly,gf->twin.width,
					  gf->twin.height, (do_leftbehind_func)rect_zoom_it );

		gf->twin.x = lx; /* new pos to twin */
		gf->twin.y = ly;

		if (JSTHIT(MBPEN))
			break;
		if (JSTHIT(MBRIGHT|KEYHIT))
		{
			gf->twin.x = ix;
			gf->twin.y = iy;
			zoom_it(); 
			break;
		}
	}
	cleanup_toptext();
}


