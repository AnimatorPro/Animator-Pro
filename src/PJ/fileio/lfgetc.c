#include "lfile.ih"

Errcode lfgetc(LFILE *f)
{
if (f->pt >= f->end) 
	{
	if (_lf_rflush(f)<Success)
		return(LEOF);
	}
return(*f->pt++);
}
