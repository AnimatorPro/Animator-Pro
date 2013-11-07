/* textbox.c - generate a 'continue' alert or a 'yes/no' dialog from
   a couple of C strings. */

#include "jimk.h"
#include "flicmenu.h"
#include "textbox.str"

/* This file does assume non-proportional text.  Here's some constants
   that tell us how to format around the font */

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 7
#define LINE_HEIGHT (CHAR_HEIGHT+2)
#define BASELINE	6

/* This is the minimum border between the outside of the requestor box
   and the text inside */
#define BORDER	8

/* The width and height of the Continue "gadget" box */
#define CONT_WIDTH (10*CHAR_WIDTH)
#define CONT_HEIGHT 16


/* a structure giving us the size and position of the entire requestor */
struct rectangle box, yes_box, no_box;
static WORD *behind;


/* find_text_box(names) -
      Pass this an array of names terminated by a NULL, and this baby
	  will figure out how wide and how tall a requester would have to
	  be to hold it.   Since live video-holes have to be
	  on WORD boundaries, and vertically on even lines (I don't know why
	  on even lines), this does a little padding to the width and height
	  to insure this.  Then it calculates the x and y offsets to
	  center the thing.  The results of all this are in the static
	  variable "box", and also in the variable xborder, which tells
	  you where to start your text on the left side so as to be
	  centered. */
static
find_text_box(names, button_height)
register char *names[];	 /* NULL terminated array of strings */
WORD button_height;
{
unsigned width, height;
unsigned this_width;
char *name;

width = 0;
height = button_height+LINE_HEIGHT;
while ((name = *names++) != NULL) 
	{
	height += LINE_HEIGHT;
	this_width = CHAR_WIDTH*strlen(name);
	if (this_width > width)
		width = this_width;
	}
if (width < 12*CHAR_WIDTH)
	width = 12*CHAR_WIDTH;
height += 2*BORDER;
width += 2*BORDER;	/* give us some room for the border etc */
box.MinX = (320 - width)>>1;
#ifdef ST
box.MinX = ((box.MinX+7)&0xfff8);	/* pad it to a byte boundary */
#endif /* ST */ 
box.MinY = (200-height)>>1;
if ((behind = askmem(Raster_block(width, height))) == NULL)
	return(0);
blit8(width, height, box.MinX, box.MinY, vf.p, vf.bpr,
	0, 0, behind, Raster_line(width));
box.MaxX = box.MinX+width;
box.MaxY = box.MinY+height;
return(1);
}


static
undraw_tbox()
{
unsigned w;

w = box.MaxX-box.MinX;
if (behind != NULL)
	{
	blit8(w, box.MaxY-box.MinY, 0, 0, behind, Raster_line(w),
		box.MinX, box.MinY, vf.p, vf.bpr);
	freemem(behind);
	}
}

static
d_top_lines_of_box(names, button_height)
char *names[];
WORD button_height;
{
WORD xoff, yoff;
char *name;

find_colors();
if (!find_text_box(names, button_height))
	return(0);
/* draw the box around it all */
colblock(swhite, box.MinX+1, box.MinY+1, box.MaxX-1, box.MaxY-1);
draw_white_box(&box);

xoff = box.MinX + BORDER;
yoff = box.MinY + BORDER;
while ((name = *names++) != NULL) 
	{
	gtext(name, xoff, yoff, sblack);
	yoff += LINE_HEIGHT;
	}
return (yoff);
}


static
draw_white_box(b)
struct rectangle *b;
{
draw_frame(sgrey, b->MinX, b->MinY, b->MaxX, b->MaxY);
}

