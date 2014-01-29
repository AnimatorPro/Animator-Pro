
#define VFONT_C

/* vfont.c  - dispatch from abstract font to Vpaint 1.0 type, Speedo,
	or whatever type of font */

#include <stdio.h>
#include <string.h>
#include "linklist.h"
#include "rastext.h"
#include "errcodes.h"
#include "gfx.h"

/* Glue to interpret the virtual font functions */
void close_vfont(Vfont *v)
{
	if (v != NULL && v->close_vfont != NULL)
	{
		v->close_vfont(v);
		v->close_vfont = NULL;
	}
}

int fendchar_width(Vfont *v, char *s)
/* returns width of char if placed at the end of a line Note: This WILL
 * include the extra spacing */
{
static char end_test[2] = " ";

	end_test[0] = *s;
	if(end_test[0] == '\t')
		return(v->tab_width);
	return(v->char_width(v,end_test));
}
int fchar_spacing(Vfont *v, char *s)
{
	if(*s == '\t')
		return(v->tab_width);
	return(v->char_width(v,s));
}
int fspace_width(Vfont *v, char *s)
{
	if(*s == '\t')
		return(v->tab_width);
	return(v->char_width(v,s));
}

#if SLUFFED
int fchar_width(Vfont *f, char *s)
/* width of char with out spacing */
{
	if(*s == '\t')
		return(f->tab_width);
	return(f->char_width(f,s)-f->spacing);
}
#endif /* SLUFFED */

Errcode fset_spacing(Vfont *f, SHORT spacing, SHORT leading)
/* if either spacing or leading is < 0 they will be ignored */
{
	if(spacing < 0)
		spacing = f->spacing;
	if(leading < 0)
		leading = f->leading;

	f->spacing = spacing;
	f->leading = leading;

	f->widest_char = f->widest_image + f->spacing;
	f->line_spacing = f->image_height + f->leading;
	return(Success);
}

Errcode fget_spacing(Vfont *f, SHORT *spacing, SHORT *leading)
{
*spacing = f->spacing;
*leading = f->leading;
return(Success);
}

int tallest_char(Vfont *v)
{
	return(v->image_height);
}

int font_cel_height(Vfont *v)
{
	return(v->line_spacing);
}

int widest_char(Vfont *v)
{
	return(Max(v->widest_image,v->widest_end));
}

int in_font(Vfont *v, int c)
{
	return(v->in_font(v,c));
}

Errcode fset_height(Vfont *v, SHORT height)
{
	if (v->scale_font)
		return v->scale_font(v,height);
	else
		return Err_null_ref;
}

Errcode fset_unzag(Vfont *v, Boolean unzag)
{
	if (v->change_unzag)
		return v->change_unzag(v, unzag);
	else
		return Err_null_ref;
}

Errcode gftext(Raster *rast,
			Vfont *f,
			char *s,
			int x,int y,
			Pixel color,Text_mode tmode,
			Pixel bcolor)
{
	/* clip for whole line at least, could process a line below but that isn't
	 * too bad for smaller test */

	if( (y + f->image_height) < 0 || y > rast->height)
		return(Success);
	return f->gftext(rast, f, (unsigned char *)s, x, y, color, tmode, bcolor);
}


/* functions based on virtual functs... */

int vfont_interleave_extra(Vfont *f,   /* the font */
						   char *s,    /* one char beyond the last char */
						   int count)  /* number of chars available before s */

/* leetle beet of messy code. Some fonts have overlapping chars. We must 
   check to see that no chars overlap to make a printed area longer
   than the end of the last char in the line or else! */

/* returns the number of pixels beyond the cumulative imbedded length in the
 * string that any previous characters may be responsible for. */
{
int widest;
int addlen;
int backlen;
int extra_len;
char end_str[2];

	if(count <= 0)
		return(0);

	widest = f->widest_image;
	backlen = f->char_width(f,--s);
	end_str[1] = 0;

	if(s[1]) /* if we are not null terminated we need to add it */
	{
		end_str[0] = *s;
		if((extra_len = f->char_width(f,end_str) - backlen) > 0)
			backlen += extra_len;
		else
			extra_len = 0;
	}
	else
		extra_len = 0;

	for(;;)
	{
		if(!(--count > 0 && backlen < widest))
			break;

		backlen += f->char_width(f,--s);
		end_str[0] = *s;
		addlen = f->char_width(f,end_str) - backlen;
		if(addlen > 0)
		{
			extra_len += addlen; /* char extends beyond the last char!! */
			backlen += addlen;
		}
	}
	return(extra_len);
}
static int count_tabs(char *s,int count)
{
char *maxc;

	maxc = s + count;
	count = 0;
	while(s < maxc)
	{
		if(*s++ == '\t')
			++count;
	}
	return(count);
}

long fnstring_width(Vfont *f,register char *s,int count)
/* note that this will NOT stop with a NULL character and will return 
 * the length to the end of the line it the string is NULL terminated */
{
long len;
char *end_char;

	if(f->flags & VFF_MONOSPACE)
	{
		if(!count)
			return(0);
		len = count_tabs(s,count);
		len = (((count - len)*f->widest_char) + (len * f->tab_width));
		if(s[count-1] != '\t')
			len -= f->spacing;
		return(len);
	}

	len = 0;
	end_char = s + count;

	while(s < end_char)
	{
		switch(*s)
		{
			case 0:
				count -= end_char - s;
				goto done;
			case '\t':
				len += f->tab_width;
				break;
			default:
				len += f->char_width(f, s);
				break;
		}
		s++;
	}

done:
	if(f->flags & VFF_XINTERLEAVE && *s == 0)
		len += vfont_interleave_extra(f,s,count);

	if(count && s[-1] != '\t')
		len -= f->spacing;
	return(len);
}

