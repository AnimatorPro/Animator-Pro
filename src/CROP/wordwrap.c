
/* wordwrap.c - given a font and a window of a certain size,
   figure out how many words we can fit in a line.  Help titling system
   justify the text. */


#include "jimk.h"
#include "gemfont.h"
#include "text.h"

char *
wwnext_word(s,buf,f,w,caller_ww)
char *s;
char *buf;
struct font_hdr *f;
int w;
int *caller_ww;
{
char c;
int i;
int ww,cw,sw;        /* word width, char width, space width */

c = *s++;
if (c == 0)
	{
	*caller_ww = 0;
	return(NULL);
	}
if (c == '\n' || c== '\r')
	{
	*buf++ = c;
	*buf++ = 0;
	*caller_ww = 0;
	return(s);
	}
ww = 0;
sw = fchar_width(f," ");
for (;;)
	{
	if (c == '\n' || c == '\r' || c == ' ' || c == '\t' || c == 0)
		break;
	else
		{
		if ((ww += fchar_width(f,s-1)) > w)
			{
			ww -= fchar_width(f,s-1);
			goto OUT;
			}
		*buf++ = c;
		}
	c = *s++;
	}
for (;;)        /* copy leading blanks and expand tabs */
	{
	if (c == ' ')
		{
		if ((ww += sw) > w)
			{
			ww -= sw;
			goto OUT;
			}
		*buf++ = c;
		}
	else if (c == '\t')
		{
		i = 4;
		while (--i >= 0)
			{
			if ((ww += sw) > w)
				{
				ww -= sw;
				s++;      /* skip over tab next time, tab may not fit on line*/
				goto OUT;
				}
			*buf++ = ' ';
			}
		}
	else
		break;
	c = *s++;
	}
OUT:
*buf++ = 0;
*caller_ww = ww;
return(s-1);
}

char *
wwnext_line(f, s, w, buf, sub_cr)
struct font_hdr *f;
char *s;
int w;
char *buf;
int sub_cr;
{
int lines;
int lw, ww;
char *ns;

if (s[0] == 0)
	return(NULL);
lw = 0;
for (;;)
	{
	if ((ns = wwnext_word(s,buf,f,w,&ww)) == NULL)
		break;
	if ( ww + lw > w)
		{
		buf[0] = 0;
		break;
		}
	if (buf[0] == '\n' || buf[0] == '\r')
		{
		s++;
		buf[0] = sub_cr;	/* text editor needs to keep hard cr's */
		buf[1] = 0;
		break;
		}
	buf += ns - s;
	s = ns;
	lw += ww;
	}
return(s);
}



#ifdef SLUFFED
/* count lines that word-wrap will take for a given string, font, and
   width */
wwcount_lines(f, s, w)
struct font_hdr *f;
char *s;
int w;
{
int lines;
int lw, ww;
char buf[256];

lines = 0;
for (;;)
	{
	if ((s = wwnext_line(f, s, w, buf, 0)) == NULL)
		break;
	lines++;
	}
return(lines);
}
#endif /* SLUFFED */


wwtext(screen, f, s, x, y, w, h, color,blit,skiplines, justify)
Video_form *screen;
struct font_hdr *f;
char *s;
int x,y,w,h;
int color;
Vector blit;
int skiplines;
int justify;
{
char buf[256];
int dy;

dy = font_cel_height(f);
for (;;)
	{
	if ((s = wwnext_line(f, s, w, buf, 0)) == NULL)
			break;
	if (--skiplines < 0)
		{
#ifdef SLUFFED
		justify_line(screen, f,buf,x,y,w,color,blit,sblack, justify);
#endif /* SLUFFED */
		gtext(buf, x, y, color);
		y += dy;
		h -= dy;
		if (h < f->frm_hgt)
				break;
		}
	}
return;
}

#ifdef SLUFFED
justify_line(screen, font, linebuf, x, y, w, color, blit, color2, just)
Video_form *screen;
struct font_hdr *font;
char *linebuf;
int x,y,w;
int color;
Vector blit;
int color2;
int just;
{
int sw;
int hardcr;
int i;
char c;

i = strlen(linebuf) - 1;
hardcr = linebuf[i] == '\n';
if (just != 0)
	{
	/* strip trailing white space */
	for (;;)
		{
		c = linebuf[i];
		if (!(c == ' ' || c == '\t' || c == '\n'))
			break;
		else
			linebuf[i] = 0;
		i -= 1;
		if (i < 0)
			return;	/* nothing much left... */
		}
	}
sw = w - fstring_width(font, linebuf);
switch (just)
	{
	case 0:	/* left justify */
		break;
	case 1: /* right justify */
		x += sw;
		break;
	case 2:	/* center justify */
		x += sw/2;
		break;
	case 3: /* fill line */
		if (!hardcr)
			{
			just_fill(screen, font, linebuf, x, y, color, blit, color2, sw);
			return;
			}
		break;
	}
gftext(screen, font, linebuf, x, y, color, blit, color2);
}

