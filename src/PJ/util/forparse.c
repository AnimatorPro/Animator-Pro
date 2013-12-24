#define FORMATF_INTERNALS
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include "formatf.h"

static char do_parsechar(Formatarg *fa)

/* ignore all chars and arguments, returning the format type chars only */
{
char *fmt = fa->fmt;

	while(*fmt)
	{
		if(*fmt++ == '%')
		{
			fa->str = fa->fmt = fmt - 1;
			return(geta_fmtchar(fa));
		}
	}
	return(0);
}
void init_format_parse(Formatarg *fa, char *fmt)
{
	init_formatarg(fa,fmt);
	fa->mflags |= FA_PARSE_MODE;
	fa->parse_stars = 0;
	fa->getchar = fa->root = do_parsechar;
}
