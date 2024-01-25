#define WORDWRAP_INTERNALS
#include "wordwrap.h"

int wwcount_lines(Vfont *f,char *s,int w, SHORT *maxwid)
/* count lines and maximum width that word-wrap will take for a given 
 * text block, font, and width */
{
int lines;
int wid;
SHORT dummywid;
char buf[512];

	if(!maxwid)
		maxwid = &dummywid;

	*maxwid = 0;
	lines = 1;
	for (;;)
	{
		if((wid = wwnext_line(f, &s, w, buf, 0)) > *maxwid)
			*maxwid = wid;
		if(s == NULL)
		{
			if(buf[0] == 0) /* last line empty, last char was a newline. */
				--lines;
			break;
		}
		lines++;
	}
	return(lines);
}
