
/* textedit.c - This file tries to be a what-you-see is what you get
   text editor with dynamic word-wrap on a possibly proportial spaced
   font - an editor open on an arbitrarily sized window.  Heck, it
   works most of the time!  See also textwind.c wordwrap.c and rfont.c */

#include "jimk.h"
#include "blit8_.h"
#include "cblock_.h"
#include "commonst.h"
#include "fli.h"
#include "flicmenu.h"
#include "gemfont.h"
#include "peekpok_.h"
#include "text.h"
#include "textedit.str"

extern char *text_buf;
extern int text_size;
extern int text_alloc;
extern int cfont_widest;

extern render_bitmap_blit();

static int text_cursor_color;
static int text_lines_visible;
static WORD font_dy, font_dx, font_hgt, font_hi, font_lo;

static WORD pixelx, pixely, pixelw;	/* text cursor pixel location */
static char *wstart;	/* upper left character in window */
static char *lwstart;	/* line above top of window... */
static char *lstart;	/* start of line of text cursor is in */
static char *llstart;	/* start of line previous to text cursor */
static char *nlstart;	/* start of line after text cursor */
static char *cstart;	/* character under cursor */
static UBYTE catend;	/* cursor at end? */
static WORD twypos;		/* line of text window cursor is in */
static WORD twxpos;		/* character in line cursor is in */

/* Make sure cursor is inside defined text area */
static
clip_cursorp()
{
if (vs.tcursor_p < 0)
	vs.tcursor_p = 0;
if (vs.tcursor_p > text_size)
	vs.tcursor_p = text_size;
}

/* Do wordwrap to find next line assuming current font and text window
   width. */
static
char *
nextline(s)
char *s;
{
char buf[256];

return(wwnext_line(usr_font, s, vs.twin_w, buf, 0));
}

/* Find character pointer to start of a specific line of buffer */
static
char *
seek_line(s, line)
char *s;
int line;
{
while (--line >= 0)
	{
	s = nextline(s);
	}
return(s);
}


/* Plot text window permanently on rendering screen considering inks
   and all. */
static
rendr_twin()
{
if (text_buf != NULL)
	{
	render_grad_twin();
	if (make_render_cashes())
		{
		wwtext(render_form, usr_font, text_buf, 
			vs.twin_x, vs.twin_y, vs.twin_w, vs.twin_h, 
			vs.ccolor, render_bitmap_blit, vs.text_yoff, 0);
		free_render_cashes();
		dirties();
		}
	}
zoom_it();
}


/* Update a couple of lines of text display */
static
wwrefresh(s, x, y, w, lines, color,skiplines)
char *s;
int x,y,w,lines;
int color;
int skiplines;
{
char buf[256];
int dy;
int erase_w;

dy = font_cel_height(usr_font);
erase_w = render_form->w - x;	/* seem to need to erase to end to be sure
								because unsure about length of mac fonts */
lines += skiplines;
while (--lines >= 0)
	{
	if (s != NULL)
		s = wwnext_line(usr_font, s, w, buf, 0);
	if (--skiplines < 0)
		{
		blit8(erase_w, font_hgt, 
			x, y, uf.p, uf.bpr, 
			x, y, render_form->p, render_form->bpr);
		if (s != NULL)
			justify_line(render_form, usr_font,buf,x,y,w,color,
			a1blit,sblack, 0);
		}
	y += dy;
	}
return;
}

/* Redraw all text in window */
static
refresh_win()
{
wwrefresh(wstart, vs.twin_x, vs.twin_y, 
	vs.twin_w, text_lines_visible, vs.ccolor, 0);
}


/* save out text buffer to temp file */
static
save_temp_text()
{
save_text(text_name);
}




/* get pixel location of cursor.  Return result in pixelx, pixely, and
   pixelw.  Set up a bunch of variables to track current character and
   line position. */
