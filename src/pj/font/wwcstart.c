#define WORDWRAP_INTERNALS
#include "wordwrap.h"

int just_charstart(Vfont *font,int x, int lwidth,
				   char *linebuf,int charpos, int just)

/* returns x position of a character in a justified line */
{
SHORT charx;

	justify_line(NULL,font,linebuf,x,0,lwidth,0,TM_MASK1,0,just,&charx,charpos);
	return(charx);
}