/* draw_yes_no_box()
*/
static
draw_yes_no_box(names, yes, no)
char *names[];
char *yes;
char *no;
{
WORD yesno_size;
WORD yes_size;
WORD no_size;
WORD box_size;
WORD x;
WORD xoff, yoff;


yoff = d_top_lines_of_box(names,CONT_HEIGHT);
if (yoff == 0)
	return(0);
yoff += LINE_HEIGHT/2 + BASELINE;

yesno_size = 4;
yes_size = strlen(yes);
if (yes_size > yesno_size)
	yesno_size = yes_size;
no_size = strlen(no);
if (no_size > yesno_size)
	yesno_size  = no_size;
yesno_size += 2;

box_size = (unsigned)(box.MaxX - box.MinX)/CHAR_WIDTH;
box_size -= 2*yesno_size;
xoff = box.MinX + CHAR_WIDTH*((box_size+1)/3);
xoff = (xoff+4)&0xfff8;	/* pad to a byte */
yes_box.MinX = xoff;
yes_box.MaxX = xoff + yesno_size*CHAR_WIDTH;
yes_box.MinY = yoff;
yes_box.MaxY = yoff + CONT_HEIGHT;
box_diag_corner(xoff,yoff,yesno_size*CHAR_WIDTH,CONT_HEIGHT,sgrey);
gtext(yes, 
	xoff + (((yesno_size - yes_size)*CHAR_WIDTH)>>1), 
	yoff + ((CONT_HEIGHT-CHAR_HEIGHT)>>1), 
	sblack);
xoff = box.MinX + CHAR_WIDTH*(yesno_size + (2*box_size)/3);
xoff = (xoff+4)&0xfff8;	/* pad to a byte */
no_box.MinX = xoff;
no_box.MaxX = xoff + yesno_size*CHAR_WIDTH;
no_box.MinY = yoff;
no_box.MaxY = yoff + CONT_HEIGHT;
box_diag_corner(xoff,yoff,yesno_size*CHAR_WIDTH,CONT_HEIGHT,sgrey);
gtext(no, 
	xoff + (((yesno_size - no_size)*CHAR_WIDTH)>>1), 
	yoff + ((CONT_HEIGHT-CHAR_HEIGHT)>>1), 
	sblack);
return(1);
}


/* draw_continue_box()
	given a NULL-terminated array of names, and a prompt to put in a box
	to tell it to go away, draw a requester, and cut out a hole so the
	video goes around it */
static
draw_continue_box(names, continu)
char *names[];
char *continu;
{
WORD xoff, yoff;
WORD continu_size;

yoff = d_top_lines_of_box(names, CONT_HEIGHT);
if (yoff == 0)
	return(0);
yoff += LINE_HEIGHT/2 + BASELINE;
xoff = box.MinX + BORDER;
xoff += (box.MaxX - box.MinX - 2*BORDER - CONT_WIDTH)>>1;
box_diag_corner(xoff,yoff,CONT_WIDTH,CONT_HEIGHT,sgrey);
continu_size = strlen(continu);
yoff += ((CONT_HEIGHT-CHAR_HEIGHT)>>1);
xoff += (CONT_WIDTH-CHAR_WIDTH*continu_size)>>1;
gtext(continu, xoff, yoff, sblack);
return(1);
}


continu_line(line)
char *line;
{
char *lines[2];

lines[0] = line;
lines[1] = NULL;
continu_box(lines);
}


continu_box(lines)
char *lines[];
{
if (!draw_continue_box(lines, textbox_100 /* "Continue" */))
	return(-1);
wait_click();
undraw_tbox();
wait_penup();
return(1);
}

static
inbox(b)
struct rectangle *b;
{
if (uzx >= b->MinX && uzx <= b->MaxX && uzy >= b->MinY &&
	uzy <= b->MaxY)
	return(1);
return(0);
}

static
poll_yes_no()
{
unsigned char c;

for (;;)
	{
	wait_click();
	if (PJSTDN)
		{
		if (inbox(&yes_box))
			return(1);
		else if (inbox(&no_box))
			return(0);
		}
	if (RJSTDN)
		return(0);
	if (key_hit)
		{
		c = key_in;
		if (c == 'n' || c == 'N')
			return(0);
		if (c == 'y' || c == 'Y')
			return(1);
		}
	}
}

yes_no_box(lines)
char *lines[];
{
WORD answer;

if (!draw_yes_no_box(lines, textbox_101 /* "Yes" */, textbox_102 /* "No" */))
	return(-1);
answer = poll_yes_no();
undraw_tbox();
wait_penup();
return(answer);
}

yes_no_line(line)
char *line;
{
char *buf[2];

buf[0] = line;
buf[1] = NULL;
yes_no_box(buf);
}