static
cursor_to_pixel()
{
char buf[256];
char *np, *p;
int xoff;
int i;
char *tend;

clip_cursorp();
pixely = vs.twin_y;
cstart = text_buf + vs.tcursor_p;
tend = text_buf + text_size;
lstart = wstart;
llstart = lwstart;
for (twypos=0;twypos<text_lines_visible;twypos++)
	{
	np = nextline(lstart);
	if (np == NULL || np > cstart || 
		(np == tend) && (tend[-1] != '\n') )
		{
		break;
		}
	llstart = lstart;
	lstart = np;
	pixely += font_dy;
	}
twxpos = cstart - lstart;
pixelx = vs.twin_x + fnstring_width(usr_font, lstart, twxpos);
if (vs.tcursor_p == text_size)
	{
	catend = 1;
	pixelw = fstring_width(usr_font, cst_space);
	}
else
	{
	catend = 0;
	pixelw = fnstring_width(usr_font, cstart, 1);
	}
}


static
show_text_cursor()
{
cursor_to_pixel();
xorblock(render_form->p, pixelx, pixely, pixelw, font_hgt, text_cursor_color);
}

static
hide_text_cursor()
{
xorblock(render_form->p, pixelx, pixely, pixelw, font_hgt, text_cursor_color);
}

static
find_wstart()
{
if (vs.text_yoff == 0)
	{
	wstart = text_buf;
	lwstart = NULL;
	}
else
	{
	lwstart = seek_line(text_buf, vs.text_yoff-1);
	wstart = seek_line(lwstart, 1);
	}
}

static
init_twin()
{
int h;
char *l;
int x;

/* if don't have a text_buf make one */
if (text_buf == NULL)
    {
    if ((text_buf = begmem(DTSIZE)) == NULL)
        return(0);
	vs.text_yoff = vs.tcursor_p = vs.tcursor_y = 0;
    text_size = 0;
    text_buf[0] = 0;
    text_alloc = DTSIZE;
    }
/* calculate some constants */
font_hgt = usr_font->frm_hgt;
font_dy = font_cel_height(usr_font);
font_dx = widest_char(usr_font);
if (font_dx < 0 || font_dx >= XMAX/2)
	font_dx = CH_WIDTH;
/* make sure window is big enough for one character or word wrap will hang...*/
if (vs.twin_h < font_dy)
	vs.twin_h = font_dy;
if (vs.twin_w < font_dx)
	vs.twin_w = font_dx;
font_lo = usr_font->ADE_lo;
font_hi = usr_font->ADE_hi;
/* calculate how many lines visible */
h = vs.twin_h;
text_lines_visible = 0;
if (vs.ccolor == 0)
	text_cursor_color = 255;
else
	text_cursor_color = vs.ccolor;
for (;;)
    {
    if (h < font_hgt)
        break;
    text_lines_visible++;
    h -= font_dy;
    }
find_wstart();
slide_up();
slide_down();
dtextcel();
return(1);
}

/* return 1 if cursor is now below text window */
static
past_window()
{
char *p, *endp;
int i;
char buf[256];

p = wstart;
endp = text_buf + vs.tcursor_p;
for (i=0;i<text_lines_visible;i++)
	{
	p = nextline(p);
	if (p == NULL || p >= endp)
		{
		return(0);
		}
	}
return(1);
}

/* if cursor position past end of window, slide it down. */
static
slide_down()
{
int slid = 0;

clip_cursorp();
while (past_window())
	{
	vs.text_yoff++;
	wstart = nextline(wstart);
	slid = 1;
	}
if (slid)
	find_wstart();
}

/* if cursor position before start of window slide it up */
static
slide_up()
{
clip_cursorp();
if (wstart == NULL || vs.tcursor_p < wstart - text_buf)
	{
	vs.text_yoff = 0;
	find_wstart();
	}
}

