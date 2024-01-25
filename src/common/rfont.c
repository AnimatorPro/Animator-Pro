/* rfont.c - Raster Font stuff, draws text based on a blit and a font
   in a format that some day may approach Ventura Publisher, but currently
   looks much more like GEM on the ST with some Mac-like mutations.
   Also data for the Aegis Animator font with proportional stuff stripped.
   (This font is just a handy example of the format, unused itself.)

   There are 2 big routines - systext() - for all text rendered by
   menuing system -  and gftext() for text rendered onto user image. */
#include <ctype.h>
#include <string.h>
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
int c, lo, hi;
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
	if (c > hi)
		c = ' ';
	c -= lo;
	if (c < 0)
		c = ' ' - lo;
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
fchar_width(struct font_hdr *f, const unsigned char *s)
{
int c;
char *offsets;
int width;
int t;

c = *s++;
if (c > f->ADE_hi)
	c = ' ';
c -= f->ADE_lo;
if (c < 0)
	c = ' ' - f->ADE_lo;
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
fnstring_width(struct font_hdr *f, const unsigned char *s, int n)
{
long acc = 0;

while (--n >= 0)
	{
	acc += fchar_width(f, s);
	s++;
	}
return(acc);
}

long
fstring_width(struct font_hdr *f, const unsigned char *s)
{
return(fnstring_width(f, s, strlen(s)));
}

int
widest_char(struct font_hdr *f)
{
unsigned char buf[2];
int i;
int c;
int widest = 1;
int w;

c = f->ADE_lo;
i = f->ADE_hi - c;
buf[1] = 0;
while (--i >= 0)
	{
	buf[0] = c++;
	w = fchar_width(f, buf);
	if (w > widest)
		widest = w;
	}
return(widest);
}

int
font_cel_height(struct font_hdr *f)
{
int dy;

dy = f->frm_hgt;
return(dy + ((dy+3)>>2) );
}