just_fill(screen, font, linebuf, x, y, color, blit, color2, xspace)
Video_form *screen;
struct font_hdr *font;
char *linebuf;
int x,y;
int color;
Vector blit;
int color2;
int xspace;
{
int chars, ichars, i;
static char cb[2];
int x1, lx1;

chars = strlen(linebuf);
ichars = chars-1;
if (ichars <= 0) /* let the small things go normally... */
	{
	gftext(screen, font, linebuf, x, y, color, blit, color2);
	return;
	}
lx1 = 0;
for (i=1; i<=chars; i++)
	{
	cb[0] = *linebuf;
	gftext(screen, font, cb, x, y, color, blit, color2);
	x1 = rscale_by(xspace, i, ichars);
	x += fchar_width(font, linebuf) + x1 - lx1;
	lx1 = x1;
	linebuf++;
	}
}


empty_list(list)
struct textlist *list;
{
list->first = (struct textline *)(&list->stopper);
list->end = (struct textline *)(&list->first);
list->stopper = NULL;
}

insert_after(last, el)
struct textline *last, *el;
{
struct textline *next;

next = last->next;
last->next = next->last = el;
el->next = next;
el->last = last;
}

unlink_node(el)
struct textline *el;
{
el->next->last = el->last;
el->last->next = el->next;
}

free_node(el)
struct textline *el;
{
freemem(el->line);
freemem(el);
}

/* substitue list for el */
sub_list_el(list, el)
struct textlist *list;
struct textline *el;
{
struct textline *last;
struct textline *ll;

ll = list->first;
if (ll->next == NULL)        /* empty list ... just delete this element */
        {
        unlink_node(el);
        free_node(el);
        return;
        }
ll->last = el->last;
el->last->next = ll;
ll = list->end;
ll->next = el->next;
el->next->last = ll;
free_node(el);
}


struct textline *
alloc_textline(words)
char *words;
{
struct textline *line;

if ((line = askmem(sizeof(*line) )) == NULL)
        return(NULL);
if ((line->line = clone_string(words)) == NULL)
        {
        freemem(line);
        return(NULL);
        }
return(line);
}

add_line(list, words)
struct textlist *list;
char *words;
{
struct textline *newel;

if ((newel = alloc_textline(words)) == NULL)
	{
	outta_memory();
	return(0);
	}
insert_after(list->end, newel);
return(1);
}

#ifdef SLUFFED
print_textback(list)
struct textline *list;
{
struct textline *last;

while ((last = list->last) != NULL)
        {
        puts(list->line);
        list = last;
        }
}
#endif /* SLUFFED */

free_textlist(list)
struct textline *list;
{
struct textline *next;

while ((next = list->next) != NULL)
        {
        freemem(list->line);
        freemem(list);
        list = next;
        }
}

#ifdef SLUFFED
print_textlist(list)
struct textline *list;
{
struct textline *next;

while ((next = list->next) != NULL)
        {
        puts(list->line);
        list = next;
        }
}
#endif /* SLUFFED */


wwlines(f, s, w, list)
struct font_hdr *f;
char *s;
int w;
struct textlist *list;
{
char line_buf[256];

empty_list(list);
for (;;)
	{
	if ((s = wwnext_line(f, s, w, line_buf, '\n')) == NULL)
		break;
	add_line(list, line_buf);
	}
}

long
unch(buf, size, ch)
register char *buf;
int size;
char ch;
{
register char *out;
long newsize = 0;
char c;

out = buf;
while (--size >= 0)
	{
	c = *buf++;
	if (c != ch)
		{
		*out++ = c;
		newsize++;
		}
	}
return(newsize);
}


load_text(name)
char *name;
{
int f;

if ((f = jopen(name, 0)) == 0)
	return(0);
free_text();
text_alloc = DTSIZE;
if ((text_buf = lbegmem((long)text_alloc)) == NULL)
	{
	jclose(f);
	return(0);
	}
text_size = jread(f, text_buf, (long)text_alloc-1);
text_size = unch(text_buf, text_size, '\r');
text_buf[text_size] = 0;
jclose(f);
return(1);
}

save_text(name)
char *name;
{
int f;

if (!text_buf)
	return(1);
if ((f = jcreate(name)) == 0)
	return(0);
if (jwrite(f, text_buf, (long)text_size) < text_size)
	{
	truncated(name);
	jclose(f);
	return(0);
	}
jclose(f);
return(1);
}
#endif /* SLUFFED */
