#include "lfile.ih"

lfgetc(LFILE *f)
{
if (f->pt >= f->end) 
	{
	if (_lf_rflush(f)<Success)
		return(LEOF);
	}
return(*f->pt++);
}