/* move cursor to closest x position possible in line p */
static 
xseek(p, x)
char *p;
int x;
{
char *pp;

if ((pp = nextline(p)) != NULL)
	{
	if (x < pp - p)	/* if enough characters in line hit it exactly */
		{
		vs.tcursor_p = p + x - text_buf;
		}
	else				/* else position cursor at end of line */
		{
		vs.tcursor_p = pp - text_buf - 1;
		}
	}
else	/* if on last line position at end of file and let clip_cursorp clean
           up any overshoot. */
	{
	vs.tcursor_p = p - text_buf + x;
	clip_cursorp();
	}
}

/* Move cursor position to close as it can find to x,y offset from p */
static
xyseek(p, x, y)
char *p;
int x, y;
{
char *np;

while (--y >= 0)
	{
	if ((np = nextline(p)) == NULL)
		break;
	p = np;
	}
xseek(p, x);
}

/* refresh cursor line to bottom of window.  If cursor line bottom of
   window scroll down too. */
static
update_end(nline)
char *nline;
{
if (nline != NULL && 
	twypos >= text_lines_visible - 1 &&
	nline - text_buf <= vs.tcursor_p)
	{
	scroll_down();
	refresh_win();
	}
else
	{
	wwrefresh(wstart,vs.twin_x,vs.twin_y,vs.twin_w,
		text_lines_visible - twypos, vs.ccolor,twypos);
	}
}

/* Called when characters inserted or deleted. Tries to minimize lines
   that have to be redrawn. */
static
update_win(extracs)
int extracs;	/* # of characters added or subtracted at cursor position */
{
char *new_lstart, *new_nlstart;
int count;

if (text_lines_visible == 1)	/* only one line, just make sure it's one with
									cursor in it and redraw */
	{
	if (llstart == NULL)	/* 1st line of text */
		{
		new_lstart = lstart;
		}
	else
		{
		new_lstart = nextline(llstart);
		}
	if (vs.tcursor_p < new_lstart - text_buf)	/* prev line? */
		{
		scroll_up();
		}
	else
		{
		/* check and see if gone on to next line */
		cursor_to_pixel();
		if (twypos >= 1)
			{
			scroll_down();
			}
		}
	refresh_win();
	}
else		/* more than one line, figure out which ones to update */
	{
	new_lstart = nextline(llstart);
	new_nlstart = nextline(new_lstart);
	/* check if effected previous line somehow... */
	if (llstart != NULL && new_lstart != lstart)
		{
		if (twypos == 0)	/* and previous line is outside window */
			{
			scroll_up();	
			refresh_win();	/* update everyone and split */
			return;
			}
		else				/* previous line in window but needs update */
			{
			wwrefresh(wstart, vs.twin_x, vs.twin_y, 
				vs.twin_w, 1, vs.ccolor, twypos-1);
			}
		}
	if (new_nlstart == NULL && nlstart == NULL) /* typing at EOF */
		{
		wwrefresh(wstart, vs.twin_x,vs.twin_y,vs.twin_w,1,vs.ccolor,twypos);
		}
	else if (new_nlstart == NULL || nlstart == NULL)
											/* update next line near EOF */
		{
		update_end(new_nlstart);
		}
	else if (new_nlstart != nlstart+extracs)	/* next line effected... */
		{
		update_end(new_nlstart);
		}
	else	/* not at EOF, but doesn't effect next line either.  normal case */
		{
		wwrefresh(wstart,vs.twin_x,vs.twin_y,vs.twin_w,1,vs.ccolor,twypos);
		}
	}
}

static
scroll_up()
{
if (vs.text_yoff > 0)
	{
	vs.text_yoff -= 1;
	find_wstart();
	}
}


static
scroll_down()
{
int owstart;

vs.text_yoff += 1;
for (;;)		/* this loop handles going past bottom... */
	{
	find_wstart();
	if (wstart != NULL)
		break;
	vs.text_yoff -= 1;
	}
}


