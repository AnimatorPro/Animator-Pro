#include "lfile.ih"

lfputc(int c, LFILE *f)
{
if (f->pt >= f->end) 
	{
	if (_lf_wflush(f)<Success)
		return(LEOF);
	}
return(*f->pt++ = c,f->is_dirty = TRUE);
}
