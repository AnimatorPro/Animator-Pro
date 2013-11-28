
/* rfont.c - raster font drawer.  Takes a font in our hybrid GEM/Macintosh
   format and displays it, or tells you how big a character or string is. */

#include <ctype.h>
#include "jimk.h"
#include "blit8_.h"
#include "gemfont.h"
#include "rfont.h"

extern UBYTE sixhi_data[];
struct font_hdr *usr_font = &sixhi_font; 

void
systext(s, x, y, color,tblit,bcolor)
register const char *s;
int x, y, color;
Vector tblit;	/* blit vector */
int bcolor;
{
register char c;

y+=1;
while ((c = *s++) != 0)
	{
	c = toupper(c);

	(*tblit)(6, 6, 6*c, 0, sixhi_data, 
		192, x, y, vf.p, 320, color, bcolor);
	x+=6;
	}
}

void
systext_clip(int width, const char *str, int x, int y, int col)
{
	char c;

	y++;
	while ((c = *str++) != '\0') {
		int sw = (width <= 6) ? width : 6;
		c = toupper(c);

		a1blit(sw, 6, 6*c, 0, sixhi_data, 192, x, y, vf.p, 320, col);

		x += 6;
		width -= sw;
		if (width <= 0)
			break;
	}
}

void
systext_keepcase(const char *s, int x, int y, int fg, int bg)
{
	char c;

	y++;
	while ((c = *s++) != '\0') {
		a2blit(6, 6, 6*c, 0, sixhi_data, 192, x, y, vf.p, 320, fg, bg);
		x += 6;
	}
}

typedef union
	{
	  WORD theInt;
	  char bytes[2];
	} myInt;

void
gftext(Video_form *screen, struct font_hdr *f, const unsigned char *s,
		int x, int y, int color, Vector tblit, int bcolor)
{
const unsigned char *ss;
unsigned char c, lo, hi;
int sx, imageWid;
WORD *off, wd, ht, *data;
myInt *OWtab, *iPtr;
int missChar;
PLANEPTR p;
int font_type;

p = screen->p;
lo = f->ADE_lo;
hi = f->ADE_hi;
off = f->ch_ofst;
wd = f->frm_wdt;
ht = f->frm_hgt,
data = f->fnt_dta;
OWtab= (myInt *)(f->hz_ofst);
font_type = f->id;

while ((c = *s++)!=0)
	{
	c -= lo;
	/* Mac prop font && its a missing char */
	if (font_type == MPROP && (*(OWtab+c)).theInt == -1) 
		{            
		c=hi-lo;                      /* last char is set */
		missChar=1;
		sx = off[c+1];
		imageWid= f->frm_wdt*8 - sx;  /* sort of a kludge */
		}
	else 
		{
		missChar=0;
		sx = off[c];
		imageWid = off[c+1]-sx;
		}
	(*tblit)(imageWid, ht, sx, 0, data, wd, x, y, p, 
		BPR, color,bcolor);
	switch (font_type)
		{
		case STPROP:
			x += imageWid;
			break;
		case MFIXED:
			x += f->wchr_wdt;          
			break;
		case MPROP:
			iPtr=OWtab+c;  
			if (!missChar)
				/* -1 means its a missing character */
				{
				x += (int)((*iPtr).bytes[1]);
				ss=s;
				if ((c=*(ss++)) != 0)
					/* look to next char to determine amt to change x */
					{
					c-= lo;
					iPtr=OWtab+c;
					/* subtract kern Of Next char */
					/* f->rgt_ofst is neg of Mac maxKern value */
					if ((*iPtr).theInt!=-1)
					   x += (int)((*iPtr).bytes[0])+ f->rgt_ofst;  
					}           
			   }
			else /* display the non print char */
				x+=imageWid;
			break;
		}
	}
}

int
fchar_width(struct font_hdr *f, const char *s)
{
char c;
char *offsets;
int width;
int t;

c = *s++;
if (c > f->ADE_hi)
	c = ' ';
c -= f->ADE_lo;
if (c < 0)
	c = 0;
switch (f->id)
	{
	case MFIXED:
		return(f->wchr_wdt);
	case STPROP:
		return(f->ch_ofst[c+1] - f->ch_ofst[c]);
	case MPROP:
		offsets = f->hz_ofst+c*2;
		if (offsets[0] == -1 && offsets[1] == -1)	/* missing char */
			{
			t = f->ADE_hi - f->ADE_lo;
			return( f->frm_wdt*8 - f->ch_ofst[t+1]);
			}
		else
			{
			width = offsets[1];
			if ((c = *s++) != 0)
				{
				c -= f->ADE_lo;
				offsets = f->hz_ofst+c*2;
				width += offsets[0] + f->rgt_ofst;
				}
			return(width);
			}
	}
}

long
fstring_width(struct font_hdr *f, const char *s)
{
long acc = 0;

while (*s != 0)
	{
	acc += fchar_width(f, s);
	s++;
	}
return(acc);
}

int
font_cel_height(struct font_hdr *f)
{
int dy;

dy = f->frm_hgt;
return(dy + ((dy+3)>>2) );
}
