#include "dfile.ih"

long pj_read(Jfl *f, void *buf, long count)
{
long rd;

	if((rd = pj_dread(f->handle.j, buf, count)) < count)
		jerr = pj_mserror(pj_dget_err());
	return(rd);
}
