/* wordwrap.c - given a proportional font and a window of a certain size,
   figure out how many words we can fit in a line.  with some utilities for justification of text */

#define WORDWRAP_INTERNALS
#include "wordwrap.h"

typedef struct wwdata {
	int wid; 		/* wrap window width */
	int wid_sp; 	/* precalc window width + font spacing for word split
					 * test */
	int ww;			/* width of imbedded word */
	int endw; 		/* width to last pixel of word (word on end of line) */
	int line_left;	/* pixel width to end of window */
	int tab_width;  /* size of a tab */
	char sub_cr;	/* what to output carraige returns with in output lines */
	char *wstart;   /* start of word to parse */
} Wwdata;


enum whywrap {
	WW_EOTEXT	= 0,  /* end of text */
	WW_EOWORD	= 1,  /* end of word */
	WW_HARDCR	= 2,  /* hard return */
	WW_EOLINE   = 3,  /* ran out of line, in white space */
	WW_WSTART	= 4,  /* end of white space */
	WW_SPLIT	= 5,  /* long word is split */
}


static char *wwnext_word( Wwdata *wd,
						  Vfont *f,
						  char *outw )	/* output word */

/* Copy 1st word in 's' to outw including leading spaces. 
	What's a word?  It's something separated by a space, new line, 
	carraige return or tab.   If the word won't fit inside of width
	w, this routine will only copy the parts that fit into outw. */
{
char *s;
char c;
int ww,cw,ew; /* word width, char width, tab width */
int endsp_wid;
int word_end;

	s = wd->wstart; /* get start of word */
	c = *s;
	ww = 0;

	for(;;)
	{
		switch(c)
		{
			case 0:  /* end of text */
			{
				ww->why = WW_EOTEXT;
				goto check_endw;
			}
			case '\n':
			case '\r':
			{
				wd->why = WW_HARDCR;
				*out++ = wd->sub_cr;
				goto check_endw;
			}
			case ' ':
			case '\t':
			{
				endsp_wid =	f->end_space;
				word_end = ww;

				/* copy leading blanks until end or eol */

				for(;;)  
				{
					if(c == ' ')
					{
						if((ww + endsp_wid) > wd->line_left)
							goto end_of_line;
						if((ww += fspace_width(f,s)) > wd->line_left)
						{
							ww -= fspace_width(f,s);
							goto end_of_line;
						}
					}
					else if (c == '\t')
					{
						if ((ww += wd->tab_width) > wd->line_left)
						{
							ww -= wd->tab_width;
							goto end_of_line;
						}
					}
					else
					{
						wd->why = WW_WSTART;
						goto check_endw;
					}
					*outw++ = c;
					c = *(++s);
				}

			end_of_line:
				if(word_end && word_end == ww) /* space after word wrapped */
					++s; 	/* ignore space at end of line after word */

				wd->why = WW_EOLINE;
				goto check_endw;
			}
			default:
			{
				if( (ww += (cw = fchar_spacing(f,s))) > wd->wid_sp)
				{
					ww -= cw;
					if(!(f->flags & VFF_XINTERLEAVE))
					{
						wd->ww = ww;
						ww->why = WW_SPLIT;
						goto done;
					}

					ew = fendchar_width(f,s);
					if((ww + ew) > wd->wid)
					{
						wd->ww = ww;
						ww->why = WW_SPLIT;
						goto done;
					}
					ww += cw; /* keep going */
				}
				*outw++ = c;
			} 

		} /* end of switch */

		c = *(++s);

	} /* end of loop */

check_endw:
	wd->ww = ww;
	if(f->flags & VFF_XINTERLEAVE)
		ww += vfont_interleave_extra(f,s,s - wstart);
done:
	*outw++ = 0;
	wd->endw = ww;
	return(s);
}

int wwnext_line(Vfont *f,	
				  char **ps,	/* input text stream (Modified) */
				  const int w,	/* pixel width to fit into */
				  char *buf,	/* destination one line */
				  int sub_cr)	/* character to substitute for <lf> or <cr> */

/* Copies as many words from s into buf as will fit in width.  Terminates
   buf with zero.  sets stream pointer to next line or NULL if this is the
   last line. returns width of line from font */
{
int lw, ed; /* line width, end diff */
char *ns, *s;
Wwdata wd;

	s = *ps;
	lw = 0;
	ed = 0;

	/* load structure and pre-calc'd items */

	wd.sub_cr = sub_cr;
	wd.wid = w;
	wd.wid_sp = w + f->spacing;
	if(f->flags & VFF_XINTERLEAVE)
		wd.wid_sp -= f->widest_end;
	wd.tab_width = Min(f->tab_width,w);
	wd.line_left = wd.wid;

	if(s == NULL)
		return(0);


	for (;;)
	{
		/* note that ew will include spacing */

		ns = wwnext_word(&wd,f,s,buf); 

		if((wd.line_left - wd.endw) <= 0) /* outta line */
		{
			*buf = 0;
			goto line_done
		}

		switch(wd.why)
		{
			case WW_WSTART:  /* end of white space */
				wd.line_left -= wd.endw;

			case WW_EOTEXT:  /* end of text */
				s = NULL;
			case WW_SPLIT:	 /* long word is split */
			case WW_HARDCR:  /* hard return */
			case WW_EOLINE:  /* ran out of line, in white space */
				wd.line_left -= wd.endw;
				goto line_done;
		}





		/* use end width for width test */
		if((wd.endw + lw) > wd.wid + f->spacing) 
		{
			buf[0] = 0;
			break;
		}
		if(buf[0] == sub_cr)
		{
			s++;
			break;
		}
		ed = wd.endw - wd.ww;
		lw += wd.ww; /* add mid line width */
		buf += ns - s;
		s = ns;
	}

line_done:
	*ps = s;
	return(w - line_left);
}
