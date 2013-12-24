#include "lfile.ih"

long lftell(LFILE *f)
{
long pos;

if (lfseek(f, 0L, LSEEK_CUR) < 0)
	return(lerrno);
pos = f->fpos;
if (f->is_dirty)
	pos += f->pt - f->start;
else
	pos -= f->end - f->pt;
return(pos);
}
