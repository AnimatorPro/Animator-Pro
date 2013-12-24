#include "lfile.ih"

Errcode lfflush(LFILE *f)
{
if (f->is_dirty)
	return(_lf_wflush(f));
else if (f->flags&BFL_READ)
	f->pt = f->end = f->start;
return(Success);
}