static
insert_char(c)
char c;
{
if (text_size < text_alloc-1)
	{
	nlstart = nextline(lstart);
	back_copy_bytes(text_buf+vs.tcursor_p, text_buf+vs.tcursor_p+1,
		text_size - vs.tcursor_p+1);
	text_buf[vs.tcursor_p] = c;
	vs.tcursor_p += 1;
	text_size += 1;
	update_win(1);
	}
}

static
delete_char()
{
int count;

if (text_size > 0)
	{
	count = text_size - vs.tcursor_p;
	if (count != 0)
		{
		nlstart = nextline(lstart);
		copy_bytes(text_buf+vs.tcursor_p+1, text_buf+vs.tcursor_p,
			count);
		text_size -= 1;
		update_win(-1);
		}
	}
}

static
text_edit()
{
int i;
char *p, *pp;

if (!init_twin())
	return(0);
for (;;)
    {
	rub_twin();
    show_text_cursor();
	zoom_it();
    wait_click();
    hide_text_cursor();
	zoom_it();
	if (RJSTDN)
		break;
    if (key_hit)
        {
        switch (key_in&0xff)
            {
            case 0:
                switch (key_in)
                    {
                    case PAGEUP:
						vs.text_yoff -= text_lines_visible;
						if (vs.text_yoff < 0)
							vs.text_yoff = 0;
						find_wstart();
						refresh_win();
						xyseek(wstart, twxpos, twypos);
                        break;
                    case PAGEDN:
						if (seek_line(wstart, text_lines_visible) != NULL)
							{
							vs.text_yoff += text_lines_visible-1;
							find_wstart();
							refresh_win();
							xyseek(wstart, twxpos, twypos);
							}
						else
							{
							vs.tcursor_p = text_size;
							}
                        break;
                    case DELKEY:
						delete_char();
                        break;
                    case ENDKEY:
						xseek(lstart, 10000);
                        break;
                    case HOMEKEY:
						xseek(lstart, 0);
                        break;
                    case LARROW:
						if (vs.tcursor_p > 0)
							{
							vs.tcursor_p-=1;
							if (vs.tcursor_p < wstart-text_buf)
								{
								scroll_up();
								refresh_win();
								}
							}
                        break;
                    case RARROW:
						if (vs.tcursor_p < text_size)
							{
							vs.tcursor_p+=1;
							if (past_window())
								{
								scroll_down();
								refresh_win();
								}
							}
                        break;
                    case UARROW:
						if (twypos <= 0)
							{
							if (wstart > text_buf)
								{
								scroll_up();
								refresh_win();
								}
							xseek(wstart, twxpos);
							}
						else
							{
							p = seek_line(wstart, twypos-1);
							xseek(p, twxpos);
							}
                        break;
                    case DARROW:
						if ((p = nextline(lstart)) != NULL)
							{
							if (twypos >= text_lines_visible-1)
								{
								scroll_down();
								refresh_win();
								}
							xseek(p, twxpos);
							}
                        break;
                    }
                break;
            case '\b':	/* backspace */
				if (vs.tcursor_p > 0)
					{
					vs.tcursor_p -= 1;
					delete_char();
					}
                break;
			case '\t':	/* expand tab to 4 spaces */
				i = 4;
				while (--i >= 0)
					{
	                insert_char(' ');
					}
                break;
            case '\r':
				insert_char('\n');
				break;
            default:
				insert_char(key_in);
                break;
            }
		zoom_it();
        }
    else if (PJSTDN)
		{
		save_temp_text();
		if (in_twin(grid_x,grid_y) )
			{
			move_twin();
			}
		else
			{
			define_twin();
			}
		if (!init_twin())
			return(0);
		}
    }
return( save_temp_text() );
}




static
in_twin(x,y)
int x,y;
{
if ((x -= vs.twin_x) < 0)
	return(0);
if ((y -= vs.twin_y) < 0)
	return(0);
if (x >= vs.twin_w)
	return(0);
if (y >= vs.twin_h)
	return(0);
return(1);
}

