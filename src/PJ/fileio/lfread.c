#include <string.h>
#include "lfile.ih"
#include "ptrmacro.h"

unsigned lfread(void *buf, unsigned size, unsigned count, LFILE *f)
/* fread - optimized for size > count.  If size is larger than
   file buffer size this will effectively turn into a direct unbuffered
   series of count reads.  Otherwise memcpy size bytes from file buffer
   count times, _lf_rflushing as necessary */
{
int sizeleft;
long lsize = size*count;
long rsize;
int c;
char *pt = buf;

	if (f->flags&BFL_TEXT)	/* in translated mode do it char at a time */
	{
		for (rsize=0; rsize<lsize; ++rsize)
		{
			if((c = lgetc(f)) < 0)
				break;
			*pt++ = c;
		}
	}
	/* if the whole entity is in the buffer just memcpy it */
	else if ((sizeleft = f->end - f->pt) >= lsize)
	{
EZCOPY:
		memcpy(pt, f->pt, (int)lsize);
		f->pt += lsize;
		pt = OPTR(pt, lsize);
		return(count);
	}
	else /* otherwise memcpy what you can... */
	{
		if (sizeleft > 0)
		{
			memcpy(pt, f->pt, sizeleft);
			pt  = OPTR(pt, sizeleft);
			lsize -= sizeleft;
		}
		/* if remaining request as large as the buffer service it directly */
		if (lsize >= f->bsize)
		{
			f->pt = f->end = f->start;		/* make buffer empty */
			if ((rsize = lread(f->lfile, pt, lsize)) < lsize)
			{
				f->ferr = lerrno = pj_ioerr();
				rsize += sizeleft;
			}
			else
				return(count);
		}
		else
		{
			if (_lf_rflush(f)>=Success)
			{
				if (f->end - f->pt >= lsize)
					goto EZCOPY;
				else
					f->ferr = lerrno = Err_eof;
			}
			rsize = sizeleft;
		}
	}
	if (size == 1)
		return(rsize);
	else
		return(rsize/size);
}
