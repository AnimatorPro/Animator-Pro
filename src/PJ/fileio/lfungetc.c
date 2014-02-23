#include "lfile.ih"

int lungetc(int c, LFILE *f)
{
if (c == LEOF || f->pt <= f->start)
	return(LEOF);
return(*--f->pt = c);
}
