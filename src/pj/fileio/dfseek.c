#include "dfile.ih"

long pj_seek(Jfl *f, long offset, int mode)
{
long pos;

	if((pos = pj_dseek(f->handle.j,offset,mode)) < Success)
		jerr = pos;
	return(pos);
}
