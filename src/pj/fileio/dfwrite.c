#include "dfile.ih"

long pj_write(Jfl *f, void *buf, long count)
{
long wt;

	if((wt = pj_dwrite(f->handle.j, buf, count)) < count)
		jerr = pj_mserror(pj_dget_err());
	return(wt);
}
