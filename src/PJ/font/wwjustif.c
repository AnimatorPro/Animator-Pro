#include <string.h>
#define WORDWRAP_INTERNALS
#include "imath.h"
#include "wordwrap.h"

static int just_fill(Raster *screen,
					 Vfont *font,
					 char *linebuf,
					 int chars,
					 int x,int y,
					 Pixel color,
					 Text_mode tmode,
					 Pixel color2,int xspace, int cxidx)

/* returns x position of char with index of cxidx, x if cxidx <= 0 */
{
int ichars, i;
static char cb[2];
int retx;
int x1, lx1;

	if(chars <= 1) /* let the small things go normally... */
	{
		if(screen)
			gftext(screen, font, linebuf, x, y, color, tmode, color2);
		return(x);
	}

	retx = x;
	ichars = chars-1;
	lx1 = 0;
	i = 0;
	for (;;)
	{
		if(cxidx-- == 0)
			retx = x;

		if(screen)
		{
			cb[0] = *linebuf;
			gftext(screen, font, cb, x, y, color, tmode, color2);
		}
		if(++i >= chars)
		{
			if(cxidx >= 0)
				retx = x;
			break;
		}
		x1 = rscale_by(xspace, i, ichars);
		x += fchar_spacing(font, linebuf) + x1 - lx1;
		lx1 = x1;
		++linebuf;
	}
	return(retx);
}

int justify_line(Raster *screen,
				  Vfont *font,
				  char *linebuf,
				  int x,int y,int w,
				  Pixel color,
				  Text_mode tmode,
				  Pixel color2,int just, SHORT *pcharx, int charx_idx)

/* returns width of text, If pcharx != NULL it loads it with
 * the start x of the char indexed by charx_idx in the line.
 * ie: if charx_idx == 0 it will return the x position of the
 * first character, if char_idx is 2 the 3rd character if >= last char
 * x position of the last char ( passing in -1 will give last char without
 * fail) if there are no chars on the line the position will be the
 * position of the first-last char */
{
int textwid;
int wleft;
unsigned int ccount;
char *last;
char savelast;


	if(0 == (ccount = strlen(linebuf)))
		goto no_text;

	last = linebuf + ccount - 1;

	if((savelast = last[0]) == ' ')
	{
		if(ccount > 1
			&& last[-1] != ' '
			&& last[-1] != '\t')
		{
			--ccount;
		}
	}
	else if(last[0] == '\n')
		--ccount;

	linebuf[ccount] = 0;

	if(0 == (textwid = fnstring_width(font, linebuf, ccount)))
		goto no_text;

	wleft = w - textwid;

	switch(just)
	{
		case JUST_FILL:
		{
			textwid = just_fill(screen, font, linebuf, ccount, x, y, color,
							 tmode, color2, wleft, charx_idx);
			if(pcharx)
			{
				/* line truncated and want last char position */
				if (*last == NULL && ((unsigned int)charx_idx >= ccount))
					*pcharx = x + w;
				else
					*pcharx = textwid;
			}
			textwid = w; /* filled up */
			goto done;
		}
		case JUST_LEFT:
		default:
			break;
		case JUST_RIGHT:
			x += wleft;
			break;
		case JUST_CENTER:
			x += wleft/2;
			break;
	}

	if(screen)
		gftext(screen, font, linebuf, x, y, color, tmode, color2);

	if(pcharx)
	{
		if(*last == NULL)
		{
			*last = ' ';
			++ccount;
		}
		if(charx_idx)
		{
			if ((unsigned int)charx_idx > ccount)
				charx_idx = ccount;
			x += fnstring_width(font,linebuf,charx_idx) + font->spacing;
		}
		*pcharx = x;
	}
done:
	*last = savelast;
	return(textwid);

no_text:
	if(pcharx)
	{
		switch(just)
		{
			case JUST_RIGHT:
				x += w - fchar_spacing(font," ");
				break;
			case JUST_CENTER:
				x += ((w - fchar_spacing(font," ")>>1));
				break;
			default:
				break;
		}
		*pcharx = x;
	}
	return(0);
}




