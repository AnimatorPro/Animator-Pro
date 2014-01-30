/* wordwrap.c - given a proportional font and a window of a certain size,
 * figure out how many words we can fit in a line. 
 * 
 */
   
#define WORDWRAP_INTERNALS
#include "wordwrap.h"
#include <ctype.h>


static char *wwnext_word(Wwdata *wd)

/* Copy 1st word in 's' to out including leading spaces. 
	What's a word?  It's something separated by a space, new line, 
	carraige return or tab.   If the word won't fit inside of width
	w, this routine will only copy the parts that fit into out. */
{
char *s;
char c;
char *out;
int ww,cw,ew; /* max_ww, word width, char width, end width */
int endsp_wid;
Vfont *f = wd->font;
char *sstart;

	s = wd->lstart; /* get start of word */
	out = wd->out;
	c = *s;
	ww = 0;

	for(;;)
	{
		switch(c)
		{
			case 0:  /* end of text */
			{
				wd->why = WW_EOTEXT;
				wd->ww = ww;
				goto check_endwidth;
			}
			case '\n':
			case '\r':
			{
				wd->why = WW_HARDCR;
				*out++ = wd->sub_cr;
				wd->ww = ww;
				if(f->flags & VFF_XINTERLEAVE)
					ww += vfont_interleave_extra(f,s,s - wd->lstart);
				++s;
				goto done;
			}
			case '\t':
			case ' ':
			{
				sstart = s;
				endsp_wid =	f->end_space;

				/* note: will return ww->why unaltered if wrap test fails */

				for (;;)  /* copy leading blanks before next word */
				{
					if (c == ' ')
					{
						if((ww + endsp_wid) > wd->line_left)
							goto check_space_wrap;
						if((ww += fspace_width(f,s)) > wd->line_left)
						{
							ww -= fspace_width(f,s);
							goto check_space_wrap;
						}
					}
					else if (c == '\t')
					{
						if ((ww += wd->tab_width) > wd->line_left)
						{
							ww -= wd->tab_width;
							goto done;
						}
					}
					else /* not a space */
					{
						wd->why = WW_EOWORD;
						goto check_endwidth;
					}

					*out++ = c;
					c = *(++s);
				}

			check_space_wrap:  /* this only handles word terminators as last
								* char on line that wraps */

				if(s == sstart   		  /* first space after word? */
					&& !ispunct(s[-1])    /* puncts force wrap */
					&& s[1] != ' '        /* followed by a space! wrap! */
					&& s[1] != '\t'
					&& s[1] != '\n'
					&& s[1] != '\c'
					&& s != wd->lstart)   /* not first char of word */
				{
					++s; /* Ignore this space on next line */
					wd->why = WW_FILLED;
				}
				goto done;
			}
			default:
			{
				/* note: will return ww->why unaltered if wrap test fails */

				if((ww += (cw = fchar_spacing(f,s))) > wd->wrap_test)
				{
					ww -= cw;
					if(!(f->flags & VFF_XINTERLEAVE))
					{
						wd->ww = ww;
						goto done;
					}
					ew = fendchar_width(f,s);
					if((ww + ew) > (wd->line_left + f->spacing))
					{
						wd->ww = ww;
						goto done;
					}
					ww += cw; /* keep going */
				}
				*out++ = c;
			} 

		} /* end of switch */

		c = *(++s);

	} /* end of loop */

check_endwidth:
	wd->ww = ww;
	if(f->flags & VFF_XINTERLEAVE)
		ww += vfont_interleave_extra(f,s,s - wd->lstart);
done:
	wd->endw = ww;
	wd->out = out;
	return(s);
}

static int ww_line(Wwdata *wd)
/* Copies as many words from s into out as will fit in wd->wid.  Terminates
   out with zero.  sets stream pointer to next line or NULL if this is the
   last line. returns width of line from font */
{
char *ns;
char *oout = NULL;
int end_diff;

	wd->wrap_test = wd->max_wrap_test;
	wd->line_left = wd->wid;
	wd->why = WW_FILLED;
	end_diff = 0;

	for (;;)
	{
		ns = wwnext_word(wd); 

		/* This could remove a single space from the beginning of the text
		 * buffer if the [-1] char is a tab or a space */

		if(wd->why == WW_LONGWORD)
		{
			wd->out = oout;
			break;
		}

		wd->line_left -= wd->ww;
		wd->wrap_test -= wd->ww;
		end_diff = wd->endw - wd->ww;
		wd->lstart = ns;

		if(wd->why <= WW_EOTEXT)
			break;

		oout = wd->out;
		wd->why = WW_LONGWORD; /* for next wrap test */
	}

	wd->line_left -= end_diff;
	*wd->out = 0;
	return(wd->why);
}

int wwnext_line(Vfont *f,	
				  char **ps,	/* input text stream (Modified) */
				  const int w,	/* pixel width to fit into */
				  char *buf,	/* destination one line */
				  int sub_cr)	/* character to substitute for <lf> or <cr> */
{
Wwdata wd;

	if((wd.lstart = *ps) == NULL)
		return(0);

	/* load structure and pre-calc'd items */

	wd.font = f;
	wd.wid = w;
	wd.max_wrap_test = w + f->spacing;
	if(f->flags & VFF_XINTERLEAVE)
		wd.max_wrap_test -= f->widest_end;
	wd.sub_cr = sub_cr;
	wd.tab_width = Min(f->tab_width,w);
	wd.out = buf;

	if(ww_line(&wd) == WW_EOTEXT)
		wd.lstart = NULL;

	*ps = wd.lstart;
	return(wd.wid - wd.line_left);
}
