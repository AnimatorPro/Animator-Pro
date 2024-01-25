#include "dfile.ih"

long pj_tell(Jfl *f)
{
long pos;

	if ((pos = pj_dtell(f->handle.j)) < Success)
		jerr = pos;
	return(pos);
}