static
undraw_twin()
{
#ifdef SHOULDWORKBUTDOESNT
blit8(vs.twin_w+2, vs.twin_h+2+font_hgt, 
	vs.twin_x-1, vs.twin_y-1, uf.p, uf.bpr,
	vs.twin_x-1, vs.twin_y-1, render_form->p, render_form->bpr);
#endif /* SHOULDWORKBUTDOESNT */
copy_form(&uf, render_form);
}

static
define_twin()
{
unundo();
zoom_it();
if (cut_out())
	{
	swap_box();
	vs.twin_x = x_0;
	vs.twin_y = y_0;
	vs.twin_w = x_1 - x_0 + 1;
	vs.twin_h = y_1 - y_0 + 1;
	}
}

static
move_twin()
{
int ix, iy;
int lx, ly;

ix = vs.twin_x;
iy = vs.twin_y;
undraw_twin();
for (;;)
	{
	lx = grid_x;
	ly = grid_y;
	dtextcel();
	wait_input();
	undraw_twin();
	vs.twin_x += grid_x - lx;
	vs.twin_y += grid_y - ly;
	if (PJSTDN)
		{
		break;
		}
	if (RJSTDN || key_hit)
		{
		vs.twin_x = ix;
		vs.twin_y = iy;
		break;
		}
	}
}

/* Edit existing text in same colors we use for menus over a blank screen.
   Don't paste the text. */
qedit_text()
{
int oc;

hide_mp();
unzoom();
if ( push_screen())
	{
	color_form(render_form,sblack);
	save_undo();
	oc = vs.ccolor;
	vs.ccolor = swhite;
	if (jexists(text_name))
		load_text(text_name);
	text_edit();
	free_text();
	vs.ccolor = oc;
	pop_screen();
	save_undo();
	}
rezoom();
draw_mp();
}

/* Edit existing text and paste a copy on screen */
qplace_text()
{
qpwt(1);
}

/* Edit existing text and optionally past a copy on screen.  */
qpwt(paste)
int paste;
{
hide_mp();
save_undo();
if (jexists(text_name))
	load_text(text_name);
text_edit();
unundo();
if (paste)
	rendr_twin();
free_text();
draw_mp();
}

/* Define a box and then let 'em type in it.  When done optionally
   paste text to screen. */
ttool(paste)
int paste;
{
if (!pti_input())
	return;
save_undo();
/* define a new area for the text */
if (rs_box())
	{
	/* clear out old text */
	gentle_freemem(text_buf);
	text_buf = NULL;
	swap_box();
	hide_mp();
	grab_usr_font();
	vs.twin_x = x_0;
	vs.twin_y = y_0;
	vs.twin_w = x_1 - x_0 + 1;
	vs.twin_h = y_1 - y_0 + 1;
	text_edit();
	unundo();
	if (paste)
		{
		rendr_twin();
		free_cfont();
		}
	free_text();
	draw_mp();
	}
}

/* A pentool for text.  On pendown make a box, and if this works out
   then let them type in it.  When done paste text to screen */
text_tool()
{
ttool(1);
}



/* bring up file requestor to load up a new font */
qfont_text()
{
char odrawer[80];
char *name;

strcpy(odrawer, vs.drawer);
change_dir(vconfg.font_drawer);
if ((name = get_filename(textedit_101 /* "Select a Font" */, ".FNT"))!= NULL)
	{
	if (load_install_font(name))
		{
		remove_suffix(vs.file);
		vs.file[8] = 0;	/* just for safety */
		strcpy(vs.fonts[0], vs.file);
		if (strcmp(vconfg.font_drawer, vs.drawer) != 0)
			{
			strcpy(vconfg.font_drawer, vs.drawer);
			rewrite_config();
			}
		}
	}
change_dir(odrawer);
}