long fstring_width(Vfont *f,char *s)
{
	if(s == NULL)
		return(0);
	return(fnstring_width(f,s,strlen(s)));
}
static int linelen(char *str)
/* returns length to first newline or '0' */
{
char *pc = str;

	while((*pc) && (*pc != '\n'))
		++pc;
	return(pc - str);
}

long fline_width(Vfont *f,char *s)
{
	if(s == NULL)
		return(0);
	return(fnstring_width(f,s,linelen(s)));
}

int font_ycent_oset(Vfont *f,SHORT height)
/* given height of cel to place line in gives offset to top of vertically
 * centered (in height) text */
{
int iheight;

	iheight = f->image_height;
	/* ht/2 - frm_hgt/2 */
	return(((height - iheight) >> 1)+1);
}

int font_xcent_oset(Vfont *f,char *s, SHORT width)
/* given height of cel to place line in gives offset to top of vertically
 * centered (in height) text */
{
int iwidth;

	iwidth = fstring_width(f,s);
	return(((width - iwidth) >> 1)+1);
}

long widest_line(Vfont *f,char **lines, int lcount)
/* returns max text line width given array of pointers to text lines 
 * this assumes the strings are terminated by the first '0' or first '\n' */
{
LONG thiswid;
LONG width = 0;

	while(lcount-- > 0)
	{
		if((thiswid = fline_width(f,*lines)) > width)
			width = thiswid;
		++lines;
	}
	return(width);
}

int widest_name(Vfont *f, struct names *list)
/* Returns pixel width of widest name in list.
 */
{
int thiswid, width=0;

while (list != 0)
	{
	if ((thiswid = fstring_width(f, list->name)) > width)
		width = thiswid;
	list = list->next;
	}
return(width);
}

static int mono_width(Vfont *f, char *str)
/* Return width of a char in mono-spaced font. */
{
	(void)str;
	return(f->widest_char);
}

void scan_init_vfont(Vfont *f)
/* scans font characters to calculate font constants, and set flags 
 * used in initializing vfonts */
{
char str_end[3];
char str_imbed[3];
int ewid, iwid;
int maxiwid, maxewid;
int maxspace;

	if(f->flags & VFF_MONOSPACE)
	{
		f->char_width = mono_width;
		f->widest_char = f->widest_end = f->end_space = f->widest_image;
		return;
	}

	f->end_space = f->char_width(f," ");
	maxewid = maxiwid = 0;
	maxspace = 0;

	str_end[0] = ' ';
	str_end[1] = 1;
	str_end[2] = 0;

	str_imbed[2] = ' ';
	str_imbed[3] = 't';

	do
	{
		if(!(f->in_font(f,str_end[1])) || str_end[1] == '\t')
			continue;

		str_imbed[0] = str_end[1];
		if((ewid = fchar_spacing(f,&str_end[1])) > maxewid)
			maxewid = ewid;
		if((ewid = fchar_spacing(f,&str_end[0])) > maxspace)
			maxspace = ewid;
		if((iwid = fchar_spacing(f,&str_imbed[0])) > maxiwid)
			maxiwid = iwid;
		if(ewid != iwid)
			f->flags |= VFF_XINTERLEAVE;
	}
	while((++str_end[1]) != 0);

	f->widest_char = f->widest_image = maxiwid;
	f->widest_end = maxewid;
	if(f->end_space < 1)
		f->end_space = maxspace;
}


void render_mask_blit(UBYTE *mplane, SHORT mbpr,
					  SHORT mx, SHORT my,
					  void *drast, /* currently ignored uses vb.pencel */
					  SHORT rx, SHORT ry, USHORT width, USHORT height, ... );

/* A table of blits that correspond to various font-drawing modes. */
VFUNC blit_for_mode[] = {pj_mask1blit, pj_mask2blit, render_mask_blit};

unsigned char oem_to_ansi[] = 
/*
 * This table converts international characters from ANSI order to
 * IBM Ascii order.
 */
	{
	0000,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0xa4,
	0x10,0x11,0x12,0x13,0xb6,0xa7,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
	0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
	0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0xc7,0xfc,0xe9,0xe2,0xe4,0xe0,0xe5,0xe7,
	0xea,0xeb,0xe8,0xef,0xee,0xec,0xc4,0xc5,
	0xc9,0xe6,0xc6,0xf4,0xf6,0xf2,0xfb,0xf9,
	0xff,0xd6,0xdc,0xa2,0xa3,0xa5,0x50,0x83,
	0xe1,0xed,0xf3,0xfa,0xf1,0xd1,0xaa,0xba,
	0xbf,0x5f,0xac,0xbd,0xbc,0xa1,0xab,0xbb,
	0x5f,0x5f,0x5f,0xa6,0xa6,0xa6,0xa6,0x2b,
	0x2b,0xa6,0xa6,0x2b,0x2b,0x2b,0x2b,0x2b,
	0x2b,0x2d,0x2d,0x2b,0x2d,0x2b,0xa6,0xa6,
	0x2b,0x2b,0x2d,0x2d,0xa6,0x2d,0x2b,0x2d,
	0x2d,0x2d,0x2d,0x2b,0x2b,0x2b,0x2b,0x2b,
	0x2b,0x2b,0x2b,0x5f,0x5f,0xa6,0x5f,0x5f,
	0x5f,0xdf,0x5f,0xb6,0x5f,0x5f,0xb5,0x5f,
	0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,
	0x5f,0xb1,0x5f,0x5f,0x5f,0x5f,0xf7,0x5f,
	0xb0,0x95,0xb7,0x5f,0x6e,0xb2,0x5f,0x5f,
	};
